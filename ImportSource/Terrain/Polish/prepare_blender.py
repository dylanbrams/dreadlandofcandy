"""Offline candy environment variants. Never modifies generator originals."""
import hashlib
import json
import math
from pathlib import Path
import bpy
import bmesh
from mathutils import Matrix, Vector

HERE = Path(__file__).resolve().parent
ROOT = HERE.parent
for folder in ('FBX', 'Textures', 'Blend'):
    (HERE / folder).mkdir(parents=True, exist_ok=True)
records = []


def reset():
    bpy.ops.wm.read_factory_settings(use_empty=True)
    bpy.context.scene.unit_settings.system = 'METRIC'
    bpy.context.scene.unit_settings.scale_length = .01


def join_import(path, height):
    bpy.ops.import_scene.gltf(filepath=str(path))
    objects = [o for o in bpy.context.scene.objects if o.type == 'MESH']
    bpy.ops.object.select_all(action='DESELECT')
    for obj in objects:
        obj.data.transform(obj.matrix_world)
        obj.matrix_world.identity()
        obj.select_set(True)
    bpy.context.view_layer.objects.active = objects[0]
    bpy.ops.object.join()
    obj = bpy.context.object
    lo = Vector(tuple(min(v.co[i] for v in obj.data.vertices) for i in range(3)))
    hi = Vector(tuple(max(v.co[i] for v in obj.data.vertices) for i in range(3)))
    origin = Vector(((lo.x + hi.x) / 2, (lo.y + hi.y) / 2, lo.z))
    factor = height / (hi.z - lo.z)
    for v in obj.data.vertices:
        v.co = (v.co - origin) * factor
    # Weld glTF attribute seams before subdivision, keeping UV discontinuities.
    bm = bmesh.new()
    bm.from_mesh(obj.data)
    bmesh.ops.remove_doubles(bm, verts=list(bm.verts), dist=.001)
    bm.to_mesh(obj.data)
    bm.free()
    return obj


def export(obj, name, source, **extra):
    obj.name = name
    obj.data.calc_loop_triangles()
    triangles = len(obj.data.loop_triangles)
    assert triangles < 25000, (name, triangles)
    for poly in obj.data.polygons:
        poly.use_smooth = True
    bpy.ops.wm.save_as_mainfile(filepath=str(HERE / 'Blend' / (name + '.blend')))
    actual = obj.data
    mirrored = actual.copy()
    mirrored.transform(Matrix.Diagonal((1, -1, 1, 1)))
    mirrored.flip_normals()
    obj.data = mirrored
    bpy.ops.export_scene.fbx(filepath=str(HERE / 'FBX' / (name + '.fbx')),
        use_selection=True, object_types={'MESH'}, apply_unit_scale=True,
        axis_forward='-Y', axis_up='Z', bake_anim=False, mesh_smooth_type='OFF')
    obj.data = actual
    bpy.data.meshes.remove(mirrored)
    bounds = [[min(v.co[i] for v in actual.vertices), max(v.co[i] for v in actual.vertices)] for i in range(3)]
    records.append(dict(name=name, triangles=triangles, bounds_cm=bounds,
        source=str(source.relative_to(ROOT.parent.parent)), source_sha256=hashlib.sha256(source.read_bytes()).hexdigest(), **extra))
    print('POLISH ASSET', name, triangles, flush=True)


for species, height in [('PeppermintBirch', 620), ('BubblegumBlossom', 580), ('CandyCaneFir', 720)]:
    source = ROOT / 'Foliage/Clean' / ('SM_Rodin_' + species + '.glb')
    for variant in range(3):
        reset()
        obj = join_import(source, height)
        # One subdivision removes facets without multiplying the forest budget excessively.
        sub = obj.modifiers.new('Rounded candy canopy', 'SUBSURF')
        sub.levels = 1
        bpy.ops.object.modifier_apply(modifier=sub.name)
        for v in obj.data.vertices:
            z = max(0, v.co.z / height)
            angle = (.12, -.18, .26)[variant] * z
            x, y = v.co.x, v.co.y
            v.co.x = (x * math.cos(angle) - y * math.sin(angle)) * (.83, 1.03, .91)[variant] + (30, -43, 62)[variant] * z * z
            v.co.y = (x * math.sin(angle) + y * math.cos(angle)) * (1.05, .84, .96)[variant] + 17 * math.sin(z * 5 + variant) * z
            v.co.z *= (.79, 1.04, 1.24)[variant]
        export(obj, 'SM_' + species + '_Warp' + str(variant + 1), source, category='tree', species=species, variant=variant)

# Smooth the original four-lobed bush and break its regular silhouette.
reset()
source = ROOT / 'Dense/Sources/SM_CandyflossThicket.obj'
vertices, faces, indices, materials = [], [], [], []
for line in source.read_text().splitlines():
    f = line.split()
    if not f:
        continue
    if f[0] == 'v':
        vertices.append(tuple(map(float, f[1:4])))
    elif f[0] == 'usemtl':
        if f[1] not in materials:
            materials.append(f[1])
        index = materials.index(f[1])
    elif f[0] == 'f':
        faces.append(tuple(int(i.split('/')[0])-1 for i in f[1:]))
        indices.append(index)
mesh = bpy.data.meshes.new('Floss')
mesh.from_pydata(vertices, [], faces)
for name in materials:
    mesh.materials.append(bpy.data.materials.new(name))
for p, i in zip(mesh.polygons, indices):
    p.material_index = i
obj = bpy.data.objects.new('Floss', mesh)
bpy.context.collection.objects.link(obj)
obj.select_set(True)
bpy.context.view_layer.objects.active = obj
sub = obj.modifiers.new('Soft spun sugar', 'SUBSURF')
sub.levels = 2
bpy.ops.object.modifier_apply(modifier=sub.name)
for v in obj.data.vertices:
    p = v.co
    ripple = 1 + .035 * math.sin(p.x * .11 + p.z * .08) * math.cos(p.y * .09)
    p.x *= ripple
    p.y *= ripple
export(obj, 'SM_SpunSugarThicket', source, category='bush', materials=materials)

# The full original Fal house is the dense striped house, rather than the flat Rodin facade.
source = ROOT.parent / 'Fal/SM_HouseFront_Fal.glb'
for variant in range(3):
    reset()
    obj = join_import(source, 510)
    # Source door faces Blender -Y; terrain cottages face local -X toward
    # the village square. Bake this into geometry, keeping actor transforms.
    obj.data.transform(Matrix.Rotation(-math.pi/2, 4, 'Z'))
    # Retain roughly an order of magnitude more detail than the old cottages.
    obj.data.calc_loop_triangles()
    dec = obj.modifiers.new('Environment house budget', 'DECIMATE')
    dec.ratio = min(1, 18000 / len(obj.data.loop_triangles))
    bpy.ops.object.modifier_apply(modifier=dec.name)
    # Keep the house within the old ~560 cm footprint. Separate roof/body warps
    # create squat, tall and gently crooked silhouettes without detached trim.
    for v in obj.data.vertices:
        z = v.co.z / 510
        v.co.x *= (1, .86, .94)[variant]
        v.co.y *= (.93, .90, 1.0)[variant]
        v.co.z *= (.92, 1.16, 1.02)[variant]
        v.co.x += (0, 15, -24)[variant] * max(0, z - .4) ** 2
    texture_names = []
    for i, slot in enumerate(obj.material_slots):
        images = [n.image for n in slot.material.node_tree.nodes if n.type == 'TEX_IMAGE' and n.image]
        # Only the base-colour image is used by the environment shader.
        bsdf = next(n for n in slot.material.node_tree.nodes if n.type == 'BSDF_PRINCIPLED')
        link = next(iter(bsdf.inputs['Base Color'].links), None)
        image = link.from_node.image if link and link.from_node.type == 'TEX_IMAGE' else images[0]
        tex_name = 'T_CandyHouse_' + str(i)
        if variant == 0:
            if max(image.size) > 2048:
                image.scale(2048, 2048)
            image.filepath_raw = str(HERE / 'Textures' / (tex_name + '.png'))
            image.file_format = 'PNG'
            image.save()
        texture_names.append(tex_name)
    export(obj, 'SM_CandyHouse_' + str(variant+1), source, category='house', variant=variant, textures=texture_names)

(HERE / 'assets.json').write_text(json.dumps(dict(assets=records, paid_generation=False), indent=2)+'\n')
print('POLISH BLENDER PASS', len(records))
