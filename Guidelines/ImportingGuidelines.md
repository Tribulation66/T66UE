# Importing Guidelines

## Overview

All 3D model imports still begin as vanilla Unreal Interchange imports, but the project importers now finish the lighting/material pass automatically in the same run.

Current rule:

1. **Import** the source GLB/FBX with the project import script.
2. **Auto-convert** the imported materials to the correct Unlit master material immediately.
3. **Reload the affected DataTable** if the imported assets are referenced from CSV-backed gameplay data.

The standalone unlit scripts are still kept as repair tools, but they are no longer the primary day-to-day workflow.

---

## Source Files

Place source files under `SourceAssets/Import/` and organize them by category:

```text
SourceAssets/Import/
├── Props/
│   ├── Tree2.glb
│   ├── Rock.glb
│   ├── Grass.glb
│   ├── Bush.glb
│   └── Boulder.glb
├── Interactables/
│   ├── Fountain.glb
│   ├── BlackBag.glb
│   ├── RedBag.glb
│   ├── YellowBag.glb
│   └── WhiteBag.glb
└── Enemies/
    ├── BlackGoblinRun.fbx
    ├── RedGoblinRun.fbx
    ├── YellowGoblinRun.fbx
    └── WhiteGoblinRun.fbx
```

---

## Scripts

| Script | Purpose |
|---|---|
| `Scripts/ImportStaticMeshes.py` | Import GLB files as static meshes, flatten Interchange output, and immediately convert imported GLB materials to Unlit |
| `Scripts/RunImportPropsAndExit.py` | Run the GLB importer against only the current prop entries in a full editor session, then quit |
| `Scripts/ImportSkeletalMeshes.py` | Import FBX skeletal meshes/animations and immediately convert imported FBX materials to Unlit |
| `Scripts/MakeGLBImportsUnlit.py` | Repair/re-run tool for GLB imports already in the project |
| `Scripts/MakeCharacterMaterialsUnlit.py` | Repair/re-run tool for FBX character imports already in the project |
| `Scripts/RepairStaticMeshImportBatch.py` | Re-run the GLB repair pass for the current prop/interactable batch without reimporting |
| `Scripts/VerifyImportBatch.py` | Validate imported meshes, material parents, DataTables, and fountain defaults; writes `Saved/Logs/VerifyImportBatch.json` |
| `Scripts/SetupPropsDataTable.py` | Reload `DT_Props` from `Content/Data/Props.csv` |
| `Scripts/SetupCharacterVisualsDataTable.py` | Reload `DT_CharacterVisuals` from `Content/Data/CharacterVisuals.csv` |
| `Scripts/SetupItemsDataTable.py` | Reload `DT_Items` from `Content/Data/Items.csv` |

---

## Static Mesh Imports (GLB)

### How to use `ImportStaticMeshes.py`

1. Place the `.glb` files in `SourceAssets/Import/<Category>/`.
2. Edit the `IMPORTS` manifest at the top of `Scripts/ImportStaticMeshes.py`.
3. Run in Unreal Editor:
   - `py "C:/UE/T66/Scripts/ImportStaticMeshes.py"`
   - or headless full-editor props-only batch: `UnrealEditor.exe <uproject> -ExecutePythonScript="C:/UE/T66/Scripts/RunImportPropsAndExit.py"`
4. If props were added/changed in `Content/Data/Props.csv`, run:
   - `py "C:/UE/T66/Scripts/SetupPropsDataTable.py"`

### What the script does

For each GLB entry it will:

- clean the destination import artifacts before reimporting, so stale textures/materials cannot survive under the same gameplay asset path
- import the GLB through Interchange
- flatten the expected static mesh path so runtime code can load `/Game/.../AssetName`
- find imported material instances under the flattened mesh layout and rebind the flattened mesh slots to them
- reparent imported GLB materials to `M_GLB_Unlit`
- preserve the imported `BaseColorTexture`, with `DiffuseColorMap` accepted as a fallback when the source import came through the FBX-style parameter layout
- apply source-faithful static-mesh build settings for GLB gameplay assets: full-precision UVs, no generated lightmap UVs, and preserved imported normals/tangents
- optionally apply per-entry material overrides such as `Brightness` and `Tint`
- optionally apply per-entry texture-parameter overrides such as disabling mip generation on `BaseColorTexture` for atlas-style props
- save the updated assets

### Manifest override rule

`Scripts/ImportStaticMeshes.py` supports three optional override blocks per manifest entry:

- `material_overrides`: per-material-instance values such as `Brightness` and `Tint`
- `texture_parameter_overrides`: per-texture settings looked up through a material parameter such as `BaseColorTexture`
- `mesh_build_settings_overrides`: per-static-mesh build flags if a specific asset needs to diverge from the default GLB gameplay mesh policy
- `cleanup`: pre-import cleanup strategy. Default `{"mode": "asset_subtree"}` deletes the flat gameplay asset plus its same-named imported subtree before reimport. Asset-specific folders such as loot bags can use `{"mode": "dest_dir"}` to purge the whole destination folder first.

Use texture-parameter overrides when a GLB's base-color atlas smears across the model in-game. Current example: `Tree2` disables mip generation on `BaseColorTexture` and sets `never_stream = true` so bark/leaf colors stay separated at gameplay distance.

### Important reimport rule for GLBs

Do not trust `replace_existing` alone for GLB reimports into an already-populated destination.

Observed failure mode:

- the flat gameplay mesh/material package updates
- but old nested `Texture2D` / `MaterialInstance` assets can survive underneath the destination folder
- Unreal then keeps rendering stale source art even though the current GLB looks correct in Blender

Concrete symptom:

- a clean raw import of the current source GLB produces one texture/material name
- the live gameplay asset still points at an older texture/material name under the same folder

Project rule:

- GLB reimports must be **clean reimports**
- let `ImportStaticMeshes.py` delete the old subtree first
- if you are diagnosing a mismatch, compare the live mesh-bound texture asset against a clean raw import before blaming the source file

### Important execution rule for GLB mesh repairs

If a GLB repair needs to touch **static mesh build settings**, run the repair through **`UnrealEditor.exe -ExecutePythonScript=...`**, not `UnrealEditor-Cmd.exe`.

Reason:

- `MakeGLBImportsUnlit.py` itself is safe in either path
- but the static-mesh build-setting write requires `StaticMeshEditorSubsystem`
- that subsystem is available in the full editor process and unreliable/unavailable in the commandlet path

Practical rule:

- **Material-only repair**: commandlet is acceptable
- **Anything that fixes atlas fidelity / UV precision / imported mesh build settings**: use full editor executable

### Current GLB destination patterns

**Props**

| Asset | Expected Path |
|---|---|
| `Tree2` | `/Game/World/Props/Tree2` |
| `Rock` | `/Game/World/Props/Rock` |
| `Grass` | `/Game/World/Props/Grass` |
| `Bush` | `/Game/World/Props/Bush` |
| `Boulder` | `/Game/World/Props/Boulder` |

**Interactables**

| Asset | Expected Path |
|---|---|
| `Fountain` | `/Game/World/Interactables/Fountain/Fountain` |
| `SM_LootBag_Black` | `/Game/World/LootBags/Black/SM_LootBag_Black` |
| `SM_LootBag_Red` | `/Game/World/LootBags/Red/SM_LootBag_Red` |
| `SM_LootBag_Yellow` | `/Game/World/LootBags/Yellow/SM_LootBag_Yellow` |
| `SM_LootBag_White` | `/Game/World/LootBags/White/SM_LootBag_White` |

### GLB material rule

GLB imports must end up parented to:

- `M_GLB_Unlit`

The texture parameter name for this path is:

- `BaseColorTexture`

If a GLB import already exists and its materials need to be repaired, run `Scripts/RepairStaticMeshImportBatch.py` for the current batch or `Scripts/MakeGLBImportsUnlit.py` for a broader manual repair pass.
If the repair includes mesh build settings, prefer `UnrealEditor.exe -ExecutePythonScript="C:/UE/T66/Scripts/RepairStaticMeshImportBatch.py"`.

---

## Skeletal Mesh Imports (FBX)

### Critical rules

1. **Each FBX gets its own destination folder.**
2. **Do not manually rename or move SkeletalMesh, Skeleton, or AnimSequence assets after import.**
3. **Let Interchange create the asset set in-place.**
4. **Update `CharacterVisuals.csv` to point at the real imported asset paths, then reload the DataTable.**

### How to use `ImportSkeletalMeshes.py`

1. Place the `.fbx` files in `SourceAssets/Import/Enemies/` (or the relevant source folder).
2. Edit the `IMPORTS` manifest in `Scripts/ImportSkeletalMeshes.py`.
3. Run in Unreal Editor:
   - `py "C:/UE/T66/Scripts/ImportSkeletalMeshes.py"`
4. Update `Content/Data/CharacterVisuals.csv` if needed.
5. Reload the table:
   - `py "C:/UE/T66/Scripts/SetupCharacterVisualsDataTable.py"`

### What the script does

For each FBX entry it will:

- import the SkeletalMesh, Skeleton, AnimSequence, PhysicsAsset, textures, and materials
- keep each FBX inside its own destination folder
- reparent imported FBX materials to the project's Unlit character master
- preserve or recover the imported `DiffuseColorMap`
- ensure the character masters are valid for skeletal rendering (`used_with_skeletal_mesh = true`)
- save the updated assets

### Current goblin destination patterns

| Visual ID | Skeletal Mesh | Animation |
|---|---|---|
| `GoblinThief_Black` | `/Game/Characters/Enemies/GoblinThief/Black/BlackGoblinRun.BlackGoblinRun` | `/Game/Characters/Enemies/GoblinThief/Black/BlackGoblinRun_Anim.BlackGoblinRun_Anim` |
| `GoblinThief_Red` | `/Game/Characters/Enemies/GoblinThief/Red/RedGoblinRun.RedGoblinRun` | `/Game/Characters/Enemies/GoblinThief/Red/RedGoblinRun_Anim.RedGoblinRun_Anim` |
| `GoblinThief_Yellow` | `/Game/Characters/Enemies/GoblinThief/Yellow/YellowGoblinRun.YellowGoblinRun` | `/Game/Characters/Enemies/GoblinThief/Yellow/YellowGoblinRun_Anim.YellowGoblinRun_Anim` |
| `GoblinThief_White` | `/Game/Characters/Enemies/GoblinThief/White/WhiteGoblinRun.WhiteGoblinRun` | `/Game/Characters/Enemies/GoblinThief/White/WhiteGoblinRun_Anim.WhiteGoblinRun_Anim` |

### FBX material rule

FBX character imports must end up parented to:

- `M_Character_Unlit`
- or `M_FBX_Unlit` when that parameter layout is the better fit

The texture parameter name for this path is:

- `DiffuseColorMap`

If an FBX import already exists and its materials need to be repaired, run `Scripts/MakeCharacterMaterialsUnlit.py`.
If a character renders fully black after reparenting, the first thing to verify is that `M_Character_Unlit` / `M_FBX_Unlit` still have `used_with_skeletal_mesh = true`; the repair script now enforces this automatically.

---

## Material Rules

### Import scripts DO

- import meshes, textures, materials, skeletons, and animations
- flatten GLB Interchange mesh placement where required
- reparent imported materials to the correct Unlit master material
- preserve the imported color texture parameter
- save updated assets

### Import scripts DO NOT

- hand-author gameplay data rows for you
- rename/move skeletal asset sets after Interchange
- change unrelated project materials outside the scan roots

### Canonical unlit targets

| Asset type | Master material | Texture param |
|---|---|---|
| FBX characters | `M_Character_Unlit` or `M_FBX_Unlit` | `DiffuseColorMap` |
| GLB props/interactables | `M_GLB_Unlit` | `BaseColorTexture` |
| Runtime/procedural world materials | `M_Environment_Unlit` | `DiffuseColorMap` |

---

## Workflow Summary

### Static meshes (GLB)

```text
1. Put GLBs in SourceAssets/Import/<Category>/
2. Update Scripts/ImportStaticMeshes.py
3. Run ImportStaticMeshes.py
4. Update Props.csv if needed
5. Run SetupPropsDataTable.py
6. Run VerifyImportBatch.py
7. Verify visually in editor
```

### Skeletal meshes (FBX)

```text
1. Put FBXs in SourceAssets/Import/
2. Update Scripts/ImportSkeletalMeshes.py
3. Run ImportSkeletalMeshes.py
4. Update CharacterVisuals.csv if needed
5. Run SetupCharacterVisualsDataTable.py
6. Run VerifyImportBatch.py
7. Verify visually in editor
```

### CSV-backed gameplay data only

```text
If Items.csv changes:
1. Update Content/Data/Items.csv
2. Run SetupItemsDataTable.py
3. Verify DT_Items no longer contains removed rows
```

### Repair / re-run only

```text
GLB materials only:        Scripts/MakeGLBImportsUnlit.py
Current static-mesh batch: Scripts/RepairStaticMeshImportBatch.py
FBX character materials:   Scripts/MakeCharacterMaterialsUnlit.py
Verification report:       Scripts/VerifyImportBatch.py
```

For static-mesh atlas fidelity fixes, run the repair step with the full editor executable:

```text
UnrealEditor.exe <uproject> -ExecutePythonScript="C:/UE/T66/Scripts/RepairStaticMeshImportBatch.py"
```
