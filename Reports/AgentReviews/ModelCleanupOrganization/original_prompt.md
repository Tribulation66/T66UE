# Original User Request

Ok now I want to have better cleanup and orgnaization for my models, what do you and claude suggest we do about models that are no longer used, and not just in theory but the models we have int he game right now that are not used what should be done?

# Task Contract

Working task:
Operator: Codex
Validator: Claude if the local helper is available and billing preflight is clean
Scope: inspect the live T66 model/import state, identify currently unused model assets or unused imported model groups, and produce a concrete cleanup/organization recommendation without deleting or changing assets.
Stop condition: answer gives repo-grounded categories, specific examples/paths, recommended action per category, caveats, and Claude/Codex token reporting.

# Relevant Repo Rules

- This is a read-only recommendation task. Do not delete or move assets.
- Start from live repo state, current model-generation instructions, current Unreal data references, and current audit scripts.
- For generated model cleanup, route through `Model Generation/MODEL_GENERATION_AGENTS.md`, `Model Generation/Instructions/README.md`, `00_MODEL_GENERATION_ROUTING_INSTRUCTIONS.md`, and relevant import/cleanup instructions.
- Model-generation generated runs are cleanup targets after imported assets are verified or rejected; runtime assets require Unreal package referencer and text/data reference proof before deletion.
- Do not use broad Git/LFS scans over `Content/`.
- Report concrete candidate categories and what should be done with each.
