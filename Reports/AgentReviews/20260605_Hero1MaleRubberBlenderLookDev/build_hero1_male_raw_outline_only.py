import importlib.util
import json
import math
from array import array
from collections import deque
from pathlib import Path

import bpy
from mathutils import Matrix, Vector


PROJECT_ROOT = Path(r"C:\UE\T66")
RUN_ROOT = PROJECT_ROOT / "Model Generation" / "Runs" / "Pixal3D" / "FriendSlopProbe_Hero1Male_20260604_1415"
LOOKDEV_ROOT = RUN_ROOT / "Blender" / "LookDev" / "Hero_1_Chad_Male_Rubber_20260605"
RENDER_ROOT = LOOKDEV_ROOT / "Renders"
SOURCE_GLB = RUN_ROOT / "Outputs" / "Hero_1_Chad_Male.glb"
SOURCE_IMAGE = RUN_ROOT / "Sources" / "Hero_1_Chad_Male.png"
BLEND_PATH = LOOKDEV_ROOT / "Hero_1_Chad_Male_RawOutlineOnly.blend"
RENDER_PATH = RENDER_ROOT / "Hero_1_Chad_Male_raw_vs_black_outline.png"
BASE_RENDER_PATH = RENDER_ROOT / "Hero_1_Chad_Male_raw_vs_raw_base.png"
MASK_PATH = RENDER_ROOT / "Hero_1_Chad_Male_outline_mask.png"
OUTLINE_IMAGE_PATH = RENDER_ROOT / "Hero_1_Chad_Male_outer_only_outline_overlay.png"
RECIPE_JSON = LOOKDEV_ROOT / "raw_black_outline_recipe.json"
RECIPE_MD = LOOKDEV_ROOT / "raw_black_outline_recipe.md"
HELPER_PATH = PROJECT_ROOT / "Reports" / "AgentReviews" / "20260605_Hero1MaleRubberBlenderLookDev" / "build_hero1_male_rubber_lookdev.py"


def load_helper():
    spec = importlib.util.spec_from_file_location("rubber_lookdev_helper", HELPER_PATH)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def assign_collection_recursive(obj, collection):
    obj["raw_outline_target_collection"] = collection.name
    for child in obj.children:
        assign_collection_recursive(child, collection)


def make_black_outline_material():
    mat = bpy.data.materials.new("M_Black_Outer_Silhouette_Only")
    mat.diffuse_color = (0.0, 0.0, 0.0, 1.0)
    mat.use_nodes = True
    nodes = mat.node_tree.nodes
    nodes.clear()
    output = nodes.new("ShaderNodeOutputMaterial")
    bsdf = nodes.new("ShaderNodeBsdfPrincipled")
    bsdf.inputs["Base Color"].default_value = (0.0, 0.0, 0.0, 1.0)
    bsdf.inputs["Roughness"].default_value = 0.8
    mat.node_tree.links.new(bsdf.outputs["BSDF"], output.inputs["Surface"])
    return mat


def add_outer_silhouette_shell(source_objects, collection, scale_factor=1.018, camera_back_offset=0.035):
    black_mat = make_black_outline_material()
    shell_objects = []
    for obj in source_objects:
        if obj.type != "MESH":
            continue
        source_world = obj.matrix_world.copy()
        shell = obj.copy()
        shell.data = obj.data.copy()
        shell.animation_data_clear()
        shell.name = obj.name + "_OuterSilhouetteShell"
        shell.data.name = obj.data.name + "_OuterSilhouetteShellMesh"
        shell.data.materials.clear()
        shell.data.materials.append(black_mat)
        shell.parent = None
        collection.objects.link(shell)
        # The camera looks from negative Y toward the character. Pushing the shell
        # positive Y keeps the raw model in front so only the expanded rim survives.
        shell.matrix_world = Matrix.Translation(Vector((0.0, camera_back_offset, 0.0))) @ source_world @ Matrix.Scale(scale_factor, 4)
        shell.show_name = False
        shell.hide_select = True
        shell["outline_role"] = "outer_silhouette_shell"
        shell_objects.append(shell)
    return shell_objects


def make_white_mask_material():
    mat = bpy.data.materials.new("M_White_Mask_Render_Only")
    mat.diffuse_color = (1.0, 1.0, 1.0, 1.0)
    mat.use_nodes = True
    nodes = mat.node_tree.nodes
    nodes.clear()
    output = nodes.new("ShaderNodeOutputMaterial")
    emission = nodes.new("ShaderNodeEmission")
    emission.inputs["Color"].default_value = (1.0, 1.0, 1.0, 1.0)
    emission.inputs["Strength"].default_value = 1.0
    mat.node_tree.links.new(emission.outputs["Emission"], output.inputs["Surface"])
    return mat


def render_outline_mask(scene, right_objects):
    view_layer = scene.view_layers[0]
    white_mat = make_white_mask_material()
    old_filepath = scene.render.filepath
    old_transparent = scene.render.film_transparent
    old_override = view_layer.material_override
    old_hide_render = {obj.name: obj.hide_render for obj in scene.objects}
    right_names = {obj.name for obj in right_objects}

    for obj in scene.objects:
        obj.hide_render = obj.name not in right_names

    scene.render.film_transparent = True
    scene.render.filepath = str(MASK_PATH)
    scene.render.image_settings.file_format = "PNG"
    scene.render.image_settings.color_mode = "RGBA"
    view_layer.material_override = white_mat
    bpy.ops.render.render(write_still=True)

    view_layer.material_override = old_override
    scene.render.film_transparent = old_transparent
    scene.render.filepath = old_filepath
    for obj in scene.objects:
        if obj.name in old_hide_render:
            obj.hide_render = old_hide_render[obj.name]


def mask_from_png(path):
    image = bpy.data.images.load(str(path), check_existing=False)
    width, height = image.size
    pixels = image.pixels[:]
    mask = bytearray(width * height)
    for i in range(width * height):
        base = i * 4
        luminance = (pixels[base] + pixels[base + 1] + pixels[base + 2]) / 3.0
        if luminance > 0.5:
            mask[i] = 1
    bpy.data.images.remove(image)
    return mask, width, height


def external_outline_mask(mask, width, height, radius=7):
    outside = bytearray(width * height)
    queue = deque()

    def enqueue_if_outside(x, y):
        if x < 0 or y < 0 or x >= width or y >= height:
            return
        idx = (y * width) + x
        if mask[idx] or outside[idx]:
            return
        outside[idx] = 1
        queue.append((x, y))

    for x in range(width):
        enqueue_if_outside(x, 0)
        enqueue_if_outside(x, height - 1)
    for y in range(height):
        enqueue_if_outside(0, y)
        enqueue_if_outside(width - 1, y)

    while queue:
        x, y = queue.popleft()
        enqueue_if_outside(x + 1, y)
        enqueue_if_outside(x - 1, y)
        enqueue_if_outside(x, y + 1)
        enqueue_if_outside(x, y - 1)

    silhouette = bytearray(1 if not outside[i] else 0 for i in range(width * height))
    dilated = bytearray(silhouette)
    for _ in range(radius):
        previous = dilated
        dilated = bytearray(previous)
        for y in range(1, height - 1):
            row = y * width
            for x in range(1, width - 1):
                idx = row + x
                if previous[idx]:
                    continue
                if (
                    previous[idx - 1]
                    or previous[idx + 1]
                    or previous[idx - width]
                    or previous[idx + width]
                    or previous[idx - width - 1]
                    or previous[idx - width + 1]
                    or previous[idx + width - 1]
                    or previous[idx + width + 1]
                ):
                    dilated[idx] = 1

    return bytearray(1 if dilated[i] and outside[i] else 0 for i in range(width * height))


def save_outline_overlay(outline_mask, width, height, path):
    image = bpy.data.images.new("Hero1_OuterOnly_BlackOutline_Overlay", width=width, height=height, alpha=True, float_buffer=False)
    pixels = array("f", [0.0]) * (width * height * 4)
    for i, value in enumerate(outline_mask):
        if not value:
            continue
        base = i * 4
        pixels[base] = 0.0
        pixels[base + 1] = 0.0
        pixels[base + 2] = 0.0
        pixels[base + 3] = 1.0
    image.pixels.foreach_set(pixels)
    image.filepath_raw = str(path)
    image.file_format = "PNG"
    image.save()
    bpy.data.images.remove(image)


def add_outline_overlay_plane(scene, image_path, camera_depth=8.35):
    camera = scene.camera
    height = camera.data.ortho_scale
    width = height * (scene.render.resolution_x / scene.render.resolution_y)
    mesh = bpy.data.meshes.new("OuterOnlyOutlineOverlayPlane_Mesh")
    z = -abs(camera_depth)
    verts = [
        (-width / 2, -height / 2, z),
        (width / 2, -height / 2, z),
        (width / 2, height / 2, z),
        (-width / 2, height / 2, z),
    ]
    mesh.from_pydata(verts, [], [(0, 1, 2, 3)])
    mesh.update()
    uv_layer = mesh.uv_layers.new(name="UVMap")
    for loop_index, uv in zip(mesh.polygons[0].loop_indices, [(0, 0), (1, 0), (1, 1), (0, 1)]):
        uv_layer.data[loop_index].uv = uv

    obj = bpy.data.objects.new("OuterOnly_Black_Outline_Overlay", mesh)
    scene.collection.objects.link(obj)
    obj.parent = camera
    obj.matrix_parent_inverse.identity()
    obj.location = (0.0, 0.0, 0.0)
    obj.rotation_euler = (0.0, 0.0, 0.0)
    obj.scale = (1.0, 1.0, 1.0)
    mat = bpy.data.materials.new("M_OuterOnly_Black_Outline_Overlay")
    mat.use_nodes = True
    mat.blend_method = "BLEND"
    mat.use_backface_culling = False
    nodes = mat.node_tree.nodes
    nodes.clear()
    output = nodes.new("ShaderNodeOutputMaterial")
    bsdf = nodes.new("ShaderNodeBsdfPrincipled")
    texcoord = nodes.new("ShaderNodeTexCoord")
    tex = nodes.new("ShaderNodeTexImage")
    tex.image = bpy.data.images.load(str(image_path), check_existing=True)
    mat.node_tree.links.new(texcoord.outputs["UV"], tex.inputs["Vector"])
    mat.node_tree.links.new(tex.outputs["Color"], bsdf.inputs["Base Color"])
    mat.node_tree.links.new(tex.outputs["Alpha"], bsdf.inputs["Alpha"])
    mat.node_tree.links.new(bsdf.outputs["BSDF"], output.inputs["Surface"])
    obj.data.materials.append(mat)
    obj["outline_role"] = "camera_locked_outer_silhouette_overlay"
    return obj


def build_outer_outline_overlay(scene, right_objects):
    render_outline_mask(scene, right_objects)
    mask, width, height = mask_from_png(MASK_PATH)
    outline = external_outline_mask(mask, width, height, radius=7)
    save_outline_overlay(outline, width, height, OUTLINE_IMAGE_PATH)
    return OUTLINE_IMAGE_PATH


def composite_outline_over_base(base_path, outline_path, final_path):
    base_image = bpy.data.images.load(str(base_path), check_existing=False)
    outline_image = bpy.data.images.load(str(outline_path), check_existing=False)
    width, height = base_image.size
    if tuple(outline_image.size) != (width, height):
        raise RuntimeError(f"Outline image size {tuple(outline_image.size)} did not match base render {(width, height)}")
    base_pixels = array("f", base_image.pixels[:])
    outline_pixels = outline_image.pixels[:]
    for i in range(width * height):
        alpha = outline_pixels[(i * 4) + 3]
        if alpha <= 0.01:
            continue
        base = i * 4
        base_pixels[base] = 0.0
        base_pixels[base + 1] = 0.0
        base_pixels[base + 2] = 0.0
        base_pixels[base + 3] = 1.0
    final_image = bpy.data.images.new("Hero1_RawVsOuterOnlyOutline_Final", width=width, height=height, alpha=True, float_buffer=False)
    final_image.pixels.foreach_set(base_pixels)
    final_image.filepath_raw = str(final_path)
    final_image.file_format = "PNG"
    final_image.save()
    bpy.data.images.remove(base_image)
    bpy.data.images.remove(outline_image)
    bpy.data.images.remove(final_image)


def make_proof_image_scene(helper, image_path):
    scene = bpy.data.scenes.new("Raw_Pixal3D_OuterOnly_Final_Proof_Image")
    helper.set_render_defaults(scene, (1900, 1000), 1)
    image = bpy.data.images.load(str(image_path), check_existing=True)
    width = 6.2 * (image.size[0] / image.size[1])
    height = 6.2
    center_z = height / 2
    mesh = bpy.data.meshes.new("FinalProofImagePlane_Mesh")
    verts = [
        (-width / 2, 0, 0),
        (width / 2, 0, 0),
        (width / 2, 0, height),
        (-width / 2, 0, height),
    ]
    mesh.from_pydata(verts, [], [(0, 1, 2, 3)])
    mesh.update()
    uv_layer = mesh.uv_layers.new(name="UVMap")
    for loop_index, uv in zip(mesh.polygons[0].loop_indices, [(0, 0), (1, 0), (1, 1), (0, 1)]):
        uv_layer.data[loop_index].uv = uv
    obj = bpy.data.objects.new("Final_RawVsOuterOnlyOutline_Proof_Image", mesh)
    scene.collection.objects.link(obj)

    mat = bpy.data.materials.new("M_Final_RawVsOuterOnlyOutline_Proof_Image")
    mat.use_nodes = True
    nodes = mat.node_tree.nodes
    nodes.clear()
    output = nodes.new("ShaderNodeOutputMaterial")
    emission = nodes.new("ShaderNodeEmission")
    texcoord = nodes.new("ShaderNodeTexCoord")
    tex = nodes.new("ShaderNodeTexImage")
    tex.image = image
    mat.node_tree.links.new(texcoord.outputs["UV"], tex.inputs["Vector"])
    mat.node_tree.links.new(tex.outputs["Color"], emission.inputs["Color"])
    emission.inputs["Strength"].default_value = 1.0
    mat.node_tree.links.new(emission.outputs["Emission"], output.inputs["Surface"])
    obj.data.materials.append(mat)

    camera = helper.add_camera(scene, "Camera_FinalProof_Ortho", (0, -8, center_z), (0, 0, center_z), height)
    scene.camera = camera
    return scene


def write_recipe():
    recipe = {
        "asset": {
            "asset_id": "Hero_1_Chad_Male",
            "source_glb": str(SOURCE_GLB),
            "source_image": str(SOURCE_IMAGE),
            "blend_path": str(BLEND_PATH),
        },
        "scope": "Raw Pixal3D material preserved exactly; only the right-side duplicate receives a black Freestyle outline.",
        "material_changes": [],
        "outline": {
            "method": "Pixel-exact outer silhouette overlay generated from the right character luminance mask and composited over the raw comparison render",
            "line_set": None,
            "line_style": None,
            "color": [0.0, 0.0, 0.0],
            "alpha": 1.0,
            "mask_render": str(MASK_PATH),
            "base_render": str(BASE_RENDER_PATH),
            "overlay_image": str(OUTLINE_IMAGE_PATH),
            "final_render": str(RENDER_PATH),
            "outline_radius_px": 7,
            "edge_types": ["outer silhouette only"],
            "collection_filter": "RawOutline_Outlined_Pixal3D",
        },
        "lighting_rig": {
            "same_helper_as_previous_lookdev": True,
            "description": "Flat white world, soft shadowless area lights, Standard view transform.",
            "lights": [
                {"name": "Key_Softbox_L", "type": "AREA", "location": [-5.5, -5.2, 5.0], "energy": 470.0, "size": 6.5, "shadows": False},
                {"name": "Key_Softbox_R", "type": "AREA", "location": [5.5, -5.2, 4.6], "energy": 360.0, "size": 7.0, "shadows": False},
                {"name": "Top_Soft_Fill", "type": "AREA", "location": [0.0, -1.5, 7.2], "energy": 230.0, "size": 8.0, "shadows": False},
                {"name": "Front_Fill", "type": "AREA", "location": [0.0, -7.5, 2.1], "energy": 150.0, "size": 9.0, "shadows": False},
            ],
        },
        "non_actioned_future_ue5_port_note": "No Unreal work was done. Later, recreate this as a post-process/object-outline or inverted-hull outline material around the original mesh. Do not change the base material if the target is the raw Pixal3D look plus outline.",
    }
    RECIPE_JSON.write_text(json.dumps(recipe, indent=2), encoding="utf-8")

    lines = [
        "# Hero 1 Chad Male Raw Black Outline Recipe",
        "",
        "## Intent",
        "",
        "Keep the original Pixal3D output as the visual target and add only a black outline around the character.",
        "",
        "## Source",
        "",
        f"- Source GLB: `{SOURCE_GLB}`",
        f"- Source image: `{SOURCE_IMAGE}`",
        f"- Blender file: `{BLEND_PATH}`",
        "",
        "## Material",
        "",
        "- No material slots, textures, Principled BSDF values, colors, roughness, or geometry modifiers are changed on the character meshes.",
        "- Both left and right characters are raw duplicates of the imported GLB.",
        "- The right character differs only in the final proof render because a black outer-silhouette overlay is composited behind/around the raw right-side render.",
        "",
        "## Outline",
        "",
        "- Method: pixel-exact outer silhouette overlay generated from the right character luminance mask and composited over the raw comparison render.",
        "- Color: black `(0, 0, 0)`.",
        "- Alpha: `1.0`.",
        "- Outline radius: `7 px`.",
        f"- Base raw comparison render: `{BASE_RENDER_PATH}`.",
        f"- Mask render: `{MASK_PATH}`.",
        f"- Overlay image: `{OUTLINE_IMAGE_PATH}`.",
        f"- Final proof render: `{RENDER_PATH}`.",
        "- Edge behavior: outside silhouette only; no clothing, fold, face, or mesh-detail lines are authored.",
        "",
        "## Lighting Rig",
        "",
        "- Render engine: EEVEE Next when available, EEVEE fallback.",
        "- World color: white `(1, 1, 1)`.",
        "- View transform: `Standard`, look `Medium High Contrast`, exposure `0`, gamma `1`.",
        "- Shadowless soft area lights:",
        "  - `Key_Softbox_L`: location `(-5.5, -5.2, 5.0)`, energy `470`, size `6.5`.",
        "  - `Key_Softbox_R`: location `(5.5, -5.2, 4.6)`, energy `360`, size `7.0`.",
        "  - `Top_Soft_Fill`: location `(0, -1.5, 7.2)`, energy `230`, size `8.0`.",
        "  - `Front_Fill`: location `(0, -7.5, 2.1)`, energy `150`, size `9.0`.",
        "",
        "## Non-Actioned Future UE5 Port Note",
        "",
        "No Unreal work was done in this pass. Later, the UE5 reproduction should keep the current raw character material and add an outline as a separate object-outline pass or inverted-hull material. The base material should not be edited for this target.",
    ]
    RECIPE_MD.write_text("\n".join(lines) + "\n", encoding="utf-8")


def main():
    helper = load_helper()
    LOOKDEV_ROOT.mkdir(parents=True, exist_ok=True)
    RENDER_ROOT.mkdir(parents=True, exist_ok=True)

    helper.clear_scene()
    scene = bpy.context.scene
    scene.name = "Raw_Pixal3D_Outline_Only_Comparison"
    helper.set_render_defaults(scene, (1900, 1000), 1)
    helper.add_flat_lighting(scene, center=(0, 0, 1.5))
    helper.make_backdrop("Raw_Outline_Backdrop", 7.6, 3.6, 1.65)

    template_objects = helper.import_glb_template()
    helper.normalize_group(template_objects)
    template_coll = bpy.data.collections.new("RawOutline_Source_Template_Hidden")
    scene.collection.children.link(template_coll)
    for obj in template_objects:
        for coll in list(obj.users_collection):
            coll.objects.unlink(obj)
        template_coll.objects.link(obj)
        obj.hide_viewport = True
        obj.hide_render = True

    raw_root, raw_collection, raw_objects = helper.duplicate_group(
        template_objects,
        "RawOutline_Raw_Pixal3D",
        (-1.55, 0, 0),
        raw=True,
        target_height=2.55,
    )
    outlined_root, outlined_collection, outlined_objects = helper.duplicate_group(
        template_objects,
        "RawOutline_Outlined_Pixal3D",
        (1.55, 0, 0),
        raw=True,
        target_height=2.55,
    )
    assign_collection_recursive(raw_root, raw_collection)
    assign_collection_recursive(outlined_root, outlined_collection)

    helper.make_text("Raw Pixal3D", (-1.55, -0.05, 2.88), size=0.14)
    helper.make_text("Raw Pixal3D + Black Outline", (1.55, -0.05, 2.88), size=0.14)
    helper.add_camera(scene, "Camera_Raw_Outline_Ortho", (0, -8.0, 1.48), (0, 0, 1.42), 6.2)
    scene.render.use_freestyle = False
    bpy.context.view_layer.update()
    overlay_image = build_outer_outline_overlay(scene, outlined_objects)

    scene.render.filepath = str(BASE_RENDER_PATH)
    scene.render.image_settings.file_format = "PNG"
    scene.render.image_settings.color_mode = "RGBA"
    bpy.ops.render.render(write_still=True)
    composite_outline_over_base(BASE_RENDER_PATH, overlay_image, RENDER_PATH)
    proof_scene = make_proof_image_scene(helper, RENDER_PATH)
    try:
        bpy.context.window.scene = proof_scene
    except Exception:
        pass
    write_recipe()
    bpy.ops.wm.save_as_mainfile(filepath=str(BLEND_PATH))

    summary = {
        "blend_path": str(BLEND_PATH),
        "render_path": str(RENDER_PATH),
        "recipe_json": str(RECIPE_JSON),
        "recipe_md": str(RECIPE_MD),
        "source_glb": str(SOURCE_GLB),
        "raw_object_count": len(raw_objects),
        "outlined_object_count": len(outlined_objects),
        "outline_method": "Pixel-exact outer silhouette overlay from luminance mask",
        "outline_radius_px": 7,
        "base_render_path": str(BASE_RENDER_PATH),
        "mask_path": str(MASK_PATH),
        "outline_overlay_image": str(OUTLINE_IMAGE_PATH),
        "proof_scene": proof_scene.name,
    }
    (LOOKDEV_ROOT / "raw_outline_build_summary.json").write_text(json.dumps(summary, indent=2), encoding="utf-8")
    print("T66_RAW_OUTLINE_SUMMARY=" + json.dumps(summary, sort_keys=True))


if __name__ == "__main__":
    main()
