# ChatGPT Image Production Workflow

This file describes the current workflow for generating reference-derived UI component sheets through ChatGPT image generation, saving the results locally, and preparing them for Unreal reconstruction work.

This workflow is reference-first. It is not a style-exploration process.

## Purpose

Use this workflow when a screen already has an approved visual reference and Codex needs to derive a reusable UI kit from it.

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
- Bridge setup readme: `C:\UE\T66\Tools\ChatGPTBridge\README.md`
- Chrome launcher: `C:\UE\T66\Tools\ChatGPTBridge\launch_debug_chrome.ps1`

## Core Rules

- Start from an approved reference image, not from imagination.
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
- the exact component family to extract or reconstruct
- the required state set
- whether text must be omitted
- whether icons must be isolated or omitted
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
