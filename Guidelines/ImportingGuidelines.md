# Importing Guidelines

## Overview

All 3D model imports go through a two-step process:

1. **Vanilla import** — bring the GLB/FBX into Unreal, letting the engine create meshes, textures, and materials with no modifications.
2. **Unlit conversion** — run `MakeAllMaterialsUnlit.py` to convert all materials to the project's Unlit shading model.

These two steps are always separate. The import scripts never touch materials.

---

## Source Files

Place all files to import in `SourceAssets/Import/`. Organize by category:

```
SourceAssets/Import/
├── Interactables/
│   ├── ChestBlack.glb
│   └── ...
├── Props/
│   ├── Barn.glb
│   └── ...
├── ArthurIdle.fbx       ← skeletal mesh + idle anim + textures
├── ArthurWalk.fbx       ← walk animation (same rig)
└── (future imports)
```

## Scripts

| Script | Purpose |
|---|---|
| `Scripts/ImportStaticMeshes.py` | Import GLB files as static meshes + flatten Interchange folder structure |
| `Scripts/ImportSkeletalMeshes.py` | Import FBX skeletal meshes + animations (each FBX to its own subdirectory) |
| `Scripts/FlattenInterchangeAssets.py` | Fix Interchange folder nesting for already-imported static mesh assets |
| `Scripts/SetupPropsDataTable.py` | Reload DT_Props DataTable from `Content/Data/Props.csv` |
| `Scripts/MakeAllMaterialsUnlit.py` | Convert all project materials to Unlit (separate step) |

---

## Static Mesh Imports (GLB)

### How to use ImportStaticMeshes.py

1. Place your `.glb` files in `SourceAssets/Import/` under the appropriate subfolder.
2. Open `Scripts/ImportStaticMeshes.py` and edit the `IMPORTS` list at the top. Each entry is a tuple of:
   - **source**: relative path from `SourceAssets/Import/` to the `.glb` file
   - **destination**: `/Game/...` content path where the asset should be placed
   - **name**: desired asset name in the content browser
3. Run in Unreal Editor: `py "C:/UE/T66/Scripts/ImportStaticMeshes.py"`
4. Run `MakeAllMaterialsUnlit.py` as a separate step.

### Interchange folder flattening

UE5's Interchange pipeline creates a nested folder structure when importing GLBs:

```
destination/Name/StaticMeshes/Name.uasset   ← what Interchange creates
destination/Name.uasset                      ← what the C++ code expects
```

`ImportStaticMeshes.py` automatically flattens this after each import. If you import manually or the flatten step fails, run `FlattenInterchangeAssets.py` to fix it.

### Static mesh destination paths

**Interactables** (referenced by C++ interactable classes):
| Asset | Expected Path |
|---|---|
| ChestBlack | `/Game/World/Interactables/Chests/Black/ChestBlack` |
| ChestRed | `/Game/World/Interactables/Chests/Red/ChestRed` |
| ChestYellow | `/Game/World/Interactables/Chests/Yellow/ChestYellow` |
| ChestWhite | `/Game/World/Interactables/Chests/White/ChestWhite` |
| Crate | `/Game/World/Interactables/Crate` |
| Totem | `/Game/World/Interactables/Totem` |
| Wheel | `/Game/World/Interactables/Wheels/Wheel` |

**Props** (referenced by `DT_Props` DataTable via `Content/Data/Props.csv`):
| Asset | Expected Path |
|---|---|
| Barn | `/Game/World/Props/Barn` |
| Hay | `/Game/World/Props/Hay` |
| Hay2 | `/Game/World/Props/Hay2` |
| Tree | `/Game/World/Props/Tree` |
| Tree2 | `/Game/World/Props/Tree2` |

---

## Skeletal Mesh Imports (FBX)

### Critical rules

1. **Each FBX gets its own subdirectory.** Never import two FBXs to the same folder — Interchange crashes on name collisions (duplicate `Image_0`, `Material_0`, etc.).
2. **Never rename or move AnimSequence or Skeleton assets.** The `skeleton` property on AnimSequence is read-only in UE5 Python. Moving an animation breaks its skeleton reference and there is no way to fix it via script.
3. **Never rename or move SkeletalMesh assets.** Same issue — internal references to the skeleton break.
4. **Let Interchange name everything.** The script provides a `name` hint and Interchange uses it as a prefix. Assets end up named like `ArthurIdle` (mesh), `ArthurIdle_Skeleton`, `ArthurIdle_Anim`, etc.

### How to use ImportSkeletalMeshes.py

1. Place your `.fbx` files in `SourceAssets/Import/`.
2. Open `Scripts/ImportSkeletalMeshes.py` and edit the `IMPORTS` list. Each entry is a dict with:
   - **source**: relative path from `SourceAssets/Import/` to the `.fbx` file
   - **dest**: `/Game/...` content path — must be a **unique subdirectory per FBX**
   - **name**: name hint for Interchange (used as asset prefix)
3. Optionally edit `CLEANUP_DIRS` to delete old assets before importing.
4. Run in Unreal Editor: `py "C:/UE/T66/Scripts/ImportSkeletalMeshes.py"`
5. Run `MakeAllMaterialsUnlit.py` as a separate step.
6. Update `Content/Data/CharacterVisuals.csv` with the actual asset paths Interchange created.
7. Reload `DT_CharacterVisuals` in editor.

### What Interchange creates per FBX

Each FBX import produces (all in the same subdirectory):

| Asset | Naming Pattern | Type |
|---|---|---|
| Skeletal Mesh | `{name}` | SkeletalMesh |
| Skeleton | `{name}_Skeleton` | Skeleton |
| Animation | `{name}_Anim` | AnimSequence |
| Physics Asset | `{name}_PhysicsAsset` | PhysicsAsset |
| Textures | `Image_0`, `Image_0_003`, etc. | Texture2D |
| Materials | `Material_0`, `Material_0_003`, etc. | MaterialInstanceConstant |

Materials are parented to `FBXLegacyPhongSurfaceMaterial` (Lit Phong). They appear black in the game until `MakeAllMaterialsUnlit.py` converts them to Unlit.

### Multi-FBX character pattern

When a character has mesh + animations in separate FBX files:

1. **Primary FBX** (mesh + skeleton + textures + one animation): import to its own subdirectory
2. **Additional FBX(s)** (same rig + another animation): import each to its own subdirectory

Each FBX creates its own skeleton, but since they share the same rig, UE5 handles cross-skeleton animation playback.

**Example (Arthur / Hero_1):**
```
ImportSkeletalMeshes.py IMPORTS = [
    { source: "ArthurIdle.fbx",  dest: ".../TypeA/Idle",  name: "ArthurIdle" },
    { source: "ArthurWalk.fbx",  dest: ".../TypeA/Walk",  name: "ArthurWalk" },
]
```

Produces:
```
Content/Characters/Heroes/Hero_1/TypeA/
├── Idle/
│   ├── ArthurIdle.uasset          (SkeletalMesh)
│   ├── ArthurIdle_Skeleton.uasset (Skeleton)
│   ├── ArthurIdle_Anim.uasset     (Idle AnimSequence)
│   ├── ArthurIdle_PhysicsAsset.uasset
│   ├── Image_0.uasset, Image_0_003.uasset, Image_0_004.uasset
│   └── Material_0.uasset, Material_0_003.uasset, Material_0_004.uasset
└── Walk/
    ├── ArthurWalk.uasset           (duplicate mesh — can ignore)
    ├── ArthurWalk_Skeleton.uasset  (Skeleton)
    ├── ArthurWalk_Anim.uasset      (Walk AnimSequence)
    └── ... (duplicate textures/materials)
```

### CharacterVisuals.csv

After importing, update `Content/Data/CharacterVisuals.csv` to point at the actual Interchange-created paths. Column mapping:

| CSV Column | Maps To | Hero_1 Example |
|---|---|---|
| SkeletalMesh | The mesh asset | `.../Idle/ArthurIdle.ArthurIdle` |
| LoopingAnimation | Walk animation (CachedWalkAnim) | `.../Walk/ArthurWalk_Anim.ArthurWalk_Anim` |
| AlertAnimation | Idle animation (CachedIdleAnim) | `.../Idle/ArthurIdle_Anim.ArthurIdle_Anim` |
| RunAnimation | Jump/run animation (optional) | (empty for Hero_1) |

---

## Rules

### What the import scripts DO:
- Import GLB/FBX files as static or skeletal meshes
- Import embedded textures — UE5 creates texture assets automatically
- Import materials — UE5 creates MaterialInstanceConstants with whatever parent it picks
- For static meshes: flatten the Interchange folder structure
- Save everything

### What the import scripts do NOT do:
- Reparent any materials
- Change shading models
- Set any material parameters
- Reference any custom master material
- Rename or move skeletal mesh / animation / skeleton assets

### Material handling
- Imported materials are left exactly as UE5 creates them
- Static mesh materials: typically parented to Interchange's default
- Skeletal mesh materials: parented to `FBXLegacyPhongSurfaceMaterial` (Lit Phong, appears black until converted)
- After import, `MakeAllMaterialsUnlit.py` handles converting everything to Unlit
- If a model has no default material, the import script logs a warning

### Grounding
- Props and showcase models are grounded via downward line trace (`ECC_WorldStatic`) + mesh bounds offset
- When spawning `AStaticMeshActor` at runtime, always call `SetMobility(Movable)` BEFORE `SetStaticMesh()`

---

## Workflow Summary

### Static meshes (GLB)
```
1. Place GLBs in SourceAssets/Import/<Category>/
2. Edit IMPORTS list in Scripts/ImportStaticMeshes.py
3. Run ImportStaticMeshes.py in Unreal Editor
4. Run MakeAllMaterialsUnlit.py
5. If props changed, update Content/Data/Props.csv and run SetupPropsDataTable.py
6. Verify in-editor
```

### Skeletal meshes (FBX)
```
1. Place FBXs in SourceAssets/Import/
2. Edit IMPORTS list in Scripts/ImportSkeletalMeshes.py (one subdirectory per FBX!)
3. Run ImportSkeletalMeshes.py in Unreal Editor
4. Run MakeAllMaterialsUnlit.py
5. Update Content/Data/CharacterVisuals.csv with actual Interchange-created asset paths
6. Reload DT_CharacterVisuals in editor
7. Verify in-editor
```
