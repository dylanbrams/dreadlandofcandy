"""Build/export the eastern Riverlands terrain extension and its props.

Run with Blender 4.5: blender -b -t 4 --python ImportSource/Terrain/Expansion/export_extension.py
Coordinates are Unreal centimeters, matching Terrain/export_blender.py.
"""
from pathlib import Path
import math
import bpy
from mathutils import Matrix

ROOT = Path(__file__).resolve().parent
OUT = ROOT / "FBX"
OUT.mkdir(parents=True, exist_ok=True)
bpy.ops.object.select_all(action="SELECT")
bpy.ops.object.delete(use_global=False)
bpy.context.scene.unit_settings.system = "METRIC"
bpy.context.scene.unit_settings.scale_length = .01

def material(name, color):
    m = bpy.data.materials.new(name)
    m.diffuse_color = (*color, 1)
    m.use_nodes = True
    m.node_tree.nodes.get("Principled BSDF").inputs["Base Color"].default_value = (*color, 1)
    m.node_tree.nodes.get("Principled BSDF").inputs["Roughness"].default_value = .88
    return m

grass = material("Riverlands_PastelGrass", (.43, .72, .39))
mint = material("Riverlands_MintMeadow", (.50, .82, .68))
bank = material("Riverlands_SandyRiverBank", (.83, .72, .48))
water = material("Riverlands_MintWater", (.24, .76, .78))
wood = material("WaferBridge_GoldenWafer", (.76, .43, .19))
railmat = material("WaferBridge_CreamIcing", (.98, .83, .52))

BED, LIP, WATER_Z, WATER_HALF = -260.0, 60.0, -70.0, 620.0

def river(y):
    return 15000 + 500 * math.sin(y / 3500)

def terrain_z(x, y):
    # Match the importer: flat western shelf, eastward low hills, clear camp pad.
    if x < 16000:
        hills = 0.0
    else:
        hills = 220 * math.sin((x - 16000) / 6000 * math.pi) ** 2 * (0.5 + 0.5 * math.cos(y / 5000))
    camp_distance = math.hypot(x - 18200, y)
    camp_t = max(0.0, min(1.0, (camp_distance - 1000) / 800))
    camp_blend = camp_t * camp_t * (3 - 2 * camp_t)
    hills *= camp_blend
    d = abs(x - river(y))
    # Fixes4: a real channel. Deep bed, steep candy banks rising to a raised
    # lip (+60) either side, then blending back into the meadow.
    def ss(t):
        t = max(0.0, min(1.0, t)); return t * t * (3 - 2 * t)
    if d <= 380:
        return BED
    if d < 660:
        return BED + (LIP - BED) * ss((d - 380) / 280)
    if d < 900:
        return LIP
    if d < 1300:
        return LIP + (hills - LIP) * ss((d - 900) / 400)
    return hills

def export_mesh(name, verts, faces, mats, face_mats):
    mesh = bpy.data.meshes.new(name)
    mesh.from_pydata(verts, [], faces)
    mesh.update()
    for m in mats: mesh.materials.append(m)
    for p, mi in zip(mesh.polygons, face_mats): p.material_index = mi
    obj = bpy.data.objects.new(name, mesh)
    bpy.context.scene.collection.objects.link(obj)
    bpy.ops.object.select_all(action="DESELECT")
    obj.select_set(True)
    bpy.context.view_layer.objects.active = obj
    # Same UE handedness compensation and FBX settings as the terrain exporter.
    original = obj.data
    export = original.copy()
    export.transform(Matrix.Diagonal((1, -1, 1, 1)))
    export.flip_normals()
    obj.data = export
    bpy.ops.export_scene.fbx(filepath=str(OUT / (name + ".fbx")), use_selection=True,
        object_types={"MESH"}, apply_unit_scale=True, axis_forward="-Y", axis_up="Z",
        bake_anim=False, use_mesh_modifiers=True, mesh_smooth_type="FACE", add_leaf_bones=False)
    obj.data = original
    bpy.data.meshes.remove(export)
    tris = sum(len(p.vertices)-2 for p in original.polygons)
    print(f"EXPORTED {name}: verts={len(original.vertices)} tris={tris} bounds=" + str(tuple(
        (min(v.co[i] for v in original.vertices), max(v.co[i] for v in original.vertices)) for i in range(3))))
    return obj

# Grid vertices include the exact seam at x=11000 and 250 cm spacing.
# Actor placement adds +4000 X, so this seam lands exactly at world X=16000.
# Fixes4: 80 cm columns through the meander band so the banks stay crisp.
xs = sorted(set(list(range(12000, 22001, 250)) + list(range(13200, 16801, 80))))
ys = list(range(-12000, 12001, 160))
verts = [(x, y, terrain_z(x, y)) for y in ys for x in xs]
faces, mids = [], []
for j in range(len(ys)-1):
    for i in range(len(xs)-1):
        a = j*len(xs)+i
        faces.append((a, a+1, a+1+len(xs), a+len(xs)))
        xc=(xs[i]+xs[i+1])*.5; yc=(ys[j]+ys[j+1])*.5
        d=abs(xc-river(yc))
        mids.append(1 if d < 1000 and ((i+j)//8)%3 else (2 if d < 1000 else (1 if ((i//13+j//11)%7==0) else 0)))
terrain = export_mesh("SM_Riverlands", verts, faces, [grass,mint,bank], mids)

# Fixes4: water fills the channel bank to bank; its edges run under the banks.
wverts=[]
WCOLS=8
for y in range(-12000,12001,100):
    cx=river(y)
    wverts.extend([(cx-WATER_HALF+2*WATER_HALF*k/WCOLS,y,WATER_Z) for k in range(WCOLS+1)])
wfaces=[]
for j in range(len(wverts)//(WCOLS+1)-1):
    for k in range(WCOLS):
        a=j*(WCOLS+1)+k; wfaces.append((a,a+1,a+WCOLS+2,a+WCOLS+1))
export_mesh("SM_RiverWater",wverts,wfaces,[water],[0]*len(wfaces))

# Wafer span aligned across the stream (east-west), broad open deck and low icing rails.
# Deck is segmented into a gentle arch, with ramps meeting z=0 at both ends.
def box(name, center, size, mat):
    bpy.ops.mesh.primitive_cube_add(size=1, location=center)
    o=bpy.context.object; o.name=name; o.dimensions=size
    bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
    o.data.materials.append(mat)
    return o

parts=[]
for k in range(13):
    x=14000+k*(2200/12)
    z=40*math.sin(math.pi*k/12)
    parts.append(box("deck",(x,0,z),(2200/12+5,650,70),wood))
# Low rails at deck edges: short supports plus long rails follow the arch in small segments.
for side in (-1,1):
    for k in range(13):
        x=14000+k*(2200/12); z=40*math.sin(math.pi*k/12)
        parts.append(box("rail",(x,side*300,z+200),(42,52,330),railmat))
        if k<12:
            xmid=x+2200/24; zm=40*math.sin(math.pi*(k+.5)/12)
            parts.append(box("rail_top",(xmid,side*300,zm+360),(2200/12+5,65,55),railmat))
# Export bridge as one merged mesh while keeping separate material slots.
bpy.ops.object.select_all(action="DESELECT")
for o in parts: o.select_set(True)
bpy.context.view_layer.objects.active=parts[0]
bpy.ops.object.join()
joined=bpy.context.object; joined.name="SM_WaferBridge"
# Apply the joined object's translation so FBX vertices remain in world coordinates.
bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)
# Join preserves materials; perform handedness conversion through common exporter.
src=joined.data
verts=[tuple(v.co) for v in src.vertices]
faces=[tuple(p.vertices) for p in src.polygons]
matlist=list(src.materials)
fm=[p.material_index for p in src.polygons]
bpy.data.objects.remove(joined,do_unlink=True)
export_mesh("SM_WaferBridge",verts,faces,matlist,fm)
print("RIVERLANDS EXTENSION EXPORT PASS")
