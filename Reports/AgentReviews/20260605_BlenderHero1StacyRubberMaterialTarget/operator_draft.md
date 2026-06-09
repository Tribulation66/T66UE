# Operator Draft

I located the likely source candidates but am stopping at the required source-confirmation gate.

Important correction: the recent FriendSlop female Hero 1 artifact appears to be a 2D reference PNG, not a 3D mesh:

`C:\UE\T66\FriendSlop\ImageGen\Heroes\Demo\Hero_1_Chad_Female\FriendSlop_Hero1_Chad_Female_ReferenceRepro_v01.png`

The best existing female Hero 1 3D source I found is:

`C:\UE\T66\Model Generation\Runs\Pixal3D\HeroDemoLineup_20260522_AccuRig\Outputs\Hero_1_Stacy.glb`

Reason: it is the raw generated GLB for `Hero_1_Stacy`, exists on disk, and the live `CharacterVisuals.csv` row shows `Hero_1_Stacy` is the active female Hero 1 row, while `Hero_1_Stacy_DemoSkin` maps to the beachgoer/demo variant. The raw GLB is also the best existing 3D baseline for the requested "raw Pixel3D output vs Fall Guys rubber version" Blender comparison. Caveat: it is Pixal3D/ToonStyle, not a confirmed FriendSlop female 3D mesh.

I have not touched Unreal, opened Blender, created materials, rendered images, or changed source assets. The decision gate is recorded in `Reports/AgentReviews/20260605_BlenderHero1StacyRubberMaterialTarget/decision_block.md`.

Question for Pablo: three real options: (a) use the existing Pixal3D/ToonStyle `C:\UE\T66\Model Generation\Runs\Pixal3D\HeroDemoLineup_20260522_AccuRig\Outputs\Hero_1_Stacy.glb` as the Blender baseline now, noting it is not the newer FriendSlop female style; (b) treat `C:\UE\T66\FriendSlop\ImageGen\Heroes\Demo\Hero_1_Chad_Female\FriendSlop_Hero1_Chad_Female_ReferenceRepro_v01.png` as the required source direction, which means generating a female FriendSlop 3D mesh first and blocks this Blender material mission; or (c) use a different exact path you specify. Which source should I use?
