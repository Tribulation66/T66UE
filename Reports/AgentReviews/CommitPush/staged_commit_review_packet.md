# Staged Commit Review Packet

Task contract: Commit approved current T66 changes to `main`, push `main`, create and push the required version tag, and verify `main` matches `origin/main` with tracked changes classified or clean.

Current commit strategy:
- Commit staged non-Mini tracked changes.
- Do not stage `Source/T66/UI/Screens/T66MinigamesScreen.cpp`; classify it as deferred because root `AGENTS.md` excludes Mini/minigame scope unless explicitly named.
- Include user-approved obsolete asset deletions and narrow `.gitignore` rules for regenerated obsolete QuickRevive/HillTile assets.
- Keep runtime DataTable `.uasset` files tracked and included.
- Bump `Config/DefaultGame.ini` from `ProjectVersion=alpha-0.8` to `ProjectVersion=alpha-0.9`, because `alpha-0.8` already exists on `origin/main`.
- After commit, push `main`, create tag `alpha-0.9` from the committed `main` tip, push tag `alpha-0.9`, and verify local/remote state.

Applicable instructions:
- `AGENTS.md` says `main` is the normal development branch.
- `AGENTS.md` says "commit and push" means commit approved changes to `main`, push `main`, create and push the next version tag, and verify the working tree afterward.
- `AGENTS.md` says no blanket reset/clean/discard without explicit approval.
- `AGENTS.md` says avoid broad Git/LFS scans over binary asset folders unless needed; this pass used path-scoped staging and targeted checks.
- `AGENTS.md` excludes Mini/minigame scope by default, so the Mini screen change is not staged.
- `.gitignore` says raw source art/generated runs stay local; runtime `Content` assets remain in Git unless explicitly carved out.

Current evidence:
- Current branch: `main`.
- Upstream: `origin/main`.
- Existing tag: `alpha-0.8`.
- Proposed new tag: `alpha-0.9`.
- Staged file count before this packet: 239.
- Staged LFS-extension file count before this packet: 25.
- Staged Mini paths before this packet: none.
- Unstaged tracked Mini path: `Source/T66/UI/Screens/T66MinigamesScreen.cpp`.
- `git diff --cached --check` passes after fixing whitespace/EOF issues.
- `git check-ignore -v --no-index` confirmed representative obsolete QuickRevive/HillTile paths match the new `.gitignore` rules.
- `git check-ignore -v --no-index -- Content/Data/DT_Stages.uasset` produced no ignore match.

Risks and mitigations:
- Risk: large mixed staged commit. Mitigation: user requested commit/push, obsolete asset handling was explicitly approved, Mini path is deferred, and diff check passes.
- Risk: tag collision. Mitigation: use `alpha-0.9` because `alpha-0.8` already exists.
- Risk: LFS churn. Mitigation: LFS staged files are runtime DataTables or explicitly obsolete deletions; raw/source-art ignores remain narrow.

Planned commands after approval:
1. Stage this packet and the resulting review artifact.
2. Re-run `git diff --cached --check`.
3. Commit with a concise alpha-0.9 message.
4. Push `main`.
5. Create annotated or lightweight tag `alpha-0.9` from `HEAD` if absent.
6. Push tag `alpha-0.9`.
7. Verify `git fetch origin`, `git rev-parse main`, `git rev-parse origin/main`, `git rev-parse alpha-0.9`, and tracked status with the Mini deferred path classified.

Reviewer request:
- Validate whether the staged commit/push strategy is safe under the current repo instructions.
- Check whether deferring the Mini path rather than including it is the right interpretation of root scope.
- Check whether `alpha-0.9` is the right next tag under the current state.
- Identify any blocker before commit/push/tag.
