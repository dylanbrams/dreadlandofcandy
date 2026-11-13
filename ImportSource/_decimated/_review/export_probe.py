import bpy,bmesh,sys,importlib.util,json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[3]
sys.dont_write_bytecode=True
spec=importlib.util.spec_from_file_location('decimate',ROOT/'decimate.py')
decimate=importlib.util.module_from_spec(spec);spec.loader.exec_module(decimate)
bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.import_scene.gltf(filepath=str(ROOT/'ImportSource/Hyper3D/SM_Fenella.glb'))
obj=next(o for o in bpy.context.scene.objects if o.type=='MESH')
bpy.context.view_layer.objects.active=obj
def stats(label):
    bm=bmesh.new();bm.from_mesh(obj.data)
    print(label,'verts',len(bm.verts),'boundary',sum(e.is_boundary for e in bm.edges),'nonmanifold',sum(len(e.link_faces)>2 for e in bm.edges),flush=True);bm.free()
stats('import')
print('transform',list(obj.matrix_world),'dimensions',list(obj.dimensions),flush=True)
obj.data.transform(obj.matrix_world);obj.matrix_world.identity();bpy.context.view_layer.update()
stats('transformed')
bm=bmesh.new();bm.from_mesh(obj.data)
bmesh.ops.remove_doubles(bm,verts=list(bm.verts),dist=max(obj.dimensions)*1e-7)
bm.to_mesh(obj.data);bm.free();obj.data.update();stats('welded')
obj.data.calc_loop_triangles()
mod=obj.modifiers.new('probe','DECIMATE');mod.ratio=25000/len(obj.data.loop_triangles);mod.use_collapse_triangulate=True
bpy.ops.object.modifier_apply(modifier=mod.name);stats('collapsed')
for stage in ('before_finish','after_finish'):
    if stage=='after_finish':decimate.cleanup.finish(obj,'SM_Fenella');stats('finished')
    path=ROOT/'ImportSource/_decimated/_review'/('probe_'+stage+'.glb')
    bpy.ops.export_scene.gltf(filepath=str(path),export_format='GLB',export_materials='EXPORT',export_texcoords=True,export_normals=True)
    print(stage,decimate.exported_topology(path),flush=True)
