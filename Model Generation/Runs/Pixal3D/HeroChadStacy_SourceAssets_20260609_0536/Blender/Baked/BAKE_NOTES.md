# Baked-lit → unlit comparison bake (2026-06-09)

Baked each of the 5 review subjects' **current LIT look** into a static texture and displayed it on an
**emission/unlit** shader, stacked directly **above** its lit original for in-viewport comparison.

## Method (faithful, no new lights)
- **Lighting used = the existing lit setup only:** `ReviewSun` (SUN, energy 2.1, rot 42°/0°/28°) + `ReviewFill`
  (AREA, energy 3200, rot 60°/0°/12°). The 4 `T66_*` softbox lights were already `hide_render=True` and did
  not contribute. For each bake, all other meshes were `hide_render` so the emissive unlit copies couldn't
  contaminate the result. No lights were added or moved.
- Per subject: duplicated the lit original (originals + their materials untouched, single-user copies),
  **Cycles `COMBINED` bake**, GPU/OPTIX, 64 adaptive samples + denoise, 8px margin, into a NEW 4096×4096 PNG
  (matching each subject's 4K base-color texture). UVs were verified first (single `UVMap`, clean [0,1] bounds,
  one material/one texture each — no overlap; the existing 4K base-color already maps correctly).
- Baked texture assigned as **Emission** on a fresh `{Subject}_BakedUnlit_Mat` (Emission → Material Output =
  flat/unlit, exactly how Unreal-unlit shows it). Each `{Subject}_BakedUnlit` object placed at the lit
  original's world XY + rotation, offset straight up (+Z by the subject's height + 0.5).

## Saved textures (this folder)
- `CurrentHero1Male_BakedLit_BaseColor.png` (4096², 7.5 MB)
- `Hero1Stacy_BakedLit_BaseColor.png` (4096², 9.0 MB)
- `Hero2Chad_BakedLit_BaseColor.png` (4096², 7.7 MB)
- `IdolProjectile_BakedLit_BaseColor.png` (4096², 13.5 MB)
- `WeaponProjectile_BakedLit_BaseColor.png` (4096², 6.7 MB)

New scene: `../HeroChadStacy_with_current_hero1_male_Baked.blend` (original `.blend` on disk left untouched).

## Translation note (general)
View-independent shading — diffuse form + ambient occlusion — bakes faithfully and looks the same from any
angle. Any **glossy/specular highlight is view-dependent**: it gets frozen into the texture at the bake
angle (lit by ReviewSun ~42°/28° + ReviewFill ~60°/12°). On the unlit copy that frozen highlight stays
painted in place, so when the model rotates the highlight does **not** move with the light — it drifts to
the "wrong" side and reads as incorrect. Diffuse and AO do not have this problem.

## Per-subject translation quality
- **CurrentHero1Male** (coat/vest/skin): mostly diffuse → translates very faithfully. Only faint fabric/skin
  sheen is frozen; drift on rotation is minor.
- **Hero1Stacy** (skin-dominant): diffuse form faithful. Soft skin specular on shoulders/chest/thighs facing
  the key is frozen → moderate drift if rotated.
- **Hero2Chad** (skin-dominant): same as above — faithful diffuse; frozen skin sheen drifts moderately on
  rotation.
- **IdolProjectile** (fire): flat/emissive-leaning base color translates well as a static texture. Any
  view-dependent transparency/refraction in the original is not captured; highlight drift is low.
- **WeaponProjectile** (dark glossy horn): **least faithful** — a glossy dark surface's specular highlight is
  strongly view-dependent and is frozen at the bake angle, so on rotation the "shine" sits in the wrong place
  and reads as a painted-on streak. Diffuse silhouette/AO are fine; the gloss is the caveat.

## Verification
EEVEE rendered-viewport screenshots confirmed each baked-unlit copy matches its lit original in color and
brightness (no colorspace error) before and after the full set.
