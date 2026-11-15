from pathlib import Path
import bpy
from math import cos, pi, sqrt
from mathutils import Vector

root = Path(__file__).resolve().parents[1]
source = root / 'ImportSource/Meshy/SM_UltMint_C.glb'
out = root / 'ImportSource/FX/MeshyMint'

bpy.ops.object.select_all(action='SELECT')
bpy.ops.object.delete(use_global=False)
bpy.ops.import_scene.gltf(filepath=str(source))

# The GLB has an unrelated six-face helper cube alongside the peppermint.
meshes = [o for o in bpy.context.scene.objects if o.type == 'MESH' and len(o.data.polygons) > 100]
if len(meshes) != 1:
    raise RuntimeError(f'Expected one peppermint mesh, found {len(meshes)}')
obj = meshes[0]
for other in list(bpy.context.scene.objects):
    if other != obj:
        bpy.data.objects.remove(other, do_unlink=True)
bpy.ops.object.select_all(action='DESELECT')
bpy.context.view_layer.objects.active = obj
obj.select_set(True)
bpy.ops.object.transform_apply(location=False, rotation=True, scale=True)

# Make a rounded candy cylinder from the sphere. Keep the Z axis as the axis
# of rotation and carry UVs/materials with their vertices. The broad middle
# forms the side wall; a cosine bevel rounds both ends into the top and bottom.
points = [obj.matrix_world @ vert.co for vert in obj.data.vertices]
lo = Vector(tuple(min(p[i] for p in points) for i in range(3)))
hi = Vector(tuple(max(p[i] for p in points) for i in range(3)))
center = (lo + hi) / 2
radius = max((hi.x - lo.x) / 2, (hi.y - lo.y) / 2)
half_height = (hi.z - lo.z) / 2
flat_fraction = 0.55
height_ratio = 0.65

for vert, point in zip(obj.data.vertices, points):
    x, y, z = (point - center)
    zn = max(-1.0, min(1.0, z / half_height))
    source_radial = sqrt(x * x + y * y) / radius
    sphere_radial = sqrt(max(1e-8, 1.0 - zn * zn))
    ring_radius = min(1.0, source_radial / sphere_radial)
    az = abs(zn)
    if az <= flat_fraction:
        profile = 1.0
    else:
        t = min(1.0, (az - flat_fraction) / (1.0 - flat_fraction))
        profile = cos(t * pi / 2)
    angle_scale = (profile * ring_radius) / max(1e-8, source_radial)
    vert.co = Vector((x * angle_scale, y * angle_scale, zn * height_ratio * radius))

obj.matrix_world.identity()
target_diameter = 0.35
scale = target_diameter / (2 * radius)
for vert in obj.data.vertices:
    vert.co *= scale

before = len(obj.data.polygons)
if before > 15000:
    decimate = obj.modifiers.new('Peppermint mesh reduction', 'DECIMATE')
    decimate.ratio = 15000 / before
    bpy.ops.object.modifier_apply(modifier=decimate.name)

final_points = [obj.matrix_world @ vert.co for vert in obj.data.vertices]
final_lo = Vector(tuple(min(p[i] for p in final_points) for i in range(3)))
final_hi = Vector(tuple(max(p[i] for p in final_points) for i in range(3)))
print('MINT source polygons', before)
print('MINT final polygons', len(obj.data.polygons), 'triangles', sum(len(poly.vertices) - 2 for poly in obj.data.polygons))
print('MINT final bounds m', tuple(round(x, 6) for x in (final_hi - final_lo)))
print('MINT final center m', tuple(round(x, 6) for x in ((final_hi + final_lo) / 2)))

out.mkdir(parents=True, exist_ok=True)
for i, image in enumerate(bpy.data.images):
    if image.type == 'IMAGE' and image.size[0] > 0:
        image.filepath_raw = str(out / f'texture_{i}.png')
        image.file_format = 'PNG'
        image.save()

bpy.ops.object.select_all(action='DESELECT')
obj.select_set(True)
bpy.context.view_layer.objects.active = obj
bpy.ops.export_scene.fbx(
    filepath=str(out / 'SM_FX_Peppermint.fbx'),
    use_selection=True,
    object_types={'MESH'},
    add_leaf_bones=False,
    bake_anim=False,
    path_mode='COPY',
    embed_textures=True,
    mesh_smooth_type='FACE',
)
