Draft source list:

YouTube candidates:
1. Blender Rubber Material Shader using Principled BSDF Texture Node — blenderian — https://www.youtube.com/watch?v=8LqN2Yy55AY
2. Blender Cycles Rubber Shader and Node Group Tutorial — Jayanam — https://www.youtube.com/watch?v=vngGTaBCZ-o
3. Basic Glossy Shader in Blender 2.8 - Materials Tutorial (Eevee) — chocofur — https://www.youtube.com/watch?v=VrsteZ3Ci3w
4. Achieving Kirby's Stylized Material in Blender [Shader Tutorial] — TooEazyCG — https://www.youtube.com/watch?v=ZatjXklv8No
5. How to make Blender Glossy Plastic and Matte Plastic Material using BSDF shader — blenderian — https://www.youtube.com/watch?v=zxvjZwg7JeI
6. Cinema 4D Tutorial - Creating Plastic Vinyl Toy Textures with Octane Render — eyedesyn — https://www.youtube.com/watch?v=8Njl5hLmGs0

Non-YouTube sources:
1. Blender Manual: Principled BSDF — https://docs.blender.org/manual/en/latest/render/shader_nodes/shader/principled.html
2. Blender Stack Exchange: How to Create a Vinyl Plastic Toy Shader? — https://blender.stackexchange.com/questions/116195/how-to-create-a-vinyl-plastic-toy-shader?noredirect=1
3. Blender Stack Exchange: grainy rough plastic material — https://blender.stackexchange.com/questions/142447/how-can-i-create-a-grainy-rough-plastic-material-in-blender-just-like-one-on-mi/142456#142456
4. CGian: Blender Black Rubber Material — https://cgian.com/blender-rubber-plastic-material/
5. Blender 4.0 Shading & Texturing release notes — https://developer.blender.org/docs/release_notes/4.0/shading/
6. PlayCanvas PBR physical rendering guide — https://developer.playcanvas.com/user-manual/graphics/physical-rendering/
7. Open 3D Engine PBR material documentation — https://www.docs.o3de.org/docs/atom-guide/look-dev/materials/pbr/
8. Roughness Setting reference — https://sameerbaloch.com/roughness-setting/

Preliminary failure hypothesis:
The prior pass likely varied clearcoat/specular/gloss too aggressively and used coat as the primary readability control, so it created resin/plastic topcoat highlights rather than soft rubber. The next pass should focus on higher roughness, lower or zero clearcoat, lower specular/f0, roughness/normal microvariation, soft Fresnel/sheens, flattened albedo, and geometry silhouette.

