# Replacement Prompt For Parallel UI Chats

Use this prompt when redirecting a parallel chat:

```text
We discovered a hard process error in the previous UI pass: do not style screens from a general main-menu feel. For every screen, modal, HUD overlay, tab, or mini-game UI you work on, you must first create a screen-specific generated reference image.

Read these files first:
- C:\UE\T66\UI\README.md
- C:\UE\T66\UI\SCREEN_WORKFLOW.md
- C:\UE\T66\Docs\UI\MASTER.md
- C:\UE\T66\Docs\UI\UI_Screen_Reference_Pipeline.md
- C:\UE\T66\Docs\UI\UI_Screen_Intake_Template.md

The single canonical style anchor is:
- C:\UE\T66\UI\screens\main_menu\reference\canonical_reference_1920x1080.png

Deploy agents immediately if your assignment covers more than one independent screen, modal, HUD overlay, tab, asset family, or review task. Use explicit ownership:
- capture/layout agent owns screenshots and layout_list.md files
- reference-gate agent owns image-generation inputs and reference validation
- asset agent owns component/state checklists and sprite-family validation
- review agent owns packaged screenshot comparison and blocker classification

Do not assign two agents to edit the same source file or asset folder. Every agent must follow the same reference gate below.

Use Codex-native image generation only. Do not use legacy browser-automation generation, request manifests, token-driven local services, or removed external image-generation tooling.

Reference generation is not completion. Do not finish your answer after generating references. Unless blocked, keep going through sprite/component generation, runtime implementation, packaged capture, and packaged review.

For each assigned screen:
1. Create/use C:\UE\T66\UI\screens\<screen_slug>\.
2. Capture the current packaged/runtime screenshot of that exact screen into current\YYYY-MM-DD\.
3. Write layout\layout_list.md listing all regions, controls, panels, live-data wells, modal variants/tabs, and required states.
4. Feed exactly these three inputs to image generation: the canonical main-menu anchor, the current target screenshot, and the layout list. Generate the screen-specific target reference and save it as reference\canonical_reference_1920x1080.png.
5. Do not begin sprite generation or Unreal styling until that target reference exists and preserves the target layout in the main-menu style.
6. Create the element/state checklist. Generate scene plates/component boards/buttons/panels/icons from image generation until the checklist is complete.
7. No manual pixel repair, masking, cover patches, clone painting, or screenshot cleanup. If an asset is wrong, regenerate it.
8. Raw non-canonical generated output may not become a production reference, sprite sheet, scene plate, slice, or runtime asset. For acceptable landscape outputs, archive the raw source and normalize a copy with `python C:\UE\T66\Scripts\InvokeDeterministicResample.py <raw_image.png> <normalized_output.png> --target-width 1920 --target-height 1080`. Reject and regenerate square, portrait, badly framed, or structurally cropped outputs.
9. Build the runtime from individual assets with real controls and live/localizable text/data.
10. Capture packaged runtime with `C:\UE\T66\Scripts\CaptureT66UIScreen.ps1` so the game opens on display 1 and rejects wrong-size baseline captures. Save outputs under outputs\YYYY-MM-DD\ and review notes under review\. Before final approval, also validate the supported aspect buckets: `16:9`, `16:10`, ultrawide `21:9`, and one smaller/windowed size when supported.

If your current work skipped the screen-specific generated reference step, stop and restart that screen from step 1. Do not continue polishing the current implementation.

Only report the assigned screen complete after all ten steps are done. If you cannot continue, report `blocked`, name the exact missing artifact or failing command, and state the next required step.
```

