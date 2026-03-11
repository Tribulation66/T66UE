import unreal
from collections import deque

LOG = '[RetroGeometryRepair]'
MATERIALS = [
    '/Game/Materials/M_Character_Unlit',
    '/Game/Materials/M_Environment_Unlit',
    '/Game/Materials/M_FBX_Unlit',
    '/Game/Materials/M_GLB_Unlit',
]
DEFAULT_TEXTURE_CANDIDATES = [
    '/Engine/EngineResources/WhiteSquareTexture.WhiteSquareTexture',
    '/Engine/EngineResources/DefaultTexture.DefaultTexture',
]
PARENT_PARAM_MAP = {
    'M_Character_Unlit': 'DiffuseColorMap',
    'M_Environment_Unlit': 'DiffuseColorMap',
    'M_FBX_Unlit': 'DiffuseColorMap',
    'M_GLB_Unlit': 'BaseColorTexture',
}
mel = unreal.MaterialEditingLibrary


def log(text):
    unreal.log(f'{LOG} {text}')


def load_asset(path):
    asset = unreal.EditorAssetLibrary.load_asset(path)
    if asset:
        return asset
    return unreal.load_asset(path)


def load_default_texture():
    for path in DEFAULT_TEXTURE_CANDIDATES:
        texture = load_asset(path)
        if texture:
            log(f'Using default texture {path}')
            return texture
    raise RuntimeError('Failed to load a fallback default texture.')


def iter_connected_expressions(material, root_expr):
    if not root_expr:
        return

    queue = deque([root_expr])
    visited = set()

    while queue:
        expr = queue.popleft()
        if not expr:
            continue

        expr_path = expr.get_path_name()
        if expr_path in visited:
            continue
        visited.add(expr_path)
        yield expr

        try:
            inputs = mel.get_inputs_for_material_expression(material, expr)
        except Exception as exc:
            log(f'Failed to inspect inputs for {expr.get_name()}: {exc}')
            continue

        for item in inputs or []:
            if item:
                queue.append(item)


def patch_parent_material(material_path, default_texture):
    material = load_asset(material_path)
    if not material:
        raise RuntimeError(f'Missing material: {material_path}')

    root = mel.get_material_property_input_node(material, unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    if not root:
        log(f'No emissive root on {material_path}; skipping')
        return False

    changed = False
    for expr in iter_connected_expressions(material, root):
        if expr.get_class().get_name() != 'MaterialExpressionTextureSampleParameter2D':
            continue

        texture = expr.get_editor_property('texture')
        if texture:
            continue

        expr.set_editor_property('texture', default_texture)
        param_name = expr.get_editor_property('parameter_name')
        log(f'Set default texture on {material_path} param {param_name}')
        changed = True

    if changed:
        mel.recompile_material(material)
        unreal.EditorAssetLibrary.save_loaded_asset(material)
    else:
        log(f'No null connected texture params on {material_path}')

    return changed


def get_texture_value(material_instance, parameter_name):
    try:
        value = mel.get_material_instance_texture_parameter_value(material_instance, parameter_name)
    except Exception:
        return None

    if isinstance(value, (tuple, list)):
        return value[-1] if value else None
    return value


def repair_material_instances():
    changed_count = 0
    scanned_count = 0
    asset_paths = unreal.EditorAssetLibrary.list_assets('/Game', recursive=True, include_folder=False)

    for asset_path in asset_paths:
        asset = unreal.EditorAssetLibrary.load_asset(asset_path)
        if not asset or not isinstance(asset, unreal.MaterialInstanceConstant):
            continue

        parent = None
        try:
            parent = asset.get_editor_property('parent')
        except Exception:
            continue
        if not parent:
            continue

        parent_name = parent.get_name()
        parameter_name = PARENT_PARAM_MAP.get(parent_name)
        if not parameter_name:
            continue

        scanned_count += 1
        texture = get_texture_value(asset, parameter_name)
        if not texture:
            continue

        mel.set_material_instance_texture_parameter_value(asset, parameter_name, texture)
        mel.update_material_instance(asset)
        unreal.EditorAssetLibrary.save_loaded_asset(asset)
        changed_count += 1
        log(f'Reapplied {parameter_name} on {asset_path}')

    log(f'Repaired {changed_count} material instances (scanned {scanned_count})')


def main():
    default_texture = load_default_texture()
    parent_changes = 0
    for material_path in MATERIALS:
        if patch_parent_material(material_path, default_texture):
            parent_changes += 1
    log(f'Patched {parent_changes} parent materials')
    repair_material_instances()
    log('Retro geometry visibility repair complete')


if __name__ == '__main__':
    main()
