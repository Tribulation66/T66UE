import json
import os
import shutil

import bpy


PROJECT_ROOT = r"C:\UE\T66"
RIG_ROOT = os.path.join(
    PROJECT_ROOT,
    "Model Generation",
    "Runs",
    "Heroes",
    "Chad_Pass02_ProcessBuild",
    "Rigging",
    "Mike_Chad_RigPrototype_A03_LiftedNeckBridge",
)
OUT_DIR = os.path.join(RIG_ROOT, "UnrealMaterialRepair")
TEXTURE_DIR = os.path.join(OUT_DIR, "ExtractedTextures")
REPORT_PATH = os.path.join(OUT_DIR, "blender_material_report.json")


def ensure_dir(path):
    os.makedirs(path, exist_ok=True)


def safe_name(name):
    return "".join(c if c.isalnum() or c in "._-" else "_" for c in name)


def image_output_path(image):
    base = safe_name(image.name or "image")
    return os.path.join(TEXTURE_DIR, f"{base}.png")


def export_image(image):
    if not image:
        return None
    ensure_dir(TEXTURE_DIR)
    out_path = image_output_path(image)
    if image.packed_file:
        previous_path = image.filepath_raw
        previous_format = image.file_format
        image.filepath_raw = out_path
        image.file_format = "PNG"
        image.save()
        image.filepath_raw = previous_path
        image.file_format = previous_format
        return out_path

    source = bpy.path.abspath(image.filepath) if image.filepath else ""
    if source and os.path.isfile(source):
        shutil.copy2(source, out_path)
        return out_path
    return None


def material_report(mat):
    entry = {
        "name": mat.name if mat else None,
        "diffuse_color": list(mat.diffuse_color) if mat else None,
        "images": [],
        "principled": [],
    }
    if not mat or not mat.use_nodes:
        return entry
    for node in mat.node_tree.nodes:
        if node.type == "TEX_IMAGE" and node.image:
            entry["images"].append(
                {
                    "node": node.name,
                    "image": node.image.name,
                    "source": bpy.path.abspath(node.image.filepath) if node.image.filepath else "",
                    "size": list(node.image.size),
                    "packed": bool(node.image.packed_file),
                    "exported": export_image(node.image),
                }
            )
        elif node.type == "BSDF_PRINCIPLED":
            base = node.inputs.get("Base Color")
            entry["principled"].append(
                {
                    "node": node.name,
                    "base_default": list(base.default_value) if base else None,
                }
            )
    return entry


def main():
    ensure_dir(OUT_DIR)
    meshes = []
    for obj in bpy.data.objects:
        if obj.type != "MESH":
            continue
        meshes.append(
            {
                "object": obj.name,
                "vertices": len(obj.data.vertices),
                "materials": [material_report(slot.material) for slot in obj.material_slots],
            }
        )

    images = []
    for image in bpy.data.images:
        images.append(
            {
                "name": image.name,
                "source": bpy.path.abspath(image.filepath) if image.filepath else "",
                "size": list(image.size),
                "packed": bool(image.packed_file),
            }
        )

    with open(REPORT_PATH, "w", encoding="utf-8") as handle:
        json.dump({"meshes": meshes, "images": images}, handle, indent=2)
    print(REPORT_PATH)


if __name__ == "__main__":
    main()
