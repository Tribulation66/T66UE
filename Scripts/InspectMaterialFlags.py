import unreal

MATERIALS = [
    '/Game/Materials/M_Character_Unlit',
    '/Game/Materials/M_Environment_Unlit',
    '/Game/Materials/M_FBX_Unlit',
    '/Game/Materials/M_GLB_Unlit',
]

for path in MATERIALS:
    mat = unreal.EditorAssetLibrary.load_asset(path)
    unreal.log('[RetroMatFlags] ============================================================')
    unreal.log('[RetroMatFlags] Material=' + path)
    if not mat:
        unreal.log('[RetroMatFlags] Missing')
        continue
    for prop in ('shading_model', 'blend_mode', 'two_sided', 'used_with_instanced_static_meshes', 'used_with_nanite', 'dithered_lod_transition', 'use_material_attributes'):
        try:
            unreal.log(f'[RetroMatFlags] {prop}={mat.get_editor_property(prop)}')
        except Exception as exc:
            unreal.log(f'[RetroMatFlags] {prop} failed: {exc}')
