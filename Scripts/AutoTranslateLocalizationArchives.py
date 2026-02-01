"""
AutoTranslateLocalizationArchives.py

Populates Unreal localization archives with machine translations (Argos Translate).

Reads:
  Content/Localization/T66/en/T66.archive  (source of truth for keys + source text)

Writes:
  Content/Localization/T66/<culture>/T66.archive  (fills Translation.Text)

Then you should run:
  UnrealEditor-Cmd.exe ... -run=GatherText -config=Config/Localization/T66_Compile.ini
to compile updated .locres files.

Notes
- This produces *machine translations* intended as a first pass; it is not a substitute for professional localization QA.
- Preserves format placeholders like {0}, {1}, and tries not to touch pure-symbol strings.
- By default this script is **non-destructive**:
  - It only fills missing/empty translations (and optionally refreshes entries it previously auto-translated when the English
    source text changes).
  - It does NOT overwrite existing non-empty translations that were not created by this script (so human translations are safe).
"""

from __future__ import annotations

import os
import json
import re
import hashlib
from dataclasses import dataclass
from pathlib import Path
from typing import Any, Dict, List, Tuple


ROOT = Path(r"C:\UE\T66")
LOC_ROOT = ROOT / "Content" / "Localization" / "T66"
SOURCE_ARCHIVE = LOC_ROOT / "en" / "T66.archive"
ARGOS_CACHE_DIR = ROOT / "Saved" / "Localization" / "ArgosModels"
LOCK_FILE = ROOT / "Saved" / "Localization" / "AutoTranslateLocalizationArchives.lock"
STATE_FILE = ROOT / "Saved" / "Localization" / "AutoTranslateLocalizationArchives.state.json"

# Argos uses Stanza by default for sentence chunking, but newer Stanza versions can break some languages.
# We force Spacy-based chunking for stability.
os.environ.setdefault("ARGOS_CHUNK_TYPE", "SPACY")


# UE culture folders we must fill.
TARGET_CULTURES: List[str] = [
    "zh-Hans",
    "zh-Hant",
    "ja",
    "ko",
    "ru",
    "pl",
    "de",
    "fr",
    "es-ES",
    "es-419",
    "pt-BR",
    "pt-PT",
    "it",
    "tr",
    "uk",
    "cs",
    "hu",
    "th",
    "vi",
    "id",
    "ar",
]


# Map UE culture tags to Argos language codes.
# Argos uses:
# - zh = Simplified Chinese
# - zt = Traditional Chinese
ARGOS_CODE_FOR_CULTURE: Dict[str, str] = {
    "zh-Hans": "zh",
    "zh-Hant": "zt",
    "es-ES": "es",
    "es-419": "es",
    "pt-BR": "pt",
    "pt-PT": "pt",
}


PLACEHOLDER_RE = re.compile(r"\{\d+\}")


def _read_json_utf16(path: Path) -> Any:
    return json.loads(path.read_text(encoding="utf-16"))


def _write_json_utf16(path: Path, data: Any) -> None:
    path.write_text(json.dumps(data, ensure_ascii=False, indent=2), encoding="utf-16")

def _sha1_utf8(s: str) -> str:
    return hashlib.sha1(s.encode("utf-8")).hexdigest()

def _read_state() -> Dict[str, Dict[str, str]]:
    """
    State format:
      {
        "<culture>": {
          "<namespace>|<key>": "<sha1(source_text)>"
        }
      }
    Only entries present in this state are considered "owned" by the auto-translator for refresh-on-source-change.
    """
    try:
        if not STATE_FILE.exists():
            return {}
        return json.loads(STATE_FILE.read_text(encoding="utf-8"))
    except Exception:
        # Fail open: treat as no prior state.
        return {}

def _write_state(state: Dict[str, Dict[str, str]]) -> None:
    STATE_FILE.parent.mkdir(parents=True, exist_ok=True)
    STATE_FILE.write_text(json.dumps(state, ensure_ascii=False, indent=2), encoding="utf-8")


def _is_only_symbols_or_whitespace(s: str) -> bool:
    if s.strip() == "":
        return True
    # keep very short pure-symbol tokens as-is (common in UI)
    if len(s) <= 3 and all(not ch.isalnum() for ch in s):
        return True
    # placeholders only
    stripped = PLACEHOLDER_RE.sub("", s).strip()
    if stripped == "":
        return True
    return False


def _extract_placeholders(s: str) -> List[str]:
    return PLACEHOLDER_RE.findall(s)


def _ensure_placeholders_present(src: str, translated: str) -> str:
    # If translation engine dropped placeholders, fall back to source.
    src_ph = _extract_placeholders(src)
    if not src_ph:
        return translated
    for ph in src_ph:
        if ph not in translated:
            return src
    return translated


def _walk_archive_children(root: Dict[str, Any]) -> List[Tuple[str, Dict[str, Any]]]:
    """
    Returns list of (namespace, child_dict) for every 'Children' entry in the archive tree.
    """
    out: List[Tuple[str, Dict[str, Any]]] = []

    def walk(prefix: str, node: Dict[str, Any]) -> None:
        ns = node.get("Namespace", "") or ""
        full = ".".join([x for x in (prefix, ns) if x])
        for child in node.get("Children", []) or []:
            out.append((full, child))
        for sub in node.get("Subnamespaces", []) or []:
            walk(full, sub)

    walk("", root)
    return out


def _get_argos_code(culture: str) -> str:
    return ARGOS_CODE_FOR_CULTURE.get(culture, culture)


def _ensure_argos_packages_installed(to_codes: List[str]) -> None:
    import argostranslate.package as pkg
    import requests

    pkg.update_package_index()
    available = pkg.get_available_packages()
    needed = set(to_codes)

    # Determine what's already installed
    import argostranslate.translate as tr

    installed = {l.code for l in tr.get_installed_languages()}
    print(f"[argos] installed before: {sorted(installed)}", flush=True)
    if "en" not in installed:
        # We'll install en->X packages, which also provides 'en' as a language.
        pass

    for to_code in sorted(needed):
        # If a target language code is already installed, skip.
        if to_code in installed:
            print(f"[skip] already installed: {to_code}", flush=True)
            continue

        candidates = [p for p in available if p.from_code == "en" and p.to_code == to_code]
        if not candidates:
            raise RuntimeError(f"No Argos package found for en -> {to_code}")

        # Prefer highest version if multiple
        chosen = sorted(
            candidates,
            key=lambda p: tuple(int(x) for x in (p.package_version or "0").split(".") if x.isdigit()),
            reverse=True,
        )[0]

        # Argos' built-in download() can hang on some networks; download ourselves with requests.
        https_links = [l for l in (chosen.links or []) if isinstance(l, str) and l.startswith("https://")]
        if not https_links:
            raise RuntimeError(f"No HTTPS download link for package en -> {to_code}")
        url = https_links[0]

        ARGOS_CACHE_DIR.mkdir(parents=True, exist_ok=True)
        filename = url.rsplit("/", 1)[-1]
        model_path = ARGOS_CACHE_DIR / filename

        print(f"[install] en -> {to_code} ({chosen.package_version})", flush=True)
        if not model_path.exists():
            print(f"[download] {url}", flush=True)
            with requests.get(url, stream=True, timeout=60) as r:
                r.raise_for_status()
                total = int(r.headers.get("Content-Length") or 0)
                downloaded = 0
                with open(model_path, "wb") as f:
                    for chunk in r.iter_content(chunk_size=1024 * 256):
                        if not chunk:
                            continue
                        f.write(chunk)
                        downloaded += len(chunk)
                        if total and downloaded % (1024 * 1024 * 8) < len(chunk):
                            mb = downloaded / (1024 * 1024)
                            tmb = total / (1024 * 1024)
                            print(f"  ... {mb:.1f} / {tmb:.1f} MB", flush=True)
        else:
            print(f"[cache] using {model_path}", flush=True)

        pkg.install_from_path(str(model_path))

        # Refresh installed codes
        installed = {l.code for l in tr.get_installed_languages()}
        print(f"[ok] installed now includes: {to_code}", flush=True)


def _build_translators(to_codes: List[str]):
    import argostranslate.translate as tr

    langs = {l.code: l for l in tr.get_installed_languages()}
    if "en" not in langs:
        raise RuntimeError("Argos 'en' language is not installed (expected after installing en->X packages).")

    from_lang = langs["en"]
    translators = {}
    for code in to_codes:
        if code == "en":
            continue
        if code not in langs:
            raise RuntimeError(f"Argos language '{code}' is not installed.")
        translators[code] = from_lang.get_translation(langs[code])
    return translators


def main() -> int:
    # Prevent concurrent runs clobbering archives.
    LOCK_FILE.parent.mkdir(parents=True, exist_ok=True)
    try:
        # Exclusive creation fails if lock already exists.
        fd = os.open(str(LOCK_FILE), os.O_CREAT | os.O_EXCL | os.O_WRONLY)
        os.close(fd)
    except FileExistsError:
        raise RuntimeError(f"Lock file exists (another run is active?): {LOCK_FILE}")

    try:
        if not SOURCE_ARCHIVE.exists():
            raise FileNotFoundError(f"Missing source archive: {SOURCE_ARCHIVE}")

        source_root = _read_json_utf16(SOURCE_ARCHIVE)
        source_children = _walk_archive_children(source_root)

        state = _read_state()
        state_changed = False

        # Collect all distinct Argos codes we need.
        argos_to_codes = sorted({_get_argos_code(c) for c in TARGET_CULTURES})

        print("[argos] ensuring packages installed...", flush=True)
        _ensure_argos_packages_installed(argos_to_codes)

        print("[argos] building translators...", flush=True)
        translators = _build_translators(argos_to_codes)

        # Translate per culture
        for culture in TARGET_CULTURES:
            argos_code = _get_argos_code(culture)
            target_archive = LOC_ROOT / culture / "T66.archive"
            if not target_archive.exists():
                print(f"[skip] missing archive for {culture}: {target_archive}", flush=True)
                continue

            target_root = _read_json_utf16(target_archive)
            target_children = _walk_archive_children(target_root)

            # Build lookup by (namespace,key) for target entries
            target_map: Dict[Tuple[str, str], Dict[str, Any]] = {}
            for ns, child in target_children:
                key = child.get("Key", "") or ""
                target_map[(ns, key)] = child

            translator = translators.get(argos_code)
            if translator is None:
                raise RuntimeError(f"No translator available for {culture} (argos code {argos_code})")

            # State bucket per culture
            culture_state = state.get(culture, {})
            if not isinstance(culture_state, dict):
                culture_state = {}

            filled = 0
            refreshed = 0
            skipped = 0
            changed_any = False
            for ns, src_child in source_children:
                key = src_child.get("Key", "") or ""
                src_text = (src_child.get("Source") or {}).get("Text", "")
                if not isinstance(src_text, str):
                    continue

                tgt_child = target_map.get((ns, key))
                if not tgt_child:
                    # If key/namespace missing, skip (shouldn't happen)
                    continue

                entry_id = f"{ns}|{key}"
                src_hash = _sha1_utf8(src_text)

                # Existing translation?
                existing_translation = ""
                if isinstance(tgt_child.get("Translation"), dict):
                    existing_translation = str(tgt_child["Translation"].get("Text", "") or "")

                # Decide whether we should write:
                # - Fill if missing/empty
                # - Also treat "same as English" as effectively untranslated for non-symbol text
                # - Refresh only if this entry was previously auto-translated by this script and the English source changed
                should_fill = (existing_translation.strip() == "")
                if (not should_fill) and (not _is_only_symbols_or_whitespace(src_text)) and (existing_translation == src_text):
                    should_fill = True

                should_refresh = (entry_id in culture_state) and (culture_state.get(entry_id) != src_hash)

                if not should_fill and not should_refresh:
                    skipped += 1
                    continue

                if _is_only_symbols_or_whitespace(src_text):
                    translated = src_text
                else:
                    translated = translator.translate(src_text)
                    translated = _ensure_placeholders_present(src_text, translated)

                # Write into Translation.Text
                tgt_child.setdefault("Translation", {})
                if not isinstance(tgt_child["Translation"], dict):
                    tgt_child["Translation"] = {}
                if tgt_child["Translation"].get("Text") != translated:
                    tgt_child["Translation"]["Text"] = translated
                    changed_any = True

                # Mark as owned by auto-translator so we can refresh when English changes in the future.
                if culture_state.get(entry_id) != src_hash:
                    culture_state[entry_id] = src_hash
                    state_changed = True

                if should_refresh:
                    refreshed += 1
                else:
                    filled += 1

            if changed_any:
                _write_json_utf16(target_archive, target_root)

            # Persist culture state back into global state
            if state.get(culture) != culture_state:
                state[culture] = culture_state

            print(
                f"[ok] {culture}: filled {filled}, refreshed {refreshed}, skipped {skipped} (total {filled + refreshed + skipped})",
                flush=True,
            )

        if state_changed:
            _write_state(state)

        print("[done] archives updated. Re-run compile to regenerate .locres.", flush=True)
        return 0
    finally:
        try:
            LOCK_FILE.unlink(missing_ok=True)
        except Exception:
            pass


if __name__ == "__main__":
    raise SystemExit(main())

