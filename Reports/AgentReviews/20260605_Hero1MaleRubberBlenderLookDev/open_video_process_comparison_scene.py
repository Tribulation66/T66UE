import bpy

SCENE_NAME = "Transcript_Process_Comparison"
CAMERA_NAME = "Camera_Transcript_Process_Ortho"

scene = bpy.data.scenes.get(SCENE_NAME)
if scene and bpy.context.window:
    bpy.context.window.scene = scene
    camera = bpy.data.objects.get(CAMERA_NAME)
    if camera:
        scene.camera = camera

if bpy.context.screen:
    for area in bpy.context.screen.areas:
        if area.type == "VIEW_3D":
            for space in area.spaces:
                if space.type == "VIEW_3D" and space.region_3d:
                    space.region_3d.view_perspective = "CAMERA"

