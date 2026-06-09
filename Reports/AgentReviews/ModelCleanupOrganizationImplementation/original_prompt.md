# Original User Request

Ok go for it do the cleanup and then add somewhere maybe under read.me in the model generation the organization and cleanup approach for future models, so we dont need to manually do this everytime

# Task Contract

Working task:
Operator: Codex
Validator: Claude if the local helper is available and billing preflight is clean
Scope: perform the approved model cleanup, starting with audit-gated unused runtime models and generated model-output cleanup, then add durable cleanup/organization guidance under Model Generation so future model passes use the same approach automatically.
Stop condition: cleanup is completed only for assets/folders proven safe by the repo gates, documentation is updated, affected Unreal assets/data are verified, and any skipped candidates are reported with reasons.

# Rules And Constraints

- Codex is Operator; Claude is Validator.
- Claude is read-only in this pass.
- Runtime `Content/` assets may only be deleted after Unreal package reference proof and text/data/binary reference proof.
- Do not delete whole hero folders unless the current audit marks them as whole-folder candidates.
- Do not use broad Git/LFS scans over `Content/`.
- Generated model-output folders under `Model Generation/Runs`, `Scenes`, `Archive`, `Reference`, and one-off batch folders are cleanup targets only after imported assets are verified or rejected and durable summaries/manifests are preserved.
- Use Unreal asset APIs for runtime package deletion, not raw filesystem deletion.
- Add reusable Model Generation cleanup/organization guidance so future model imports do not require manual rediscovery.
