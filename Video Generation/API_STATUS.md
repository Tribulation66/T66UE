# Kling API Status

Last checked: 2026-04-28

## Local Credentials

The machine had these user environment variables set:

- `KLING_ACCESS_KEY`
- `KLING_SECRET_KEY`

These are not stored in this repo. Do not paste them into chat.

## Auth Check

JWT auth was generated locally with HS256 using:

- `iss`: access key
- `exp`: current time + 1800 seconds
- `nbf`: current time - 5 seconds

Authenticated probe:

```text
GET /v1/videos/image2video/not-a-real-task-id
```

Result:

```json
{
  "code": 1201,
  "message": "Task not found by id/external id: not-a-real-task-id"
}
```

This is a useful result because it proves the API recognized the request and auth token. If auth was invalid, it would have failed differently.

## Motion Brush Findings

Endpoint:

```text
POST https://api-singapore.klingai.com/v1/videos/image2video
```

Attempt 1:

```json
{
  "model_name": "kling-v1-6",
  "mode": "pro",
  "duration": "5",
  "has_static_mask": true
}
```

Result:

```json
{
  "code": 1201,
  "message": "kling-v1-6 is not supported with motion brush"
}
```

Attempt 2:

```json
{
  "model_name": "kling-v1",
  "mode": "pro",
  "duration": "5",
  "has_static_mask": true
}
```

Result:

```json
{
  "code": 1102,
  "message": "Account balance not enough"
}
```

Attempt 3:

```json
{
  "model_name": "kling-v1",
  "mode": "std",
  "duration": "5",
  "has_static_mask": true
}
```

Result:

```json
{
  "code": 1102,
  "message": "Account balance not enough"
}
```

## Interpretation

The official API recognizes the static mask parameter as a Motion Brush path, but current support appears limited to `kling-v1`.

The newer models are better visually, so the next practical test should be Video 3.0 without Motion Brush. Use Motion Brush only if the newer model moves the idol too much.
