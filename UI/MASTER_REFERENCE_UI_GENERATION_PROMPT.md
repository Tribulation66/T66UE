# Master Reference UI Generation Prompt

Use this master prompt for T66 UI work from a reference image. It replaces the old per-screen copy-paste prompt bundles as the active source of truth.

Copy this whole prompt into the target chat, then fill the target-specific fields before the agent starts. Do not remove the contract sections.

```text
T66 Reference UI Generation From Image

Read C:\UE\T66\UI\Processes\SCREEN_MODAL_TASK.md first. Then read C:\UE\T66\UI\Processes\UI_GENERATION.md and follow it as the global authority. This master prompt is self-contained; if an older instruction conflicts with this prompt, follow this prompt for execution order and proof policy.

TARGET FIELDS TO FILL BEFORE STARTING

Target name:
Base screen/modal:
Target state:
Exact target reference image:
Workspace folder:
Runtime asset folder:
Optional same-screen common runtime folder:
Working visual capture command:
Automation capture extra args:
Coordinator worker mode:
Target source files:
- 
Protected/shared scope notes:

NON-NEGOTIABLE EXECUTION CONTRACT

Required preflight:
- Confirm C:\UE\T66\UI\Processes\SCREEN_MODAL_TASK.md exists and read it.
- Confirm C:\UE\T66\UI\Processes\UI_GENERATION.md exists and read it.
- Confirm C:\UE\T66\Scripts\CaptureT66UIScreen.ps1 exists.
- Confirm C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe exists. If it is missing or stale after a runtime-facing change, refresh the staged standalone build before final visual proof.
- Confirm the exact target reference image exists before imagegen or implementation.
- Confirm the exact target reference image dimensions before writing any geometry/occupancy numbers. Do not claim the reference is 1920x1080 unless the file actually is 1920x1080.
- Confirm every listed target source file exists before editing. If one is missing, report the exact path and stop before making changes.
- Open/view the reference image before generating or editing anything.
- If the working visual capture command uses C:\UE\T66\Scripts\CaptureT66UIScreen.ps1, it must explicitly pass -Exe C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe or rely on that same staged executable as the script default.

Scope:
- Work only on the named target and target state.
- Do not broaden into other screens, modals, gameplay systems, shared top-bar systems, or unrelated UI assets.
- If this is a split-state screen, use state-specific folders for state-specific assets and the same-screen Common folder only for truly shared pieces.
- Do not touch sibling state runtime folders.
- Do not touch C:\UE\T66\RuntimeDependencies\T66\UI\Reference\Screens\MainMenu or C:\UE\T66\RuntimeDependencies\T66\UI\Reference\Shared unless the target explicitly is MainMenu/shared chrome.
- Before implementation, write the owned-area/protected-area boundary in Workspace\MANIFEST_MASTER.md. Owned areas are the only areas you may change. Protected/shared areas must be ignored in the difference list unless this exact target explicitly owns them.

Fresh-start rule:
- Start from the reference image and this prompt, not from old accepted/generated chrome.
- Do not copy archived V2/V3/V4 assets back in as a shortcut.
- If active generated runtime assets already exist for this target, archive them first under Workspace\Archive\PreRun_Reset_<timestamp>\ before replacing them.
- Replace only files owned by this target/pass. Do not broad-delete unknown files, source-authored assets, real reference screenshots, or sibling-state assets.
- Real reference screenshots are never reset or deleted.
- MainMenu is protected unless the user explicitly asks for MainMenu work.

Imagegen execution:
- Use built-in imagegen only.
- Do not use OPENAI_API_KEY, OpenAI SDK scripts, API fallback scripts, or scripts/image_gen.py.
- Do not say generation was skipped because no API key exists. API keys are irrelevant to this workflow.
- Do not say no local output is possible because inline previews are disallowed. Built-in imagegen output counts as generation; save/export/promote the result into the pass candidate path using the available local image-output mechanism.
- If built-in imagegen runs but the environment exposes no local artifact/export path, stop with BLOCKED_TRUE_MISSING_INPUT and include exact evidence. Do not silently skip imagegen or substitute scripts/API fallback.
- First imagegen pass must be reference-derived from the exact target reference screenshot, not a word-only prompt.
- Generate a clean text-free sprite/component sheet first. It must remove labels, numbers, title text, player names, stat values, live data, screenshots, portraits, and runtime text while preserving the UI chrome families.
- Do not manually crop the original reference screenshot, do not use Pillow/pixel surgery to erase text, and do not patch screenshot pixels into runtime assets.
- After the text-free sheet passes the quality gate, create accepted runtime elements as backgroundless PNGs or repo-supported SVGs. Runtime PNGs are preferred unless the source code already supports SVG for that element.
- Slicing your own generated text-free component sheet is allowed only when the sheet is already clean and text-free. Slicing the original reference screenshot is not allowed.
- Do not use a crowded visual reference sheet as the runtime extraction source. If gutters, proportions, shadows, or crop boundaries are not exact and uniform, reject it for slicing.
- For accepted runtime assets, use imagegen to produce either one isolated component at a time or one uniform component-family row at a time. Uniform rows must have identical cell sizes, fixed gutters, transparent/backgroundless output, and a written slice contract before deterministic cropping.
- Deterministic slicing is allowed only from those approved isolated outputs or uniform rows. Do not manually guess crop rectangles from a mixed sheet.

Atomic component rule:
- The generated sheet must break the UI into empty atomic components, not baked mini-layouts.
- A panel asset is only an empty panel/shell/interior. A row asset is only an empty row background. A slot asset is only one empty slot/frame. A button asset is only one empty button state. A tab, dropdown, scrollbar, divider, icon frame, meter track, and ornament are each separate components.
- Parent components must not contain child controls baked into them. Panels must not include baked slots, buttons, tabs, labels, icons, rows, list items, portraits, or embedded child layouts.
- Buttons must not include live labels, prices, icons, or adjacent controls. Slots must not include portraits, plus signs, lock icons, labels, or selection data unless the target specifically asks for that static icon asset.
- If imagegen creates a panel with embedded slots/buttons/rows/icons/text, reject the sheet and regenerate with stricter atomic-component wording. Do not slice a baked composite and call the child controls separate.
- For every component on the accepted sheet, record the component name, atomic role, parent/child status, transparent/backgroundless expectation, and resize contract in Workspace\MANIFEST_MASTER.md before slicing or promotion.

Sprite sheet quality gate before assembly:
- Before slicing, assembling, promoting, or wiring any generated UI element, open/view the exact reference screenshot and the generated text-free sprite/component sheet side by side.
- Compare at 100% scale and fit-to-screen scale.
- Check color temperature, brightness, paper tone, wood tone, metal tone, border thickness, corner ornament density, grain/noise amount, bevel strength, darkness/contrast, geometry, proportions, and whether the generated sheet preserved the reference UI families.
- The sheet fails if it is darker, grainier, blurrier, more ornate, more damaged, flatter, brighter, lower contrast, thicker-bordered, differently colored, or materially different from the reference.
- The sheet fails if it contains labels, numbers, title text, player names, stat values, live data, screenshots, portraits, or runtime text that should remain live.
- The sheet fails if panels/buttons/slots/tabs do not visually correspond to the reference elements. Do not accept a sheet just because it looks polished.
- The sheet fails if parent shells include baked child controls or if any component boundary would force the runtime layout to inherit the generated sheet's mini-layout.
- Complete an acceptance table before approval: color temperature, brightness, paper tone, wood tone, metal tone, border thickness, ornament density, grain/noise, bevel strength, contrast, geometry/proportions, component correspondence, text-free status, and atomic-component status. Every row must pass.
- If the sheet fails, archive it under Workspace\Archive\Rejected\Pass_XX with a short rejection note and generate a new sheet.
- Do not assemble the screen from a failed sheet. A bad sprite sheet cannot be fixed by layout.
- Record the accepted sheet path, rejected sheet paths, and concrete reasons the accepted sheet passed in Workspace\MANIFEST_MASTER.md.

Screen hierarchy, containment, and reference occupancy gate:
- Before assembly, create a 1920x1080 hierarchy map from the reference.
- The map must list every parent component and its children: screen frame, top-level panels, cards, rows, tabs, dropdowns, slots, buttons, scrollbars, dividers, icons, and live-text regions.
- For each parent and child, record x, y, width, height, padding, spacing, anchor, and containment relationship.
- If the reference image is not exactly 1920x1080, record two rect sets for every element: native reference pixels and normalized 1920x1080 pixels. Use scaleX = 1920 / nativeWidth and scaleY = 1080 / nativeHeight. The implementation must target the normalized 1920x1080 rects, while the manifest must preserve native rect evidence.
- Do not invent or estimate 1920x1080 coordinates without noting the native reference size and scale factors.
- The 1920x1080 reference image is the source of truth for how much screen space each owned element occupies. The implementation must place each owned top-level panel, card, row, slot, and button in the same screen area as the reference.
- Apply C:\UE\T66\UI\Processes\LAYOUT_AND_SIZING.md for runtime sizing. Top-bar screens must fill the viewport below the shared top bar; parent panels must grow, compact, stack, or scroll when children do not fit; do not use a fixed 1920x1080 shell as the runtime root.
- For every owned component, record a reference rect and intended implementation rect: name, role, parent, x, y, width, height, anchor, padding, spacing, resize contract, and whether it is fixed/3-slice/9-slice/tiled.
- Top-level owned panels and footer/header bars should be within 12 px or 2 percent of the reference rect, whichever is larger. Buttons, slots, tabs, rows, and child controls should be within 8 px or 2 percent, whichever is larger. Intentional reference overlap must be named explicitly.
- Every child must visually fit inside its designated parent. Buttons inside cards must stay inside the card. Rows inside panels must stay inside the panel. Icons and controls must not hang outside their intended shells unless the reference clearly shows intentional overlap.
- If a child does not fit, resize or reposition the parent/child hierarchy to match the reference. Do not leave overflow and call it an art issue.
- Do not fix containment by making text or buttons tiny if the reference uses a larger parent. Grow or reposition the parent component when that is what the reference implies.
- Action buttons that belong to a card, slot, row, or panel are part of that parent component. If those buttons overflow, the parent component is too small or the internal spacing is wrong.
- Geometry is a binding contract, not a planning note. Do not proceed to polish until the implementation uses the reference rect table as its layout target.
- A screen cannot reach WORKING_VISUAL_PASS or READY_FOR_CENTRAL_BUILD_AND_CAPTURE while any owned top-level element occupies noticeably different screen space, any owned child component overflows its intended parent, intersects unrelated UI, or sits outside its designated card/panel/row.

Art direction restraint:
- Match the reference exactly. Do not upgrade, embellish, or reinterpret it.
- Do not add extra filigree, gems, ornate corner brackets, embossed curls, heavy gold bevels, noisy trim, or decorative density beyond what is visibly present in the reference.
- Simple dark slots must stay simple dark slots. Plain brown buttons must stay plain brown buttons. Quiet panels must stay quiet panels.
- If imagegen creates a fancier but less accurate result, reject it and regenerate with stronger restraint.
- Geometry beats decoration: width, height, spacing, anchoring, panel proportions, card containment, and button sizes must match before polish is accepted.
- If the reference shows a button, skin, card, slot, row, or content state with no current backend/gameplay infrastructure, create it as a visually accurate stub and mark it for later wiring. Backend readiness must not block frontend reference fidelity.

Shared top bar freeze:
- If this screen has a shared top bar/header/nav/currency/avatar/back/settings component, do not edit it and do not generate per-screen replacements for it.
- Top-bar differences are approved out-of-scope shared UI differences unless this exact target is shared top-bar work.

Build policy:
- Do not run C:\UE\T66\Scripts\StageStandaloneBuild.ps1, RunUAT, BuildCookRun, cook, stage, pak, or package for this individual target.
- Individual target work uses WORKING_VISUAL_PROOF from the local development executable and loose RuntimeDependencies. Compare captures with C:\UE\T66\Scripts\CompareUIScreen.ps1. Final packaged proof is a coordinator pass after the batch.
- If Coordinator worker mode is yes, do not run any build or capture command. Stop at READY_FOR_COORDINATOR_SPRITE_REVIEW after generating the atomic sheet, then wait for coordinator approval. After coordinator approval and implementation, stop at READY_FOR_CENTRAL_BUILD_AND_CAPTURE with exact changed files, accepted assets, rect map, and capture command.
- If you changed only PNG/SVG/runtime asset files, skip build and capture directly with the working visual capture command.
- If you changed C++ layout/routing/source, run the normal Unreal build, not UAT:
  & "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" T66 Win64 Development "C:\UE\T66\T66.uproject" -WaitMutex -NoHotReloadFromIDE
- If a normal Unreal build is needed and is already occurring, or UBT reports a mutex/conflict, try up to 10 total build attempts. Wait exactly 1 minute between attempts. Inspect process command lines before the final status. Do not kill active user/agent builds.
- If the build still cannot start after 10 timing/mutex attempts, report TRUE_BLOCKED_BUILD_FAILURE with the exact command, process evidence, and next capture command.
- If a deterministic compile error blocks the normal build outside your target-owned files, report TRUE_BLOCKED_BUILD_FAILURE with exact file/line errors unless the fix is trivial, compile-only, and inside your listed target source files. Do not redesign unrelated screens.

Capture policy:
- Capture at 1920x1080 using the working visual capture command.
- If Coordinator worker mode is yes, do not capture locally. Provide the exact central capture command for the coordinator instead.
- A lower-resolution capture is not final proof.
- CaptureT66UIScreen.ps1 is expected to recover screenshots that Unreal writes under the cooked sandbox mirror before failing. If it still reports timeout, first check whether the requested screenshot was written under the cooked sandbox equivalent:
  C:\UE\T66\Saved\Cooked\Windows\T66\<project-relative-output-path>
- If the sandbox screenshot exists, copy it back to the requested proof path and treat capture as successful after confirming dimensions.
- The helper script C:\UE\T66\Scripts\CaptureT66UIScreen.ps1 is expected to perform this sandbox recovery automatically. If it does not, do the manual check above before reporting capture failure.
- If capture still fails, retry up to 3 times before reporting TRUE_BLOCKED_CAPTURE_FAILURE. Increase delay each time: 3.5 seconds, then 6 seconds, then 10 seconds. Confirm whether the screenshot file exists after each attempt.

Visual iteration loop:
- Create a 1920x1080 geometry map from the reference.
- Create a hierarchy/containment map from the reference.
- Generate and approve the text-free sprite/component sheet through the sprite sheet quality gate.
- In Coordinator worker mode, stop at READY_FOR_COORDINATOR_SPRITE_REVIEW and wait after the sheet is generated and documented. Do not assemble from an unapproved sheet.
- Assemble/wire the screen-owned assets and layout.
- Bind layout to the reference occupancy table. The same screen space used by the reference's owned panels, cards, buttons, slots, and bars must be occupied by the implementation.
- In Coordinator worker mode, stop at READY_FOR_CENTRAL_BUILD_AND_CAPTURE after implementation and do not build or capture.
- Capture the current implementation at 1920x1080.
- Compare reference and capture visually.
- For each capture, create a reference-vs-current audit table: component, reference rect, current rect, delta x/y/w/h, containment status, and pass/fail.
- List concrete remaining differences by asset, hierarchy/containment, layout, color/material, typography/live-data, and approved shared-top-bar/live-data differences.
- VISUALLY CHECK through screenshot if its still different IF IT IS DIFFERENT DO ANOTHER PASS.
- NEEDS_ANOTHER_PASS_CONTINUING is an internal progress state only. It is not an allowed final answer.
- If the screenshot exists and still has unapproved visible differences, immediately start the next pass in this same chat. Do not stop to summarize. Do not hand the work back.
- Do not stop after one pass. Do not stop after imagegen. Do not stop after build. Do not stop after copying assets. Stop only at WORKING_VISUAL_PASS with no unapproved visible differences in the owned area, or at a true blocker status listed below.

Cleanup and manifest:
- Working candidates go under Workspace\Working\Pass_XX\Candidates.
- Accepted runtime assets go only under the target runtime folder, plus the same-screen Common folder only when truly shared within the same base screen.
- Rejected candidates go under Workspace\Archive\Rejected\Pass_XX with a short note saying why they failed.
- Do not leave generated candidates loose in SourceAssets or in the workspace root.
- Maintain Workspace\MANIFEST_MASTER.md with pass number, generated candidate paths, accepted runtime paths, rejected candidate paths, sprite sheet quality gate result, atomic component table, hierarchy/containment map, reference occupancy table, implementation rect audit if captured, source files changed, build command/status if used, capture attempts, screenshot proof path, remaining differences, approved live-data/top-bar differences, and exact next action if not passing.

Allowed final statuses:
- WORKING_VISUAL_PASS
- READY_FOR_COORDINATOR_SPRITE_REVIEW
- READY_FOR_CENTRAL_BUILD_AND_CAPTURE
- TRUE_BLOCKED_BUILD_FAILURE
- TRUE_BLOCKED_CAPTURE_FAILURE
- NEEDS_EXACT_REFERENCE_PHASE0
- BLOCKED_TRUE_MISSING_INPUT

Required final response format:
Status: WORKING_VISUAL_PASS, READY_FOR_COORDINATOR_SPRITE_REVIEW, READY_FOR_CENTRAL_BUILD_AND_CAPTURE, TRUE_BLOCKED_BUILD_FAILURE, TRUE_BLOCKED_CAPTURE_FAILURE, NEEDS_EXACT_REFERENCE_PHASE0, or BLOCKED_TRUE_MISSING_INPUT
Target:
Coordinator worker mode: yes/no
Pass count:
Built-in imagegen used: yes/no, must be yes unless blocked before imagegen
Reference-derived sheet generated: yes/no/path, must be yes/path unless blocked before imagegen
Sprite sheet quality gate: PASS/FAIL, accepted sheet path, rejected sheet paths, acceptance table, and reasons
Atomic component gate: PASS/FAIL, component table path, any baked parent/child failures
Hierarchy/containment/occupancy gate: PASS/FAIL, hierarchy map path, reference occupancy table path, remaining overflow/containment/rect-delta issues
Reference image path: <must exist; provide full path>
Current implementation screenshot path: <must exist for WORKING_VISUAL_PASS; provide full path>
Generated candidate paths:
Accepted runtime asset paths:
Archived/reset asset paths:
Source files changed:
Central capture command if coordinator mode:
Build attempts if source changed: <0 for asset-only, otherwise list 1-10 attempts and result>
Capture attempts: <list attempts and exact output paths; include sandbox recovery path if used>
Working visual screenshot proof: <same as current implementation screenshot path, must exist for WORKING_VISUAL_PASS>
Remaining visual differences:
Approved live-data/top-bar-shared differences:
Next action if not pass:

Final response rules:
- Do not use NEEDS_ANOTHER_PASS_CONTINUING as a final status.
- Do not provide an attempted screenshot path as the current screen. The current implementation screenshot path must exist unless the status is TRUE_BLOCKED_BUILD_FAILURE or TRUE_BLOCKED_CAPTURE_FAILURE.
- If status is READY_FOR_COORDINATOR_SPRITE_REVIEW, built-in imagegen used must be yes, the reference-derived sheet path must exist, the atomic component table must exist, and no runtime assembly/build/capture should have been attempted.
- If status is READY_FOR_CENTRAL_BUILD_AND_CAPTURE, coordinator worker mode must be yes, sprite sheet quality gate must be PASS, atomic component gate must be PASS, hierarchy/containment/occupancy gate must be PASS, source/assets must be implemented, and no build/capture should have been attempted by the worker.
- If status is WORKING_VISUAL_PASS, both the reference image path and current implementation screenshot path must exist, built-in imagegen used must be yes, reference-derived sheet path must exist, sprite sheet quality gate must be PASS, atomic component gate must be PASS, hierarchy/containment/occupancy gate must be PASS, and remaining visual differences must be empty or approved live-data/shared-top-bar differences only.
- If status is TRUE_BLOCKED_BUILD_FAILURE, include exact file/line compile errors or mutex evidence and the 10-attempt build log.
- If status is TRUE_BLOCKED_CAPTURE_FAILURE, include the three failed capture commands, delays used, timeout/errors, sandbox recovery path checked, and proof that no output file exists.
```
