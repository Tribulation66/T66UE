import bpy
mat = bpy.data.materials.new('Probe')
mat.use_nodes = True
nodes = mat.node_tree.nodes
print('TYPES_WITH_VOLUME_OR_PRINCIPLED')
for name in sorted(dir(bpy.types)):
    if 'Volume' in name or 'Principled' in name:
        if name.startswith('ShaderNode'):
            print(name)
print('SOCKETS')
for node_id in ['ShaderNodeVolumeAbsorption','ShaderNodeVolumeScatter','ShaderNodeVolumeInfo','ShaderNodeEmission','ShaderNodeBsdfPrincipled']:
    try:
        node = nodes.new(node_id)
        print('NODE', node_id)
        print('  inputs', [s.name for s in node.inputs])
        print('  outputs', [s.name for s in node.outputs])
    except Exception as e:
        print('FAIL', node_id, e)
