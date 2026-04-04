import bpy
import os

src = r"B:/zhoenus-ue5/Zhoenus/Art/GoalFix/goal-v1.fbx"
out_fbx = r"B:/zhoenus-ue5/Zhoenus/Art/GoalFix/goal-v1-fixed.fbx"
out_blend = r"B:/zhoenus-ue5/Zhoenus/Art/GoalFix/goal-v1-fixed.blend"

bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.import_scene.fbx(filepath=src)

for obj in [o for o in bpy.context.scene.objects if o.type == 'MESH']:
    bpy.context.view_layer.objects.active = obj
    obj.select_set(True)
    bpy.ops.object.mode_set(mode='EDIT')
    bpy.ops.mesh.select_all(action='SELECT')
    bpy.ops.mesh.normals_make_consistent(inside=False)
    bpy.ops.mesh.remove_doubles(threshold=0.0001)
    bpy.ops.object.mode_set(mode='OBJECT')
    obj.select_set(False)

for obj in bpy.context.scene.objects:
    obj.select_set(obj.type == 'MESH')

bpy.ops.export_scene.fbx(
    filepath=out_fbx,
    use_selection=True,
    apply_unit_scale=True,
    bake_space_transform=False,
    add_leaf_bones=False,
    path_mode='COPY',
    embed_textures=False
)

bpy.ops.wm.save_as_mainfile(filepath=out_blend)
print('BLENDER_FIX_DONE', out_fbx)
print('BLENDER_BLEND_DONE', out_blend)
