import unreal

for name in sorted(dir(unreal.MaterialEditingLibrary)):
    if 'material' in name.lower() or 'expression' in name.lower() or 'parameter' in name.lower():
        unreal.log('[RetroMatApi] ' + name)
