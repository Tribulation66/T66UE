# Agent 2 - Characters

Agent 2 owns the character Pixal3D source images in this folder.

Scope:
- Included: heroes, available hero demo skins, companions, and companion demo skins.
- Excluded: interactables, boosts, gates, visual props, enemies, failed iterations, contact sheets, and the skipped loot crate.
- PNG count: 26.

Production workflow:
- Use `C:\UE\T66\Model Generation\Instructions\09_PIXAL3D_TOONSTYLE_PRODUCTION_IMPORT_INSTRUCTIONS.md`.
- For any rigged/editable character work, also read `C:\UE\T66\Model Generation\Rigging and Animation\RIGGING_ANIMATION_AGENTS.md`.
- Build a production manifest from these PNGs and run the ToonStyle Pixal3D production path.
- Do not manually assign ToonStyle materials after import.
- Heroes and companions are runtime character visuals, not map-placed static props.

## Heroes

Hero source images are full-body character references for model generation. Runtime visual rows should be owned by `Content/Data/CharacterVisuals.csv` / `DT_CharacterVisuals`; hero definitions are in `Content/Data/Heroes.csv`.

| Source image | Intended character | Hero row / visual row expectation |
| --- | --- | --- |
| `Characters/Heroes/Base/hero_1_chad_george_founding_male.png` | George / Founding Chad inspired male founding commander giga chad | `Hero_1`, Chad/male visual |
| `Characters/Heroes/Base/hero_1_stacy_george_founding_female.png` | George / Founding Chad inspired female founding commander giga chad | `Hero_1`, Stacy/female visual |
| `Characters/Heroes/Base/hero_2_chad_lubu_male.png` | Lu Bu inspired male giga chad | `Hero_2`, Chad/male visual |
| `Characters/Heroes/Base/hero_2_stacy_lubu_female.png` | Lu Bu inspired female giga chad | `Hero_2`, Stacy/female visual |
| `Characters/Heroes/Base/hero_3_chad_boxer_male.png` | Mike Tyson inspired male boxer giga chad | `Hero_3`, Chad/male visual |
| `Characters/Heroes/Base/hero_3_stacy_boxer_female.png` | Mike Tyson inspired female boxer giga chad | `Hero_3`, Stacy/female visual |
| `Characters/Heroes/Base/hero_4_chad_billy_cowboy_male.png` | Billy the Kid inspired male cowboy giga chad | `Hero_4`, Chad/male visual |
| `Characters/Heroes/Base/hero_4_stacy_billy_cowboy_female.png` | Billy the Kid inspired female cowboy giga chad | `Hero_4`, Stacy/female visual |
| `Characters/Heroes/Base/hero_5_chad_yakub_male.png` | Yakub inspired male giga chad | `Hero_5`, Chad/male visual |
| `Characters/Heroes/Base/hero_5_stacy_yakub_female.png` | Yakub inspired female giga chad | `Hero_5`, Stacy/female visual |

## Hero Demo Variants

These are purchasable demo or early-adopter skin source images. Demo source images exist only for `Hero_1`, `Hero_3`, `Hero_4`, and `Hero_5`. `Hero_2` / Lu Bu remains base-only because no finalized Lu Bu demo image exists.

| Source image | Intended character skin |
| --- | --- |
| `Characters/Heroes/Demo/hero_1_chad_george_founding_male_demo.png` | Hero 1 Chad demo skin |
| `Characters/Heroes/Demo/hero_1_stacy_george_founding_female_demo.png` | Hero 1 Stacy demo skin |
| `Characters/Heroes/Demo/hero_3_chad_boxer_male_demo.png` | Hero 3 Chad demo skin |
| `Characters/Heroes/Demo/hero_3_stacy_boxer_female_demo.png` | Hero 3 Stacy demo skin |
| `Characters/Heroes/Demo/hero_4_chad_billy_cowboy_male_demo.png` | Hero 4 Chad demo skin |
| `Characters/Heroes/Demo/hero_4_stacy_billy_cowboy_female_demo.png` | Hero 4 Stacy demo skin |
| `Characters/Heroes/Demo/hero_5_chad_yakub_male_demo.png` | Hero 5 Chad demo skin |
| `Characters/Heroes/Demo/hero_5_stacy_yakub_female_demo.png` | Hero 5 Stacy demo skin |

Skin notes:
- Runtime keeps the legacy asset-backed `Beachgoer` skin ID as the purchasable hero skin slot for compatibility, but hero selection displays it as `Demo`.
- `UT66SkinSubsystem` offers that skin only for `Hero_1`, `Hero_3`, `Hero_4`, and `Hero_5`; default skins remain available for every hero.
- `Content/Data/CharacterVisuals.csv` currently keeps asset-backed `Hero_1_Chad_Beachgoer`, `Hero_3_Chad_Beachgoer`, `Hero_4_Chad_Beachgoer`, and `Hero_5_Chad_Beachgoer` rows only. These are placeholder-backed runtime rows until the Pixal3D birthday-sash demo meshes are imported.
- After Pixal3D import, add or replace both `Chad` and `Stacy` visual rows for the four demo heroes so the male and female demo source images become real runtime visuals. Do not add demo rows for `Hero_2` unless a finalized Lu Bu demo source image is created.

## Companions

Companion source images are full-body character references for model generation. Runtime definitions should be owned by `Content/Data/Companions.csv` / `DT_Companions`; visuals should be in `Content/Data/CharacterVisuals.csv` / `DT_CharacterVisuals`.

| Source image | Intended companion |
| --- | --- |
| `Characters/Companions/Base/companion_01_light_skin_black_rap_vixen.png` | Light-skinned Black early-2000s rap vixen companion |
| `Characters/Companions/Base/companion_02_blonde_tavern_barmaid.png` | Blonde medieval tavern barmaid companion |
| `Characters/Companions/Base/companion_03_brunette_college_girl.png` | Brunette college girl companion |
| `Characters/Companions/Base/companion_04_black_haired_office_lady.png` | Black-haired strict office lady companion |

## Companion Demo Variants

| Source image | Intended companion skin |
| --- | --- |
| `Characters/Companions/Demo/companion_01_light_skin_black_rap_vixen_demo.png` | Companion 1 demo skin |
| `Characters/Companions/Demo/companion_02_blonde_tavern_barmaid_demo.png` | Companion 2 demo skin |
| `Characters/Companions/Demo/companion_03_brunette_college_girl_demo.png` | Companion 3 demo skin |
| `Characters/Companions/Demo/companion_04_black_haired_office_lady_demo.png` | Companion 4 demo skin |

## Character Placement

- Heroes are selected and loaded through the hero selection and character visual data flow, not placed directly in the map as static props.
- Companions should unlock through companion progression data and use companion spawn/visual systems, not hardcoded map placement.
- Demo variants should be treated as skins or cosmetics. Keep base character identity and unlock data separate from cosmetic source art.
