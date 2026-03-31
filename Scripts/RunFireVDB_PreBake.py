import importlib.util
import bpy
spec = importlib.util.spec_from_file_location('firevdb', r'C:\UE\T66\Scripts\GenerateFireExplosionVDB.py')
mod = importlib.util.module_from_spec(spec)
spec.loader.exec_module(mod)
print('STEP ensure_dirs')
mod.ensure_dirs()
print('STEP clean_scene')
mod.clean_scene()
print('STEP configure_scene')
scene = mod.configure_scene()
print('STEP ground')
mod.add_ground()
print('STEP cameras')
hero, side = mod.add_cameras(scene)
print('STEP lights')
mod.add_lights()
print('STEP core')
core = mod.add_fire_flow('CoreBurst',(0.0,0.0,0.82),(0.92,0.92,0.62),((1,0.0),(2,1.2),(3,1.0),(4,0.45),(5,0.0)),((1,0.0),(2,1.8),(3,1.6),(4,0.55),(5,0.0)),((1,0.0),(2,2.0),(3,1.8),(4,0.7),(5,0.0)),(0.0,0.0,2.1),1.35,0.55)
print('STEP shell')
shell = mod.add_fire_flow('ShellBurst',(0.0,0.0,0.58),(1.55,1.55,0.42),((1,0.0),(2,0.95),(3,0.82),(4,0.18),(5,0.0)),((1,0.0),(2,1.15),(3,0.95),(4,0.18),(5,0.0)),((1,0.0),(2,1.5),(3,1.2),(4,0.3),(5,0.0)),(0.0,0.0,1.05),1.55,0.48)
print('STEP lobe')
lobe = mod.add_fire_flow('AsymLobe',(0.34,-0.22,0.96),(0.62,0.50,0.72),((1,0.0),(3,0.55),(4,0.72),(6,0.12),(7,0.0)),((1,0.0),(3,0.85),(4,1.0),(6,0.18),(7,0.0)),((1,0.0),(3,1.3),(4,1.55),(6,0.28),(7,0.0)),(0.45,-0.15,1.55),0.82,0.62)
print('STEP scale keys')
for obj, keys in (
    (core, ((1, (0.82, 0.82, 0.56)), (3, (1.16, 1.16, 0.76)), (5, (1.32, 1.32, 0.92)))),
    (shell, ((1, (1.40, 1.40, 0.34)), (3, (1.95, 1.95, 0.44)), (5, (2.20, 2.20, 0.55)))),
    (lobe, ((2, (0.56, 0.45, 0.62)), (4, (0.76, 0.62, 0.96)), (7, (0.96, 0.82, 1.18)))),
):
    for frame, scale in keys:
        obj.scale = scale
        obj.keyframe_insert(data_path='scale', frame=frame)
print('STEP force fields')
mod.add_force_fields()
print('STEP domain')
domain = mod.configure_domain(scene)
print('STEP save')
bpy.ops.wm.save_as_mainfile(filepath=mod.BLEND_PATH)
print('DONE pre-bake')
