import os, bpy
for root, _, files in os.walk(r'C:\UE\T66\SourceAssets\Import\VFX\Idol_Fire_Explosion\vdb_cache'):
    for name in files:
        if name.endswith('.vdb'):
            print(os.path.join(root, name), os.path.getsize(os.path.join(root, name)))
            raise SystemExit
print('NO_VDB')
