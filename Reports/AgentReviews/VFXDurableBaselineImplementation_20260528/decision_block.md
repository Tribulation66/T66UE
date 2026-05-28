# VFX Durable Baseline Decision Block

Date: 2026-05-28

## Working Goal

Implement and locally commit a durable VFX-only baseline, including the repeated-question prevention change in `AGENTS.md`, VFX process infrastructure, combat-VFX-local generated asset policy, infrastructure-only Hero 1 DOT/Pierce/Bounce support, production-path automation proof, and commit-gate evidence.

## User Decisions

- DOT/Pierce/Bounce scope: infrastructure only now.
- Normal proof: production-path automation proof first.
- Generated asset policy: combat-VFX-local policy now, repo-wide generated asset policy out of scope for this pass.
- Idol overlay scope: architecture-only seam document now; no idol VFX assets, active idol rows, or implemented idol behavior.
- Existing generated/runtime VFX data/assets: include the approved Hero 1 AOE production binding/runtime assets in the local VFX-only commit after staged-diff review; do not author new visual VFX assets in this pass.
- Automated best-frame selection: in scope because the user listed it under "What Is Missing"; implement as opt-in evidence tooling only, preserving current manual/default behavior.
- Root `AGENTS.md` update: in scope because the user explicitly said the repeated-question goal change needs to be reflected in `AGENTS.md`.
- Commit policy: local commit only, no push.
- Include root `AGENTS.md` changes.
- Keep durable proof/handoff docs and exclude/delete transient review attempts.
- Commit generated/runtime VFX data/assets relevant to this VFX baseline.
- Use `Gameplay/Combat/VFX_PROCESS_INDEX.md` as the process index filename.

## Authorization Source Excerpts

- Root `AGENTS.md` scope: user said, "Ok this goal change needs to be reflected in the agents.md file, because this occurred in a different agent as well, with the repeated questions."
- Implementation permission and narrowed decisions: user said, "1. Infrastructure only. 2. B. 3. B. Ok you have permission to go with the implementation now."
- Current root process-router context: the user supplied the current `AGENTS.md` instructions for `C:\UE\T66`, including the Report Artifact Routing section and the requirement to route durable report/proof artifacts under `Reports/`.
- Combat router/process note scope: the implementation permission covered the requested durable VFX process infrastructure; `Gameplay/Combat/MASTER_COMBAT.md` is updated only as the Combat folder router that points future VFX work to `VFX_PROCESS_INDEX.md` and the production-path proof contract.

## Continuation Rule

On automatic continuation with no new user input, do not reprint the decision questions. Reference this file and continue the active implementation flow unless a new blocker changes gameplay behavior, repo policy, commit scope, or irreversible assets.
