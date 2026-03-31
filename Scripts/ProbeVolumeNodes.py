import bpy
mat = bpy.data.materials.new('Probe')
mat.use_nodes = True
nodes = mat.node_tree.nodes
for node_id in [
    'ShaderNodePrincipledVolume',
    'ShaderNodeVolumeAbsorption',
    'ShaderNodeVolumeScatter',
    'ShaderNodeVolumeInfo',
    'ShaderNodeEmission',
    'ShaderNodeAddShader',
    'ShaderNodeMath',
    'ShaderNodeMapRange'
]:
    try:
        node = nodes.new(node_id)
        print('OK', node_id, type(node).__name__)
    except Exception as e:
        print('FAIL', node_id, e)
