#!/usr/bin/env python3
"""Small native Kling API client for T66 video-generation smoke tests."""

from __future__ import annotations

import base64
import hashlib
import hmac
import json
import time
import urllib.parse
from pathlib import Path
from typing import Any

import requests


PROJECT_ROOT = Path(__file__).resolve().parents[2]
DEFAULT_ACCESS_FILE = PROJECT_ROOT / "Model Generation" / "LOCAL_ACCESS.env"


class KlingClientError(RuntimeError):
    """Raised when the Kling API client cannot complete a request."""


def load_local_access(path: Path = DEFAULT_ACCESS_FILE) -> dict[str, str]:
    values: dict[str, str] = {}
    if not path.exists():
        raise KlingClientError(f"Local access file does not exist: {path}")

    for line_number, raw_line in enumerate(path.read_text(encoding="utf-8").splitlines(), start=1):
        line = raw_line.strip()
        if not line or line.startswith("#"):
            continue
        if "=" not in line:
            raise KlingClientError(f"Invalid LOCAL_ACCESS line {line_number}: missing '='")
        key, value = line.split("=", 1)
        values[key.strip()] = value.strip()

    return values


def b64url(data: bytes) -> str:
    return base64.urlsafe_b64encode(data).rstrip(b"=").decode("ascii")


def json_part(value: dict[str, Any]) -> str:
    encoded = json.dumps(value, separators=(",", ":"), sort_keys=True).encode("utf-8")
    return b64url(encoded)


def make_kling_jwt(access_key: str, secret_key: str, now: int | None = None) -> str:
    current = int(time.time()) if now is None else now
    header = {"alg": "HS256", "typ": "JWT"}
    payload = {
        "iss": access_key,
        "exp": current + 1800,
        "nbf": current - 5,
    }
    signing_input = f"{json_part(header)}.{json_part(payload)}"
    digest = hmac.new(secret_key.encode("utf-8"), signing_input.encode("ascii"), hashlib.sha256).digest()
    return f"{signing_input}.{b64url(digest)}"


def normalize_api_root(base_url: str) -> str:
    normalized = base_url.strip().rstrip("/")
    if not normalized:
        raise KlingClientError("KLING_BASE_URL is empty")
    if normalized.endswith("/v1"):
        return normalized
    return f"{normalized}/v1"


def encode_image_base64(path: Path) -> str:
    return base64.b64encode(path.read_bytes()).decode("ascii")


class KlingClient:
    def __init__(
        self,
        base_url: str,
        access_key: str,
        secret_key: str,
        timeout_seconds: int = 60,
    ) -> None:
        self.api_root = normalize_api_root(base_url)
        self.access_key = access_key
        self.secret_key = secret_key
        self.timeout_seconds = timeout_seconds

    @classmethod
    def from_local_access(cls, access_file: Path = DEFAULT_ACCESS_FILE) -> "KlingClient":
        values = load_local_access(access_file)
        provider = values.get("KLING_PROVIDER", "").lower()
        if provider != "native":
            raise KlingClientError(f"Expected KLING_PROVIDER=native, found {provider or '<missing>'}")

        missing = [
            key
            for key in ("KLING_BASE_URL", "KLING_ACCESS_KEY", "KLING_SECRET_KEY")
            if not values.get(key)
        ]
        if missing:
            raise KlingClientError(f"Missing required Kling local access keys: {', '.join(missing)}")

        return cls(
            base_url=values["KLING_BASE_URL"],
            access_key=values["KLING_ACCESS_KEY"],
            secret_key=values["KLING_SECRET_KEY"],
        )

    def endpoint_url(self, endpoint: str) -> str:
        endpoint = endpoint.strip()
        if not endpoint:
            raise KlingClientError("Empty endpoint")
        endpoint = endpoint.lstrip("/")
        if endpoint.startswith("v1/"):
            endpoint = endpoint[3:]
        return f"{self.api_root}/{endpoint}"

    def headers(self) -> dict[str, str]:
        return {
            "Authorization": f"Bearer {make_kling_jwt(self.access_key, self.secret_key)}",
            "Content-Type": "application/json",
        }

    def request(self, method: str, endpoint: str, **kwargs: Any) -> dict[str, Any]:
        url = self.endpoint_url(endpoint)
        response = requests.request(
            method=method,
            url=url,
            headers=self.headers(),
            timeout=self.timeout_seconds,
            **kwargs,
        )
        text = response.text
        try:
            body: Any = response.json()
        except ValueError:
            body = {"rawText": text[:2000]}

        return {
            "ok": response.ok,
            "http_status": response.status_code,
            "url_path": urllib.parse.urlparse(url).path,
            "body": body,
        }

    def get(self, endpoint: str) -> dict[str, Any]:
        return self.request("GET", endpoint)

    def post(self, endpoint: str, payload: dict[str, Any]) -> dict[str, Any]:
        clean_payload = {key: value for key, value in payload.items() if value is not None}
        return self.request("POST", endpoint, json=clean_payload)

    def download(self, url: str, output_path: Path) -> dict[str, Any]:
        output_path.parent.mkdir(parents=True, exist_ok=True)
        with requests.get(url, stream=True, timeout=self.timeout_seconds) as response:
            response.raise_for_status()
            with output_path.open("wb") as handle:
                for chunk in response.iter_content(chunk_size=1024 * 1024):
                    if chunk:
                        handle.write(chunk)
        return {
            "path": output_path.as_posix(),
            "bytes": output_path.stat().st_size,
        }


def summarize_response(response: dict[str, Any]) -> dict[str, Any]:
    body = response.get("body")
    summary: dict[str, Any] = {
        "ok": response.get("ok"),
        "http_status": response.get("http_status"),
        "url_path": response.get("url_path"),
    }
    if isinstance(body, dict):
        summary["code"] = body.get("code")
        summary["message"] = body.get("message")
        summary["request_id"] = body.get("request_id")
        data = body.get("data")
        if isinstance(data, list):
            summary["data_type"] = "list"
            summary["data_count"] = len(data)
            summary["task_statuses"] = sorted(
                {
                    str(item.get("task_status"))
                    for item in data
                    if isinstance(item, dict) and item.get("task_status")
                }
            )
        elif isinstance(data, dict):
            summary["data_type"] = "object"
            for key in ("task_id", "task_status", "task_status_msg", "created_at", "updated_at"):
                if key in data:
                    summary[key] = data.get(key)
        elif data is None:
            summary["data_type"] = "null"
    return summary
