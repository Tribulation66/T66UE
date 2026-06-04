# Commit/Push Version Decision

User request: `Commit and push`

Current blocker:

- Root `AGENTS.md` defines `commit and push` as committing approved changes to `main`, pushing `main`, creating and pushing the next version tag, and verifying clean state afterward.
- The prompt did not name a new version.
- The current project version is already `alpha-1.0`.
- The `alpha-1.0` tag already exists, so reusing or moving it for a new commit would be a release decision, not a deterministic repo rule.

Decision needed:

What exact version string should be used for the next release tag and matching `Config/DefaultGame.ini` `ProjectVersion`?

Examples only: `alpha-1.1`, `alpha-2.0`, or another value you choose.

Wait state:

Do not commit, push, retag, or update `ProjectVersion` until the user answers this version decision or explicitly says to commit/push `main` without creating a version tag.
