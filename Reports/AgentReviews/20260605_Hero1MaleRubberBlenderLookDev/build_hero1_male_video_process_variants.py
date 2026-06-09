from pathlib import Path
import importlib.util
import json

import bpy

BUILD_SCRIPT = Path(
    r"C:\UE\T66\Reports\AgentReviews\20260605_Hero1MaleRubberBlenderLookDev\build_hero1_male_rubber_lookdev.py"
)
LOOKDEV_ROOT = Path(
    r"C:\UE\T66\Model Generation\Runs\Pixal3D\FriendSlopProbe_Hero1Male_20260604_1415\Blender\LookDev\Hero_1_Chad_Male_Rubber_20260605"
)
BLEND_PATH = LOOKDEV_ROOT / "Hero_1_Chad_Male_RubberLookDev.blend"
RENDER_ROOT = LOOKDEV_ROOT / "Renders"
SCENE_NAME = "Transcript_Process_Comparison"
GRID_PATH = RENDER_ROOT / "Hero_1_Chad_Male_video_process_comparison.png"
PREVIEW_PATH = RENDER_ROOT / "Hero_1_Chad_Male_video_process_comparison_preview_1300.png"
RECIPE_MD = LOOKDEV_ROOT / "video_process_recipe.md"
RECIPE_JSON = LOOKDEV_ROOT / "video_process_recipe.json"

spec = importlib.util.spec_from_file_location("rubber_lookdev_build", BUILD_SCRIPT)
build = importlib.util.module_from_spec(spec)
spec.loader.exec_module(build)


VIDEO_VARIANTS = [
    {
        "id": "P01_fall_guys_rough_diffuse",
        "label": "Fall Guys Rough Diffuse",
        "video": "Make your own 3D Fall Guy in Blender 2.9",
        "mode": "principled",
        "principle": "Bright fun color, high roughness, diffuse soft character read.",
        "saturation": 1.35,
        "value": 1.16,
        "contrast": -0.18,
        "roughness": 0.72,
        "specular": 0.24,
        "coat": 0.0,
        "coat_roughness": 0.65,
        "subsurface": 0.0,
        "noise_scale": 140.0,
        "noise_detail": 5.0,
        "noise_roughness": 0.45,
        "bump_strength": 0.006,
        "bump_distance": 0.004,
        "roughness_noise": 0.0,
        "bevel_width": 0.040,
        "bevel_segments": 4,
    },
    {
        "id": "P02_kirby_toon_black_outline",
        "label": "Kirby Toon Black Outline",
        "video": "Achieving Kirby's Stylized Material in Blender [Shader Tutorial]",
        "mode": "kirby_toon_outline",
        "principle": "Flat/toon color behavior with a black Fresnel outline mixed into the material.",
        "saturation": 1.20,
        "value": 1.12,
        "contrast": -0.30,
        "roughness": 0.86,
        "specular": 0.08,
        "coat": 0.0,
        "coat_roughness": 0.9,
        "subsurface": 0.0,
        "outline_threshold": 0.14,
        "outline_softness": 0.015,
        "outline_color": [0.0, 0.0, 0.0, 1.0],
        "bevel_width": 0.035,
        "bevel_segments": 3,
    },
    {
        "id": "P03_cycles_rubber_node_group",
        "label": "Cycles Rubber Node Group",
        "video": "Blender Cycles Rubber Shader and Node Group Tutorial",
        "mode": "cycles_rubber_group",
        "principle": "Diffuse plus translucent softness, then only a small rough glossy component.",
        "saturation": 1.08,
        "value": 1.02,
        "contrast": -0.24,
        "translucent_factor": 0.55,
        "gloss_factor": 0.08,
        "gloss_roughness": 0.40,
        "bevel_width": 0.030,
        "bevel_segments": 3,
    },
    {
        "id": "P04_principled_fine_bump_rubber",
        "label": "Principled Rubber Fine Bump",
        "video": "Blender Rubber Material Shader using Principled BSDF Texture Node",
        "mode": "principled",
        "principle": "Principled BSDF rubber defined by roughness, high-scale noise, and very low bump distance.",
        "saturation": 1.10,
        "value": 1.03,
        "contrast": -0.30,
        "roughness": 0.50,
        "specular": 0.32,
        "coat": 0.0,
        "coat_roughness": 0.80,
        "subsurface": 0.0,
        "noise_scale": 500.0,
        "noise_detail": 8.0,
        "noise_roughness": 0.55,
        "bump_strength": 0.045,
        "bump_distance": 0.010,
        "roughness_noise": 0.0,
        "bevel_width": 0.030,
        "bevel_segments": 3,
    },
    {
        "id": "P05_octane_vinyl_roughness_grunge",
        "label": "Octane Vinyl Roughness Grunge",
        "video": "Cinema 4D Tutorial - Creating Plastic Vinyl Toy Textures with Octane Render",
        "mode": "principled",
        "principle": "Adapted Octane glossy vinyl: roughness channel variation plus subtle grunge/bump imperfections.",
        "adapted": True,
        "saturation": 1.14,
        "value": 1.04,
        "contrast": -0.24,
        "roughness": 0.34,
        "specular": 0.44,
        "coat": 0.06,
        "coat_roughness": 0.42,
        "subsurface": 0.0,
        "noise_scale": 75.0,
        "noise_detail": 10.0,
        "noise_roughness": 0.62,
        "bump_strength": 0.025,
        "bump_distance": 0.008,
        "roughness_noise": 0.18,
        "bevel_width": 0.034,
        "bevel_segments": 3,
    },
    {
        "id": "P06_procedural_plastic_noise_bump",
        "label": "Procedural Plastic Noise Bump",
        "video": "Creating Procedural Plastic Materials in Blender (Tutorial)",
        "mode": "principled",
        "principle": "Procedural plastic controls: noise texture, bump, material roughness, and clearcoat kept controllable.",
        "saturation": 1.05,
        "value": 1.02,
        "contrast": -0.18,
        "roughness": 0.38,
        "specular": 0.36,
        "coat": 0.0,
        "coat_roughness": 0.55,
        "subsurface": 0.0,
        "noise_scale": 100.0,
        "noise_detail": 10.0,
        "noise_roughness": 0.30,
        "bump_strength": 0.030,
        "bump_distance": 0.020,
        "roughness_noise": 0.08,
        "bevel_width": 0.024,
        "bevel_segments": 3,
    },
]


def remove_prefixed_collections(prefix):
    for coll in list(bpy.data.collections):
        if coll.name.startswith(prefix):
            for obj in list(coll.objects):
                bpy.data.objects.remove(obj, do_unlink=True)
            bpy.data.collections.remove(coll)


def source_color(src_mat):
    return build.source_base_color(src_mat)


def set_input(node, names, value):
    return build.set_input(node, names, value)


def image_color_chain(nodes, links, src_mat, variant, x=-760, y=80):
    image = build.get_first_image(src_mat)
    fallback = build.adjusted_color(source_color(src_mat), variant.get("saturation", 1.0), variant.get("value", 1.0))
    if image is None:
        rgb = nodes.new("ShaderNodeRGB")
        rgb.location = (x, y)
        rgb.outputs["Color"].default_value = fallback
        return rgb.outputs["Color"], fallback

    tex = nodes.new("ShaderNodeTexImage")
    tex.location = (x, y)
    tex.image = image
    hsv = nodes.new("ShaderNodeHueSaturation")
    hsv.location = (x + 230, y)
    hsv.inputs["Saturation"].default_value = variant.get("saturation", 1.0)
    hsv.inputs["Value"].default_value = variant.get("value", 1.0)
    bc = nodes.new("ShaderNodeBrightContrast")
    bc.location = (x + 470, y)
    bc.inputs["Bright"].default_value = 0.0
    bc.inputs["Contrast"].default_value = variant.get("contrast", 0.0)
    links.new(tex.outputs["Color"], hsv.inputs["Color"])
    links.new(hsv.outputs["Color"], bc.inputs["Color"])
    return bc.outputs["Color"], fallback


def add_noise_bump(nodes, links, bsdf, variant, x=-520, y=-260):
    if variant.get("bump_strength", 0.0) <= 0.0:
        return
    noise = nodes.new("ShaderNodeTexNoise")
    noise.location = (x, y)
    noise.inputs["Scale"].default_value = variant.get("noise_scale", 120.0)
    noise.inputs["Detail"].default_value = variant.get("noise_detail", 5.0)
    noise.inputs["Roughness"].default_value = variant.get("noise_roughness", 0.5)
    bump = nodes.new("ShaderNodeBump")
    bump.location = (x + 260, y)
    bump.inputs["Strength"].default_value = variant["bump_strength"]
    bump.inputs["Distance"].default_value = variant["bump_distance"]
    links.new(noise.outputs["Fac"], bump.inputs["Height"])
    if "Normal" in bsdf.inputs:
        links.new(bump.outputs["Normal"], bsdf.inputs["Normal"])
    if variant.get("roughness_noise", 0.0) > 0.0 and "Roughness" in bsdf.inputs:
        ramp = nodes.new("ShaderNodeValToRGB")
        ramp.location = (x + 260, y - 170)
        base = variant["roughness"]
        amount = variant["roughness_noise"]
        ramp.color_ramp.elements[0].position = 0.12
        ramp.color_ramp.elements[0].color = (max(0.0, base - amount),) * 3 + (1.0,)
        ramp.color_ramp.elements[1].position = 1.0
        ramp.color_ramp.elements[1].color = (min(1.0, base + amount),) * 3 + (1.0,)
        links.new(noise.outputs["Fac"], ramp.inputs["Factor"])
        links.new(ramp.outputs["Color"], bsdf.inputs["Roughness"])


def make_principled_video_material(src_mat, variant, slot_index):
    mat = bpy.data.materials.new(f"{variant['id']}_{slot_index}_{src_mat.name if src_mat else 'mat'}")
    mat.use_nodes = True
    mat.diffuse_color = build.adjusted_color(source_color(src_mat), variant.get("saturation", 1.0), variant.get("value", 1.0))
    nodes = mat.node_tree.nodes
    links = mat.node_tree.links
    nodes.clear()
    output = nodes.new("ShaderNodeOutputMaterial")
    output.location = (520, 0)
    bsdf = nodes.new("ShaderNodeBsdfPrincipled")
    bsdf.location = (250, 0)
    links.new(bsdf.outputs["BSDF"], output.inputs["Surface"])

    color_socket, fallback = image_color_chain(nodes, links, src_mat, variant)
    links.new(color_socket, bsdf.inputs["Base Color"])
    set_input(bsdf, ["Metallic"], 0.0)
    set_input(bsdf, ["Roughness"], variant["roughness"])
    set_input(bsdf, ["Specular IOR Level", "Specular"], variant["specular"])
    set_input(bsdf, ["Coat Weight", "Coat"], variant["coat"])
    set_input(bsdf, ["Coat Roughness"], variant["coat_roughness"])
    set_input(bsdf, ["Subsurface Weight", "Subsurface"], variant["subsurface"])
    set_input(bsdf, ["Subsurface Scale"], 0.05)
    set_input(bsdf, ["Subsurface Color"], fallback)
    add_noise_bump(nodes, links, bsdf, variant)

    mat["video_process_recipe"] = json.dumps(variant, sort_keys=True)
    return mat


def make_cycles_rubber_group_material(src_mat, variant, slot_index):
    mat = bpy.data.materials.new(f"{variant['id']}_{slot_index}_{src_mat.name if src_mat else 'mat'}")
    mat.use_nodes = True
    mat.diffuse_color = build.adjusted_color(source_color(src_mat), variant["saturation"], variant["value"])
    nodes = mat.node_tree.nodes
    links = mat.node_tree.links
    nodes.clear()
    output = nodes.new("ShaderNodeOutputMaterial")
    output.location = (700, 0)

    color_socket, fallback = image_color_chain(nodes, links, src_mat, variant, x=-860, y=160)
    diffuse = nodes.new("ShaderNodeBsdfDiffuse")
    diffuse.location = (-90, 120)
    translucent = nodes.new("ShaderNodeBsdfTranslucent")
    translucent.location = (-90, -70)
    glossy = nodes.new("ShaderNodeBsdfGlossy")
    glossy.location = (170, -230)
    mix_dt = nodes.new("ShaderNodeMixShader")
    mix_dt.location = (170, 50)
    mix_gloss = nodes.new("ShaderNodeMixShader")
    mix_gloss.location = (450, 0)

    links.new(color_socket, diffuse.inputs["Color"])
    set_input(diffuse, ["Roughness"], 0.72)
    translucent.inputs["Color"].default_value = (fallback[0] * 0.45, fallback[1] * 0.45, fallback[2] * 0.45, 1.0)
    glossy.inputs["Color"].default_value = (0.82, 0.82, 0.82, 1.0)
    glossy.inputs["Roughness"].default_value = variant["gloss_roughness"]
    mix_dt.inputs["Fac"].default_value = variant["translucent_factor"]
    mix_gloss.inputs["Fac"].default_value = variant["gloss_factor"]
    links.new(diffuse.outputs["BSDF"], mix_dt.inputs[1])
    links.new(translucent.outputs["BSDF"], mix_dt.inputs[2])
    links.new(mix_dt.outputs["Shader"], mix_gloss.inputs[1])
    links.new(glossy.outputs["BSDF"], mix_gloss.inputs[2])
    links.new(mix_gloss.outputs["Shader"], output.inputs["Surface"])

    mat["video_process_recipe"] = json.dumps(variant, sort_keys=True)
    return mat


def make_kirby_toon_outline_material(src_mat, variant, slot_index):
    mat = bpy.data.materials.new(f"{variant['id']}_{slot_index}_{src_mat.name if src_mat else 'mat'}")
    mat.use_nodes = True
    mat.diffuse_color = build.adjusted_color(source_color(src_mat), variant["saturation"], variant["value"])
    nodes = mat.node_tree.nodes
    links = mat.node_tree.links
    nodes.clear()

    output = nodes.new("ShaderNodeOutputMaterial")
    output.location = (900, 0)
    color_socket, fallback = image_color_chain(nodes, links, src_mat, variant, x=-1060, y=120)

    toon = nodes.new("ShaderNodeBsdfToon")
    toon.location = (-300, -120)
    toon.inputs["Size"].default_value = 0.55
    toon.inputs["Smooth"].default_value = 0.05
    links.new(color_socket, toon.inputs["Color"])

    shader_to_rgb = nodes.new("ShaderNodeShaderToRGB")
    shader_to_rgb.location = (-80, -120)
    shadow_mix = nodes.new("ShaderNodeMixRGB")
    shadow_mix.location = (150, 80)
    shadow_mix.blend_type = "MULTIPLY"
    shadow_mix.inputs["Factor"].default_value = 0.18
    links.new(toon.outputs["BSDF"], shader_to_rgb.inputs["Shader"])
    links.new(color_socket, shadow_mix.inputs["Color1"])
    links.new(shader_to_rgb.outputs["Color"], shadow_mix.inputs["Color2"])

    layer = nodes.new("ShaderNodeLayerWeight")
    layer.location = (120, -240)
    layer.inputs["Blend"].default_value = 0.33
    outline_ramp = nodes.new("ShaderNodeValToRGB")
    outline_ramp.location = (340, -240)
    outline_ramp.color_ramp.interpolation = "CONSTANT"
    outline_ramp.color_ramp.elements[0].position = variant["outline_threshold"]
    outline_ramp.color_ramp.elements[0].color = (0.0, 0.0, 0.0, 1.0)
    outline_ramp.color_ramp.elements[1].position = min(1.0, variant["outline_threshold"] + variant["outline_softness"])
    outline_ramp.color_ramp.elements[1].color = (1.0, 1.0, 1.0, 1.0)

    outline_mix = nodes.new("ShaderNodeMixRGB")
    outline_mix.location = (590, 80)
    outline_mix.inputs["Color2"].default_value = tuple(variant["outline_color"])
    links.new(layer.outputs["Fresnel"], outline_ramp.inputs["Factor"])
    links.new(outline_ramp.outputs["Color"], outline_mix.inputs["Factor"])
    links.new(shadow_mix.outputs["Color"], outline_mix.inputs["Color1"])

    emission = nodes.new("ShaderNodeEmission")
    emission.location = (760, 60)
    emission.inputs["Strength"].default_value = 1.0
    links.new(outline_mix.outputs["Color"], emission.inputs["Color"])
    links.new(emission.outputs["Emission"], output.inputs["Surface"])

    mat["video_process_recipe"] = json.dumps(variant, sort_keys=True)
    return mat


def make_video_material(src_mat, variant, slot_index):
    if variant["mode"] == "cycles_rubber_group":
        return make_cycles_rubber_group_material(src_mat, variant, slot_index)
    if variant["mode"] == "kirby_toon_outline":
        return make_kirby_toon_outline_material(src_mat, variant, slot_index)
    return make_principled_video_material(src_mat, variant, slot_index)


def add_video_softening(objects, variant):
    for obj in objects:
        if obj.type != "MESH":
            continue
        try:
            bpy.context.view_layer.objects.active = obj
            obj.select_set(True)
            bpy.ops.object.shade_smooth()
            obj.select_set(False)
        except Exception:
            pass
        bevel = obj.modifiers.new("Transcript_Process_Soft_Edge", "BEVEL")
        bevel.width = variant.get("bevel_width", 0.02)
        bevel.segments = variant.get("bevel_segments", 2)
        try:
            bevel.profile = 0.58
        except Exception:
            pass
        normal = obj.modifiers.new("Transcript_Process_Weighted_Normals", "WEIGHTED_NORMAL")
        try:
            normal.keep_sharp = True
        except Exception:
            pass


def duplicate_video_variant(template_objects, variant, location, target_height=2.22):
    root, coll, objs = build.duplicate_group(
        template_objects,
        "Transcript_" + variant["id"],
        location,
        material_builder=make_video_material,
        variation=variant,
        target_height=target_height,
    )
    add_video_softening(objs, variant)
    return root, coll, objs


def build_scene(template_objects):
    old_scene = bpy.data.scenes.get(SCENE_NAME)
    if old_scene:
        bpy.data.scenes.remove(old_scene, do_unlink=True)
    remove_prefixed_collections("Transcript_")

    scene = bpy.data.scenes.new(SCENE_NAME)
    try:
        bpy.context.window.scene = scene
    except Exception:
        pass

    with bpy.context.temp_override(scene=scene, view_layer=scene.view_layers[0]):
        build.set_render_defaults(scene, (2600, 1500), 1)
        build.add_flat_lighting(scene, center=(0, 0, 2.95))
        build.make_backdrop("Transcript_Process_Backdrop", 12.4, 6.5, 2.95)
        xs = [-4.5, -1.5, 1.5, 4.5]
        top_z = 3.05
        bottom_z = 0.0
        model_height = 2.22
        build.make_image_panel(build.SOURCE_IMAGE, "Reference image", xs[0], z=top_z, height=model_height)
        build.duplicate_group(
            template_objects,
            "Transcript_Raw_Pixal3D_GLB",
            (xs[1], 0, top_z),
            raw=True,
            target_height=model_height,
        )
        build.make_text("Raw Pixal3D GLB", (xs[1], -0.05, top_z + model_height + 0.22), size=0.115)

        positions = [
            (xs[2], top_z),
            (xs[3], top_z),
            (xs[0], bottom_z),
            (xs[1], bottom_z),
            (xs[2], bottom_z),
            (xs[3], bottom_z),
        ]
        for variant, (x, z) in zip(VIDEO_VARIANTS, positions):
            duplicate_video_variant(template_objects, variant, (x, 0, z), target_height=model_height)
            build.make_text(variant["label"], (x, -0.05, z + model_height + 0.22), size=0.105)
        build.add_camera(scene, "Camera_Transcript_Process_Ortho", (0, -15.5, 3.0), (0, 0, 2.9), 12.2)
    return scene


def render_scene_to(scene, path, resolution):
    scene.render.resolution_x = resolution[0]
    scene.render.resolution_y = resolution[1]
    scene.render.resolution_percentage = 100
    scene.frame_set(1)
    scene.render.image_settings.file_format = "PNG"
    scene.render.filepath = str(path)
    bpy.ops.render.render(write_still=True, scene=scene.name)


def write_recipe():
    data = {
        "scene": SCENE_NAME,
        "blend": str(BLEND_PATH),
        "source_glb": str(build.SOURCE_GLB),
        "source_image": str(build.SOURCE_IMAGE),
        "comparison_grid": str(GRID_PATH),
        "preview_grid": str(PREVIEW_PATH),
        "variants": VIDEO_VARIANTS,
    }
    RECIPE_JSON.write_text(json.dumps(data, indent=2), encoding="utf-8")
    lines = [
        "# Hero 1 Male Video Process Material Comparison",
        "",
        f"- Scene: `{SCENE_NAME}`",
        f"- Blend: `{BLEND_PATH}`",
        f"- Source GLB: `{build.SOURCE_GLB}`",
        f"- Comparison grid: `{GRID_PATH}`",
        f"- Preview grid: `{PREVIEW_PATH}`",
        "",
        "Each version follows one transcript/video process. They are deliberately not collapsed into one combined shader.",
        "",
        "| ID | Label | Video | Process Principle | Key Parameters |",
        "|---|---|---|---|---|",
    ]
    for v in VIDEO_VARIANTS:
        if v["mode"] == "cycles_rubber_group":
            params = f"translucent factor {v['translucent_factor']}; gloss factor {v['gloss_factor']}; gloss roughness {v['gloss_roughness']}"
        elif v["mode"] == "kirby_toon_outline":
            params = f"black Fresnel outline threshold {v['outline_threshold']}; roughness {v['roughness']}; no coat"
        else:
            params = (
                f"roughness {v['roughness']}; specular {v['specular']}; coat {v['coat']}; "
                f"noise scale {v['noise_scale']}; bump distance {v['bump_distance']}"
            )
        lines.append(f"| {v['id']} | {v['label']} | {v['video']} | {v['principle']} | {params} |")
    lines.extend(
        [
            "",
            "## Notes",
            "",
            "- `P01_fall_guys_rough_diffuse` is the closest to the Fall Guys material transcript: bright, diffuse, high roughness, no coat.",
            "- `P02_kirby_toon_black_outline` is the only version that intentionally follows the Kirby NPR material and includes the requested black outline.",
            "- `P05_octane_vinyl_roughness_grunge` is an adapted Blender mapping of the C4D/Octane vinyl process, not a literal Octane node reproduction.",
            "- `P06_procedural_plastic_noise_bump` follows the procedural plastic/latex control idea and may be useful as a boundary, not necessarily as the final rubber target.",
        ]
    )
    RECIPE_MD.write_text("\n".join(lines) + "\n", encoding="utf-8")


def main():
    RENDER_ROOT.mkdir(parents=True, exist_ok=True)
    template_collection = bpy.data.collections.get("Source_Template_Hidden")
    if not template_collection:
        template_objects = build.import_glb_template()
        build.normalize_group(template_objects)
        template_collection = bpy.data.collections.new("Source_Template_Hidden")
        bpy.context.scene.collection.children.link(template_collection)
        for obj in template_objects:
            for coll in list(obj.users_collection):
                coll.objects.unlink(obj)
            template_collection.objects.link(obj)
            obj.hide_viewport = True
            obj.hide_render = True
    else:
        template_objects = [obj for obj in template_collection.objects if obj.type == "MESH"]

    if not template_objects:
        raise RuntimeError("No template mesh objects found for video process comparison.")

    scene = build_scene(template_objects)
    render_scene_to(scene, GRID_PATH, (2600, 1500))
    render_scene_to(scene, PREVIEW_PATH, (1300, 750))
    write_recipe()
    bpy.ops.wm.save_as_mainfile(filepath=str(BLEND_PATH))
    print(
        "T66_VIDEO_PROCESS_COMPARISON="
        + json.dumps(
            {
                "scene": SCENE_NAME,
                "blend": str(BLEND_PATH),
                "grid": str(GRID_PATH),
                "preview": str(PREVIEW_PATH),
                "recipe_md": str(RECIPE_MD),
                "recipe_json": str(RECIPE_JSON),
                "variants": [v["id"] for v in VIDEO_VARIANTS],
            },
            sort_keys=True,
        )
    )


if __name__ == "__main__":
    main()
