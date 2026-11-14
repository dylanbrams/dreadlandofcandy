"""Read bridge Blend assets, report world bounds/deck samples and render a preview."""
from pathlib import Path
import bpy
from mathutils import Vector

PROJECT = Path(__file__).resolve().parents[3]
ASSETS = PROJECT / "Assets" / "DylanGenerationFoliageThree" / "Clean"
OUT = Path(__file__).resolve().parent / "bridge_contact.png"

records=[]
for name in ("SM_Bridge125", "SM_Bridge126"):
    bpy.ops.wm.open_mainfile(filepath=str(ASSETS/(name+".blend")))
    meshes=[o for o in bpy.context.scene.objects if o.type=="MESH"]
    print("ASSET",name,"mesh objects",len(meshes))
    coords=[]
    for o in meshes:
        coords.extend([o.matrix_world @ v.co for v in o.data.vertices])
    lo=Vector([min(v[i] for v in coords) for i in range(3)])
    hi=Vector([max(v[i] for v in coords) for i in range(3)])
    dims=hi-lo
    axis=0 if dims.x>=dims.y else 1
    cross=1-axis
    print("BOUNDS_M",tuple(round(v,6) for v in lo),tuple(round(v,6) for v in hi),"long_axis",("X" if axis==0 else "Y"),"dimensions_m",tuple(round(v,6) for v in dims))
    # Find deck height: upper surface vertex distribution at each longitudinal end and midpoint.
    for label,t in (("end_a",.05),("center",.5),("end_b",.95)):
        target=lo[axis]+dims[axis]*t
        near=[v for v in coords if abs(v[axis]-target)<=dims[axis]*.045 and abs(v[cross]-(lo[cross]+hi[cross])/2)<=dims[cross]*.18]
        zsorted=sorted(v.z for v in near)
        print("DECK_SAMPLE",name,label,"long_cm",round(target*100,2),"z_top_cm",round(zsorted[-1]*100,2) if zsorted else None,"z_q80_cm",round(zsorted[int(.8*(len(zsorted)-1))]*100,2) if zsorted else None,"n",len(near))
    records.append((name,lo,hi,axis))

# Read-only source assets above; compose simple contact scene with two preview copies.
bpy.ops.wm.read_factory_settings(use_empty=True)
for idx,(name,lo,hi,axis) in enumerate(records):
    src=bpy.data.libraries.load(str(ASSETS/(name+".blend")),link=False)
    with src as (data_from,data_to):
        data_to.objects=[n for n in data_from.objects if n]
    for o in data_to.objects:
        if o is None: continue
        bpy.context.scene.collection.objects.link(o)
        o.location.x += (-.62 if idx==0 else .62)

world=bpy.data.worlds.new("PreviewWorld")
world.use_nodes=True
world.node_tree.nodes["Background"].inputs[0].default_value=(.42,.48,.54,1)
world.node_tree.nodes["Background"].inputs[1].default_value=.8
bpy.context.scene.world=world
light_data=bpy.data.lights.new("PreviewKey","AREA"); light_data.energy=1800; light_data.size=4
light=bpy.data.objects.new("PreviewKey",light_data); bpy.context.scene.collection.objects.link(light); light.location=(0,-2,3)
light.rotation_euler=(0,0,0)
cam_data=bpy.data.cameras.new("PreviewCamera"); cam=bpy.data.objects.new("PreviewCamera",cam_data); bpy.context.scene.collection.objects.link(cam)
cam.location=(0,-2.6,2.4); target=Vector((0,0,.2)); cam.rotation_euler=(target-Vector(cam.location)).to_track_quat("-Z","Y").to_euler(); cam_data.type="ORTHO"; cam_data.ortho_scale=4.5
bpy.context.scene.camera=cam
scene=bpy.context.scene; scene.render.engine="BLENDER_EEVEE_NEXT"; scene.render.resolution_x=1200; scene.render.resolution_y=700; scene.render.resolution_percentage=100
scene.render.image_settings.file_format="PNG"; scene.render.filepath=str(OUT)
bpy.ops.render.render(write_still=True)
print("BRIDGE CONTACT PASS",OUT)
