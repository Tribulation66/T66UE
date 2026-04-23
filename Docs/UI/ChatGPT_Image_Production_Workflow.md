# ChatGPT Image Production Workflow

This file describes the fallback workflow for generating reference-derived UI component sheets through the local ChatGPT bridge, saving the results locally, and preparing them for Unreal reconstruction work.

This workflow is reference-first. It is not a style-exploration process.

Native Codex `image_gen` is now the preferred first generation path for sprite-family boards. Use this bridge workflow when native generation is not the right fit for the current request, or when the task is a canonical full-screen style-reference master that depends on multiple explicit reference anchors.

## Purpose

Use this workflow when a screen already has an approved visual reference and Codex needs to derive a reusable UI kit from it.

Prefer the native path first. Fall back to the bridge when the task needs:

- multiple repo-local reference attachments
- a repo-checked request manifest for reproducibility
- explicit API-side controls or attachment behavior not exposed in the local tool surface

Typical outputs:

- top-bar component sheets
- CTA button sheets
- tabs, dropdowns, and toggle sheets
- panel shell sheets
- icon strips
- decorative trim packs

## Canonical Files

- Workflow doc: `C:\UE\T66\Docs\UI\ChatGPT_Image_Production_Workflow.md`
- Bridge server: `C:\UE\T66\Tools\ChatGPTBridge\server.py`
- Bridge request client: `C:\UE\T66\Tools\ChatGPTBridge\request_generate.py`
- Bridge setup readme: `C:\UE\T66\Tools\ChatGPTBridge\README.md`
- Chrome launcher: `C:\UE\T66\Tools\ChatGPTBridge\launch_debug_chrome.ps1`

## Core Rules

- Start from an approved reference image, not from imagination.
- Run the content ownership audit first and identify open apertures, icon sockets, portrait wells, and runtime-text regions before prompting.
- Generate one component family at a time.
- Keep sessions short. If output quality drifts, start a fresh conversation instead of forcing the same one longer.
- Preview one family first before asking for larger batches.
- Do not mix unrelated families such as top bar, panels, and CTA buttons in one request unless the goal is a presentation board.
- For localizable controls, request empty plates without text.
- Request explicit state variants when the control needs them.
- Save the final settled render, not a transient intermediate image.

## Preferred Session Pattern

Use separate focused conversations or short batches for:

1. top-bar elements
2. CTA buttons
3. tabs, toggles, and dropdowns
4. panel shells and row frames
5. icons and decorative accents

Once a family is approved, reuse that approved output as a reference image in the next fresh conversation instead of relying on memory.

## Prompting Rules

Each prompt should specify:

- the attached reference image
- any `content_ownership.json` constraints for runtime-owned interiors
- the exact component family to extract or reconstruct
- the required state set
- whether text must be omitted
- whether icons must be isolated or omitted
- whether portrait, media, or icon wells must remain open or neutral
- whether the output should be transparent-background assets or a clean presentation sheet

Avoid prompts that ask for a whole redesigned screen when the goal is a runtime component kit.

## Recommended Output Forms

Use one of these depending on the step:

- `component board`
  Good for review and family approval.
- `state sheet`
  Good for buttons, tabs, toggles, and selectors.
- `isolated transparent assets`
  Good for final import preparation.

The usual order is:

1. component board for review
2. state sheet for approved families
3. isolated assets for runtime prep

## Button And Panel Policy

For runtime use, prefer:

- text-free button plates
- text-free panel shells
- socket frames or open apertures for runtime-owned portraits, icons, media, and previews
- separate `normal`, `hover`, `pressed`, `disabled`, or `selected` states as needed
- live text and live data in-engine

Do not rely on baked English labels in the shipped runtime assets.

## Validation Rules

After generating a family:

1. inspect the sheet directly
2. reject any drift from the reference material language
3. reject malformed states or inconsistent perspective
4. only then split or import the assets

If the family is close but not stable, start a fresh generation pass instead of over-editing a bad sheet.

## Current Best Practice

- Use the approved screen reference as the artistic authority.
- Generate narrow, focused sheets by family.
- Reset the session when drift starts.
- Promote approved families into a shared UI kit.
- Rebuild the live screen with real widgets and real text.
- Validate the live result against the original reference image.

## Repo-Driven Request Pattern

When the prompt and reference images already live in the repo, prefer the bridge client over manual `curl`.

Use:

- `request_generate.py --prompt-file ... --image ... --image ... --json-out ... --copy-to-dir ...`
- or `request_generate.py --request-file ...`

This keeps:

- the prompt under version control
- the reference-image set explicit
- the request metadata saved locally
- the approved output copied into the target reference pack

For screen-master generation, store the request JSON next to the prompt pack so the style-reference stage can be reproduced from version-controlled inputs.
