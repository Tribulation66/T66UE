# FriendslopStyle Screen Folders

Every FriendslopStyle screen owns a folder under:

`UI/FriendslopStyle/Screens/<Screen>/`

If the folder does not exist when a new screen starts, create it before any
generation or implementation work. Use `Screens/MainMenu/` as the working
example, but do not copy Main Menu's family names unless the new screen really
has the same structure.

## Required Starting Point

The user provides the visual family breakdown for each new screen. Record it in
the screen folder's `README.md` and `element_manifest.md` before launching
imagegen workers.

The family breakdown must state:

- Family name.
- What region of the reference it covers.
- Which visible elements it includes.
- Which elements are live text/data/icons and must not be baked into assets.
- Which reference or textless family crop will be used as worker context.

Family count is variable per screen. Main Menu has five families; another
screen may have three, six, or another count. The number of local Codex CLI
workers is driven by the number of visual `FAIL` families that require
generation in that iteration: one worker per failed family.

## Folder Contents

Keep screen-owned docs and static artifacts in the screen folder:

- `README.md`
- `geometry.md`
- `geometry_overlay.png`
- `element_manifest.md`
- `component_contract_current.md`
- `slice_specs.md`
- `checklist.md`
- `visual_scorecard_template.md` or the current visual review template
- Optional subfolders such as `slice_artifacts/` for screen-specific contact
  sheets and slice proof images

Runtime/pass outputs belong in the matching screen output roots:

- `Saved/Codex/UI/FriendslopStyle/<Screen>/`
- `SourceAssets/UI/FriendslopStyle/<Screen>/`
- `RuntimeDependencies/T66/UI/FriendslopStyle/<Screen>/`

Top-level `Checklists`, `Elements`, `Geometry`, and `SliceSpecs` under
`UI/FriendslopStyle/` are not the target for new screen-owned files. Treat them
as historical or transitional unless a current instruction explicitly says
otherwise.
