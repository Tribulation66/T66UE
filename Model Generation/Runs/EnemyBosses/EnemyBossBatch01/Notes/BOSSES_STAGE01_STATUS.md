# Bosses Stage 01 Status

Status: Stage 1 artifacts generated for all 23 live boss rows.

Generated counts:
- Source images: 23/23
- Raw Trellis GLBs: 23/23
- Front QA renders: 23/23

No Quad Remesher, Quad Retro/Blender retro pass, Unreal import, or staged cook was run.

Questionable rows:
- Hell_Horseman_Death: QuestionableReview - Questionable: local derived stand-in source based on Famine; QA render is visible but derivative. Reroll source image before art approval if time allows. source_image_method=local_derivative_fallback
- Hell_FalseProphet: QuestionableReview - Questionable: local derived stand-in source; QA render is visible but blocky around banners/halo. Reroll source image before art approval. source_image_method=local_derivative_fallback
- Hell_Antichrist: QuestionableReview - Questionable: local derived stand-in source based on Bael; QA render is visible but duplicate silhouette. Reroll source image before art approval. source_image_method=local_derivative_fallback
- Hell_GreatDragon: NeedsSourceReroll - Questionable/fail-art: local vector fallback source produced a visible but overly abstract raw model. Must reroll source image and Trellis before approval. source_image_method=local_vector_fallback

Key files:
- Manifest: Reports/Stage01_Bosses_TrellisManifest.json
- Source contact sheet: QA/TrellisFront/Bosses_source_contact_23.png
- Trellis front QA contact sheet: QA/TrellisFront/Bosses/Bosses_TrellisFront_contact_23.png
- RunPod log: Reports/Stage01_Bosses_TrellisRunPod.log
