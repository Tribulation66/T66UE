import bpy
mat = bpy.data.materials.new('Probe')
mat.use_nodes = True
node = mat.node_tree.nodes.new('ShaderNodeVolumePrincipled')
print('inputs', [s.name for s in node.inputs])
print('outputs', [s.name for s in node.outputs])
