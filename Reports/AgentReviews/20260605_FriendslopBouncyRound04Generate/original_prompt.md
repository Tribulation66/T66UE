# Original Prompt And Task Contract

User request:

> No do not generate variation around the same chrome already produced, I want unique variations continue doing one per codex cli's deploy them for the image generation and then close them, where we get to the look, generate another 5 images, all with bouncy and rubber concept but these different themes I mentioned, do one which is like dungeon theme as well and one that is like guild theme like isekai guild theme. But figure out how to keep the same bouncy identity

Working task:
Operator: Codex
Validator: Claude
Scope: Generate five new Round04 FriendslopStyle main-menu references, one fresh Codex CLI worker per image, preserving the T66 main-menu layout/content and `Chadpocalypse`, with bouncy glossy rubber/plastic as the shared identity but unique themed variations: midnight, post-apocalyptic, bloody, dungeon, and isekai guild. No runtime UI implementation, no Unreal import, no source code edit, no Git operation.
Stop condition: five saved PNGs, all workers exited/closed, contact sheet, manifest/QA notes, Claude validation, and token reporting.

Relevant live artifacts:

- Baseline screenshot: `C:\UE\T66\Saved\Codex\UI\MainMenu\baseline_capture.png`
- Structural inventory: `C:\UE\T66\UI\Geometry\main_menu_structural_inventory.md`
- Prior Round03 folder: `C:\UE\T66\UI\FriendslopStyle\Reference\MainMenu\Round03`

Process:

- Use the approved account-backed built-in imagegen path through separate local Codex CLI workers.
- Do not use `OPENAI_API_KEY` or OpenAI API image scripts.
- Attach the baseline screenshot to each worker.
- Each image must use a fresh `codex exec` process and must not copy prior generated outputs.
- Close/verify all worker processes are exited before final response.

Important interpretation:

- The shared identity is not the same chrome reused five times.
- The shared identity is the physical language: glossy squishy rubber/plastic material, rounded/circular/pill geometry, soft inflated edges, and implied bounce/jiggle/rebound behavior.
- The five themes must be unique reinterpretations of that physical language.

Process gates:

```text
PPF CHECK
Objective: Generate five Round04 internal bouncy-rubber main-menu references.
Proven process: AGENTS.md Image generation row plus the local Codex CLI worker pattern used in Round03.
My planned implementation: Five self-contained prompt contracts, one fresh `codex exec` worker per image, account-backed built-in imagegen only, baseline screenshot attached for layout, outputs saved under UI/FriendslopStyle/Reference/MainMenu/Round04.
Same method class: YES
If NO, why: Not applicable.
User approval required before proceeding: NO
Verification evidence: saved PNGs, exited workers, unique hashes, contact sheet, manifest/QA notes, Claude validation.
```

```text
ARTIFACT PARITY GATE
Reference artifact/category: current T66 main-menu layout/content screenshot.
Role: Primary
Required: YES
Planned artifact/path: baseline screenshot attached to every worker.
Status: SAME
Evidence: existing baseline capture and structural inventory.

Reference artifact/category: bouncy rubber/plastic identity.
Role: Primary
Required: YES
Planned artifact/path: five prompts keep glossy squishy rubber/plastic, rounded geometry, and bounce-motion notes as the constant.
Status: EQUIVALENT
Evidence: prompt contracts and contact sheet QA.

Reference artifact/category: unique theme atmospheres.
Role: Primary
Required: YES
Planned artifact/path: midnight, post-apocalyptic, bloody, dungeon, isekai guild.
Status: EQUIVALENT
Evidence: prompt contracts and final visual inspection.
```

```text
MECHANISM MANIFEST
Reference/source: User feedback on bouncy identity.
Required mechanisms:
  1. Bouncy identity: glossy rubber/plastic material, circular/pill geometry, implied rebound/jiggle. Evidence: every prompt and visual QA.
  2. Unique variation: no shared chrome clone; each theme must reinterpret bouncy material differently. Evidence: contact sheet and notes.
  3. Layout preservation: same current main-menu regions/content. Evidence: visual inspection.
  4. Worker isolation: fresh CLI per image, all closed before final. Evidence: worker logs/process checks.
```

Question for Claude:

Before Codex runs generation, identify any missed constraints or prompt risks that would cause the next pass to reuse the same chrome, lose the bouncy identity, or drift into non-HD/gritty themes.
