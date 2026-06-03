You are Claude reviewing a Codex implementation or answer plan for the T66 Unreal project.

Rules:
- Start your response immediately with the verdict line. Do not write any
  preface, summary, confirmation, Markdown rule, or other text before it.
- Do not edit files.
- Do not run commands.
- Do not implement the plan.
- Review only the packet below.
- Be strict about contradictions with repo instructions, missing verification, unsafe scope, and unclear goals.
- Treat Codex as the implementer and you as the reviewer.

The first non-empty line of your review must be exactly one of these four lines:
Verdict: APPROVE
Verdict: REVISE
Verdict: NEEDS_HUMAN_DECISION
Verdict: BLOCK

After that verdict line, return a concise Markdown review with exactly these headings:
Blockers
Major Issues
Minor Issues
Clarifying Questions
Required Verification
Rationale

Verdict meanings:
- APPROVE: the reviewed plan/output is safe for Codex to proceed to implementation under the reviewed scope. Codex should not ask for redundant manual user approval after APPROVE unless the user explicitly marked the work planning-only, asked Codex to stop before implementation, the packet has an unresolved user-only decision, or AGENTS/PPF requires explicit approval for a method substitution.
- REVISE: Codex can resolve the issue by improving the plan/output, inspecting more repo state, tightening verification, changing implementation approach, or otherwise doing more Codex-owned work. Codex should revise and rerun review.
- NEEDS_HUMAN_DECISION: the plan/output depends on product direction, vision, risk acceptance, scope choice, or another decision only the user can make. Codex should save a decision block, ask once, and stop until the user answers.
- BLOCK: the plan/output cannot safely proceed because of a hard blocker, missing prerequisite, external-state issue, unavailable credential/context, or contradiction that is not solved by normal Codex revision.

Review scope:
- Packet path: C:\UE\T66\Reports\AgentReviews\CommitPushInstructionsQuestion\review_packet.md
- Output scope: review of the packet below only.

<review_packet>
# Review Packet: Commit And Push Instructions Question

Working goal: Identify the live T66 instructions for commits and pushes in AGENTS.md and any other repo instruction files.

Scope: Read-only answer. No code, content, config, or git state changes are proposed.

User question: "What are the instructions regarding commits and push in agents, are there instructions anywhere else other than agents.md?"

Clarified assumptions:
- The user wants a summary of current live repo instructions, not historical report artifacts.
- "agents" means root `AGENTS.md` and folder-owned `*_AGENTS.md` router files.
- "anywhere else" also includes current release/demo process docs that those agent routers point to.
- Mini/minigame systems are out of scope unless an instruction file is only mentioned as a discovered router; no Mini docs were inspected for content beyond the agent-router list.

Applicable repo instructions:
- `AGENTS.md` root router requires live repo inspection, folder instruction discovery, and Claude review by default.
- No PPF is required because this is a read-only process question with no visual/media/import/runtime artifact.

Evidence inspected:
- `AGENTS.md:200-213`: root `Version, Commit, And Push` and `Git LFS Status` sections.
- `Release/RELEASE_AGENTS.md:5-22`: Release router owns version naming, tag, branch, Steam upload, staged standalone, and points to release docs.
- `Release/README.md:5-9`: Release docs start page points to project guidelines and Steamworks upload/testing instructions.
- `Release/PROJECT_GUIDELINES_INSTRUCTIONS.md:45-48` and `:146-158`: runtime-facing changes must refresh staged standalone; packaged Development standalone is runtime source of truth.
- `Release/Steam/STEAMWORKS_UPLOAD_AND_TESTING_INSTRUCTIONS.md:1-18`, `:33-84`, `:255-262`: Steam upload workflow, branch/live-build context, correct staged Windows folder, and working-tree verification before Steamwork.
- `Demo/DEMO_AGENTS.md:11-22` and `Demo/DEMO_RELEASE_INSTRUCTIONS.md:73-88`, `:145-163`: demo-specific release/upload validation path.
- Agent-router search across `AGENTS.md` and `*_AGENTS.md`: only root `AGENTS.md` contains the generic commit/push/tag policy. Other routers add specialized constraints such as release upload routing, demo upload validation, "do not commit live secrets" in model generation, and "do not commit extracted vendor files" in rigging/animation.

Planned answer:
- Say the generic Git commit/push policy lives in root `AGENTS.md`.
- Summarize root rules:
  - `main` is the normal development branch; no version branches unless explicitly requested.
  - A named version updates `ProjectVersion` in `Config/DefaultGame.ini` and uses the same value for GitHub tag/release naming unless told otherwise.
  - Version snapshots are tags on `main`; after intended changes are committed and pushed to `origin/main`, create and push the tag from committed `main`.
  - "commit and push" means commit approved changes to `main`, push `main`, create and push next version tag, then verify clean tree.
  - Completion requires `main == origin/main` and no remaining tracked changes; remaining tracked changes must be classified as commit, restore, ignore/untrack, or deferred.
  - No blanket discard/reset/clean without explicit approval.
  - Avoid broad Git/LFS status or diff over binary asset folders unless required; use narrow path checks and warn before broad scans.
- Say other current docs do not override the generic Git policy, but add adjacent release/upload requirements:
  - `Release/RELEASE_AGENTS.md` routes release/version/tag/upload work to release docs.
  - `Release/PROJECT_GUIDELINES_INSTRUCTIONS.md` requires staged standalone refresh for runtime-facing changes.
  - `Release/Steam/STEAMWORKS_UPLOAD_AND_TESTING_INSTRUCTIONS.md` is the Steam upload/branch/live-build source; upload root `Saved/StagedBuilds/Windows`, not inner `T66`, verify appmanifest/branch/tag/working tree.
  - `Demo/DEMO_AGENTS.md` and `Demo/DEMO_RELEASE_INSTRUCTIONS.md` are separate demo release/upload lane.
  - `Model Generation/MODEL_GENERATION_AGENTS.md` says not to commit live secrets or pod-local access material.
  - `Model Generation/Rigging and Animation/RIGGING_ANIMATION_AGENTS.md` says not to commit extracted vendor files.

Known risks:
- The Steamworks doc includes a historical "Current Git backup snapshot" from 2026-04-24; answer should label it as Steam operational memory, not current Git truth.
- Avoid implying Steam upload is automatic with Git push; it is a separate release operation.

Verification:
- Targeted `rg` over live `AGENTS.md`, `*_AGENTS.md`, `Release`, and `Demo` docs.
- Direct line-number reads of the relevant root, release, Steamworks, and demo instruction files.
- No build/cook/staged validation needed because this is read-only.

Reviewer request:
- Check whether the answer scope is correct, whether any important live instruction file is missing, and whether the planned answer overstates or understates the relationship between Git commit/push, version tags, staged standalone refresh, and Steam upload.

</review_packet>
