"""Prepare generated foliage for Unreal; preserve every raw GLB unchanged.

Blender 4.5 background script. Writes normalized, budgeted GLB/FBX, 1k textures
and individual previews. No Unreal process or source/world edits required.
"""
import hashlib
import json
import math
from pathlib import Path
import struct
import sys
import bpy
import bmesh
from mathutils import Matrix, Vector

HERE = Path(__file__).resolve().parent
JOBS = json.loads((HERE / "jobs.json").read_text(encoding="utf-8"))["jobs"]


def triangle_count(mesh):
    mesh.calc_loop_triangles()
    return len(mesh.loop_triangles)


def glb_triangles(path):
    raw = path.read_bytes()
    length = struct.unpack_from("<I", raw, 12)[0]
    doc = json.loads(raw[20:20 + length])
    return sum(doc["accessors"][p.get("indices", p["attributes"]["POSITION"])]["count"] // 3
        for node in doc.get("nodes", []) if "mesh" in node
        for p in doc["meshes"][node["mesh"]]["primitives"])


def source_image(socket, visited=None):
    visited = set() if visited is None else visited
    for link in socket.links:
        node = link.from_node
        if node in visited:
            continue
        visited.add(node)
        if node.type == "TEX_IMAGE":
            return node.image
        for pin in node.inputs:
            found = source_image(pin, visited)
            if found:
                return found
    return None


def prepare(job):
    name = job["name"]
    raw = HERE / "Raw" / (name + ".glb")
    digest = hashlib.sha256(raw.read_bytes()).hexdigest()
    bpy.ops.wm.read_factory_settings(use_empty=True)
    scene = bpy.context.scene
    scene.unit_settings.system = "METRIC"
    scene.unit_settings.scale_length = .01
    bpy.ops.import_scene.gltf(filepath=str(raw))
    objects = [o for o in scene.objects if o.type == "MESH"]
    before = sum(triangle_count(o.data) for o in objects)
    for obj in objects:
        obj.data = obj.data.copy()
        obj.data.transform(obj.matrix_world)
        obj.matrix_world.identity()
        bm = bmesh.new()
        bm.from_mesh(obj.data)
        epsilon = max(max(obj.dimensions), .01) * 1e-7
        bmesh.ops.remove_doubles(bm, verts=list(bm.verts), dist=epsilon)
        bmesh.ops.dissolve_degenerate(bm, edges=list(bm.edges), dist=epsilon)
        bm.to_mesh(obj.data)
        bm.free()
        obj.data.update()
    bpy.ops.object.select_all(action="DESELECT")
    for obj in objects:
        obj.select_set(True)
    bpy.context.view_layer.objects.active = objects[0]
    bpy.ops.object.join()
    obj = bpy.context.object
    obj.name = name
    vertices = [v.co for v in obj.data.vertices]
    minimum = Vector(tuple(min(v[i] for v in vertices) for i in range(3)))
    maximum = Vector(tuple(max(v[i] for v in vertices) for i in range(3)))
    factor = job["height_cm"] / (maximum.z - minimum.z)
    origin = Vector(((minimum.x + maximum.x) / 2, (minimum.y + maximum.y) / 2, minimum.z))
    for vertex in obj.data.vertices:
        vertex.co = (vertex.co - origin) * factor
        assert all(math.isfinite(c) for c in vertex.co), "Non-finite vertex"
    obj.data.update()
    bpy.context.view_layer.update()
    for attempt in range(3):
        count = triangle_count(obj.data)
        if count <= 1500:
            break
        decimate = obj.modifiers.new("Foliage triangle budget", "DECIMATE")
        decimate.ratio = 1490 / count
        decimate.use_collapse_triangulate = True
        bpy.ops.object.modifier_apply(modifier=decimate.name)
    count = triangle_count(obj.data)
    bpy.context.view_layer.update()
    assert 0 < count <= 1500, count
    textures = []
    material_records = []
    for index, slot in enumerate(obj.material_slots):
        mat = slot.material
        if not mat or not mat.use_nodes:
            raise RuntimeError("Missing generated material")
        bsdf = next(n for n in mat.node_tree.nodes if n.type == "BSDF_PRINCIPLED")
        albedo = source_image(bsdf.inputs["Base Color"])
        if albedo is None:
            raise RuntimeError("No base-colour texture")
        tex_name = name.replace("SM_", "T_", 1) + f"_BaseColor_{index}"
        if max(albedo.size) > 1024:
            scale = 1024 / max(albedo.size)
            albedo.scale(round(albedo.size[0] * scale), round(albedo.size[1] * scale))
        albedo.filepath_raw = str(HERE / "Textures" / (tex_name + ".png"))
        albedo.file_format = "PNG"
        albedo.save()
        # Matte foliage uses just the original base colour. Retain no unused
        # normal/roughness/metallic maps in the cleaned GLB.
        for pin_name in ("Metallic", "Roughness", "Normal"):
            for link in list(bsdf.inputs[pin_name].links):
                mat.node_tree.links.remove(link)
        bsdf.inputs["Metallic"].default_value = 0
        bsdf.inputs["Roughness"].default_value = .85
        mat.surface_render_method = "DITHERED"
        for node in list(mat.node_tree.nodes):
            if node.type == "TEX_IMAGE" and node.image != albedo:
                mat.node_tree.nodes.remove(node)
        textures.append(dict(file=tex_name + ".png", width=albedo.size[0], height=albedo.size[1]))
        material_records.append(dict(slot=index, name=mat.name, base_color=tex_name))
    output = HERE / "Clean" / (name + ".glb")
    bpy.ops.export_scene.gltf(filepath=str(output), export_format="GLB", use_selection=True,
        export_materials="EXPORT", export_texcoords=True, export_normals=True)
    actual = glb_triangles(output)
    assert actual <= 1500, actual
    export_mesh = obj.data.copy()
    export_mesh.transform(Matrix.Diagonal((1, -1, 1, 1)))
    export_mesh.flip_normals()
    real_mesh = obj.data
    obj.data = export_mesh
    bpy.ops.export_scene.fbx(filepath=str(HERE / "FBX" / (name + ".fbx")), use_selection=True,
        object_types={"MESH"}, apply_unit_scale=True, axis_forward="-Y", axis_up="Z", bake_anim=False,
        use_mesh_modifiers=True, mesh_smooth_type="FACE", add_leaf_bones=False)
    obj.data = real_mesh
    bpy.data.meshes.remove(export_mesh)
    assert hashlib.sha256(raw.read_bytes()).hexdigest() == digest, "Raw original changed"
    result = dict(name=name, category=job["category"], biomes=job["biomes"], height_cm=job["height_cm"],
        source_triangles=before, triangles=actual, raw_sha256=digest,
        clean_sha256=hashlib.sha256(output.read_bytes()).hexdigest(),
        width_cm=obj.dimensions.x, depth_cm=obj.dimensions.y,
        materials=material_records, textures=textures, collision="NoCollision")
    if "--no-render" not in sys.argv:
        render_preview(obj, name, scene)
    return result


def render_preview(obj, name, scene):
    world = bpy.data.worlds.new("Neutral candy review")
    world.use_nodes = True
    world.node_tree.nodes["Background"].inputs[0].default_value = (.48, .60, .69, 1)
    world.node_tree.nodes["Background"].inputs[1].default_value = .65
    scene.world = world
    light_data = bpy.data.lights.new("Soft daylight", "SUN")
    light_data.energy = 2.5
    light_data.angle = .25
    light = bpy.data.objects.new("Soft daylight", light_data)
    scene.collection.objects.link(light)
    light.rotation_euler = (.4, -.6, -.4)
    cam_data = bpy.data.cameras.new("Review")
    cam = bpy.data.objects.new("Review", cam_data)
    scene.collection.objects.link(cam)
    height = obj.dimensions.z
    radius = max(obj.dimensions.x, obj.dimensions.y, height)
    target = Vector((0, 0, height * .48))
    cam.location = target + Vector((radius * 1.55, -radius * 2.3, radius * .85))
    cam.rotation_euler = (target - cam.location).to_track_quat("-Z", "Y").to_euler()
    cam_data.type = "ORTHO"
    cam_data.ortho_scale = radius * 1.4
    cam_data.clip_end = 100000
    scene.camera = cam
    scene.render.engine = "CYCLES"
    scene.cycles.samples = 16
    scene.cycles.use_denoising = True
    scene.render.resolution_x = 640
    scene.render.resolution_y = 640
    scene.render.resolution_percentage = 100
    scene.view_settings.view_transform = "Standard"
    scene.render.image_settings.file_format = "PNG"
    scene.render.filepath = str(HERE / "Review" / (name + ".png"))
    bpy.ops.render.render(write_still=True)


def main():
    for folder in ("Clean", "FBX", "Textures", "Review"):
        (HERE / folder).mkdir(parents=True, exist_ok=True)
    results = []
    for job in JOBS:
        if not (HERE / "Raw" / (job["name"] + ".glb")).exists():
            continue
        result = prepare(job)
        results.append(result)
        (HERE / "prepared_manifest.json").write_text(json.dumps({"assets": results}, indent=2) + "\n", encoding="utf-8")
        print("FOLIAGE PREPARED " + job["name"] + " triangles=" + str(result["triangles"]), flush=True)
    print(f"FOLIAGE PREPARATION PASS {len(results)}/{len(JOBS)}")


if __name__ == "__main__":
    main()
