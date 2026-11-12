"""Unreal Python commandlet: author /Game/World/L_Sugarborough.

Run only during TERRAIN's exclusive engine window. Imports only our FBXs,
creates only /Game/World assets, and bakes scatter into persistent map actors.
PCG graphs preserve the exact approved placement palette for later editing.
"""
import json
import hashlib
from collections import defaultdict
from pathlib import Path
import unreal as u

ROOT = Path(u.Paths.convert_relative_path_to_full(u.Paths.project_dir())) / "ImportSource" / "Terrain"
DATA = json.loads((ROOT / "world_manifest.json").read_text(encoding="utf-8"))
BASE = "/Game/World"
EAL = u.EditorAssetLibrary
TOOLS = u.AssetToolsHelpers.get_asset_tools()
ACTORS = u.get_default_object(u.EditorActorSubsystem)
SM_EDITOR = u.get_default_object(u.StaticMeshEditorSubsystem)


def asset(name, folder, cls, factory):
    path = f"{BASE}/{folder}/{name}"
    if EAL.does_asset_exist(path):
        return u.load_asset(path)
    result = TOOLS.create_asset(name, f"{BASE}/{folder}", cls, factory)
    if not result:
        raise RuntimeError("Could not create " + path)
    return result


def materials():
    result = {}
    for name, rgb in DATA["palette"].items():
        mat = asset("M_" + name, "Materials", u.Material, u.MaterialFactoryNew())
        if not EAL.get_metadata_tag(mat, "Terrain.Authored"):
            color = u.MaterialEditingLibrary.create_material_expression(mat, u.MaterialExpressionConstant3Vector)
            color.set_editor_property("constant", u.LinearColor(*rgb, 1))
            u.MaterialEditingLibrary.connect_material_property(color, "", u.MaterialProperty.MP_BASE_COLOR)
            rough = u.MaterialEditingLibrary.create_material_expression(mat, u.MaterialExpressionConstant)
            rough.set_editor_property("r", .85)
            u.MaterialEditingLibrary.connect_material_property(rough, "", u.MaterialProperty.MP_ROUGHNESS)
            u.MaterialEditingLibrary.recompile_material(mat)
            EAL.set_metadata_tag(mat, "Terrain.Authored", "Original Sugarborough palette")
            EAL.save_loaded_asset(mat)
        result[name] = mat
    texture_path = BASE + "/Textures/T_SugarboroughGround"
    if not EAL.does_asset_exist(texture_path):
        task = u.AssetImportTask()
        task.set_editor_properties(dict(filename=str(ROOT / "Sources" / "T_SugarboroughGround.png"), destination_path=BASE + "/Textures", automated=True, save=True))
        TOOLS.import_asset_tasks([task])
    texture = u.load_asset(texture_path)
    texture.set_editor_property("srgb", False)
    texture.set_editor_property("max_texture_size", 1024)
    EAL.save_loaded_asset(texture)
    mat = asset("M_Terrain", "Materials", u.Material, u.MaterialFactoryNew())
    if not EAL.get_metadata_tag(mat, "Terrain.Authored"):
        sample = u.MaterialEditingLibrary.create_material_expression(mat, u.MaterialExpressionTextureSample)
        sample.set_editor_property("texture", texture)
        sample.set_editor_property("sampler_type", u.MaterialSamplerType.SAMPLERTYPE_LINEAR_COLOR)
        u.MaterialEditingLibrary.connect_material_property(sample, "RGB", u.MaterialProperty.MP_BASE_COLOR)
        rough = u.MaterialEditingLibrary.create_material_expression(mat, u.MaterialExpressionConstant)
        rough.set_editor_property("r", .9)
        u.MaterialEditingLibrary.connect_material_property(rough, "", u.MaterialProperty.MP_ROUGHNESS)
        u.MaterialEditingLibrary.recompile_material(mat)
        EAL.set_metadata_tag(mat, "Terrain.Authored", "Original 1k ground atlas")
        EAL.save_loaded_asset(mat)
    result["Terrain"] = mat
    return result


def import_meshes(mats):
    result = {}
    for record in DATA["meshes"]:
        name = record["name"]
        path = f"{BASE}/Meshes/{name}"
        source_hash = hashlib.sha256((ROOT / "FBX" / (name + ".fbx")).read_bytes()).hexdigest()
        if EAL.does_asset_exist(path) and EAL.get_metadata_tag(u.load_asset(path), "Terrain.FBXHash") == source_hash:
            mesh = u.load_asset(path)
        else:
            task = u.AssetImportTask()
            task.set_editor_properties(dict(filename=str(ROOT / "FBX" / (name + ".fbx")), destination_path=BASE + "/Meshes",
                destination_name=name, automated=True, save=True, replace_existing=True))
            options = u.FbxImportUI()
            options.set_editor_properties(dict(import_mesh=True, import_as_skeletal=False, import_materials=False, import_textures=False))
            options.static_mesh_import_data.set_editor_properties(dict(combine_meshes=True, auto_generate_collision=False, import_uniform_scale=1.0))
            task.set_editor_property("options", options)
            TOOLS.import_asset_tasks([task])
            found = [u.load_asset(p) for p in task.get_editor_property("imported_object_paths")]
            found = [a for a in found if isinstance(a, u.StaticMesh)]
            if len(found) != 1:
                raise RuntimeError(f"Expected one mesh: {name}, got {found}")
            mesh = found[0]
        # Resolve by imported slot name; FBX may reorder material slots.
        for i, slot in enumerate(mesh.get_editor_property("static_materials")):
            slot_name = str(slot.get_editor_property("imported_material_slot_name"))
            if slot_name not in mats:
                slot_name = record["materials"][i]
            mesh.set_material(i, mats[slot_name])
        body = mesh.get_editor_property("body_setup")
        body.set_editor_property("collision_trace_flag", u.CollisionTraceFlag.CTF_USE_COMPLEX_AS_SIMPLE)
        body.set_editor_property("double_sided_geometry", True)
        for section in range(mesh.get_num_sections(0)):
            SM_EDITOR.enable_section_collision(mesh, True, 0, section)
        # This rebuild must happen AFTER setting complexity: changing BodySetup's
        # raw property alone leaves the old cooked collision geometry in place.
        SM_EDITOR.remove_collisions(mesh)
        EAL.set_metadata_tag(mesh, "Terrain.Source", "ImportSource/Terrain/build_sources.py")
        EAL.set_metadata_tag(mesh, "Terrain.FBXHash", source_hash)
        EAL.save_loaded_asset(mesh)
        result[name] = mesh
    # Fail on FBX unit mistakes before saving a wrong-sized map.
    bounds = result["SM_GingerbreadCottage"].get_bounds()
    if not 450 < bounds.box_extent.x * 2 < 650:
        raise RuntimeError("FBX unit conversion wrong: cottage extent " + str(bounds.box_extent))
    return result


def spawn(cls, location=(0, 0, 0), yaw=0, pitch=0):
    result = ACTORS.spawn_actor_from_class(cls, u.Vector(*location), u.Rotator(pitch=pitch, yaw=yaw))
    if not result:
        raise RuntimeError("Could not spawn " + str(cls))
    return result


def mesh_actor(mesh, location, scale=(1, 1, 1), yaw=0, blocking=False, label=None):
    actor = spawn(u.StaticMeshActor, location, yaw)
    actor.set_actor_label(label or mesh.get_name())
    actor.set_folder_path("Sugarborough/" + ("Terrain" if mesh.get_name().startswith("SM_Terrain") else "Dressing"))
    actor.static_mesh_component.set_static_mesh(mesh)
    actor.set_actor_scale3d(u.Vector(*scale))
    comp = actor.static_mesh_component
    comp.set_collision_profile_name("BlockAll" if blocking else "NoCollision")
    comp.set_collision_enabled(u.CollisionEnabled.QUERY_AND_PHYSICS if blocking else u.CollisionEnabled.NO_COLLISION)
    return actor


def pcg_graphs(meshes):
    # Exact world-space points have already passed exclusion and slope checks.
    # Baked actors are authoritative in the delivered map; no runtime generation.
    groups = defaultdict(list)
    for p in DATA["placements"]:
        if p["category"] == "Scatter":
            groups[p["biome"], p["mesh"]].append(p)
    graphs = []
    for b in ("Meadow", "Molars", "Orchard"):
        name = "PCG_" + b
        full = f"{BASE}/PCG/{name}"
        if EAL.does_asset_exist(full):
            graphs.append(full)
            continue
        graph = asset(name, "PCG", u.PCGGraph, u.PCGGraphFactory())
        for row, ((biome, mesh_name), points) in enumerate((item for item in sorted(groups.items()) if item[0][0] == b)):
            node, settings = graph.add_node_of_type(u.PCGCreatePointsSettings)
            node.set_node_position(0, row * 240)
            node.set_editor_property("node_title", mesh_name + " clear points")
            pcg_points = []
            for index, p in enumerate(points):
                point = u.PCGPoint()
                transform = u.Transform(location=u.Vector(*p["location"]), rotation=u.Rotator(yaw=p["yaw"]), scale=u.Vector(*p["scale"]))
                point.set_editor_property("transform", transform)
                point.set_editor_property("seed", DATA["seed"] + index)
                pcg_points.append(point)
            settings.set_editor_property("points_to_create", pcg_points)
            spawner, spawn_settings = graph.add_node_of_type(u.PCGStaticMeshSpawnerSettings)
            spawner.set_node_position(400, row * 240)
            spawn_settings.set_mesh_selector_type(u.PCGMeshSelectorWeighted)
            selector = spawn_settings.get_editor_property("mesh_selector_parameters")
            entry = u.PCGMeshSelectorWeightedEntry()
            descriptor = entry.get_editor_property("descriptor")
            descriptor.set_editor_property("static_mesh", meshes[mesh_name])
            body = descriptor.get_editor_property("body_instance")
            body.set_editor_property("collision_profile_name", "NoCollision")
            descriptor.set_editor_property("body_instance", body)
            entry.set_editor_property("descriptor", descriptor)
            selector.set_editor_property("mesh_entries", [entry])
            graph.add_edge(node, "Out", spawner, "In")
            graph.add_edge(spawner, "Out", graph.get_output_node(), "Out")
        EAL.set_metadata_tag(graph, "Terrain.Baked", "Exact points baked into L_Sugarborough; regenerate in a separate map to avoid duplicates")
        EAL.save_loaded_asset(graph)
        graphs.append(full)
    return graphs


def main():
    mats = materials()
    meshes = import_meshes(mats)
    graphs = pcg_graphs(meshes)
    world = u.EditorLoadingAndSavingUtils.new_blank_map(False)
    world.get_world_settings().set_editor_property("force_no_precomputed_lighting", True)
    for name, mesh in meshes.items():
        if name.startswith("SM_Terrain"):
            mesh_actor(mesh, (0, 0, 0), blocking=True)
    for i, p in enumerate(DATA["placements"]):
        mesh_actor(meshes[p["mesh"]], p["location"], p["scale"], p["yaw"], p["blocking"], f"{p['category']}_{i:03}_{p['mesh']}")
    # Hidden continuous walls back the visible mountain edge, including corners.
    cube = u.load_asset("/Engine/BasicShapes/Cube")
    for x, y, sx, sy in [(12050, 0, 1, 244), (-12050, 0, 1, 244), (0, 12050, 244, 1), (0, -12050, 244, 1)]:
        wall = mesh_actor(cube, (x, y, 3500), (sx, sy, 100), blocking=True, label="Mountain perimeter collision")
        wall.set_actor_hidden_in_game(True)
        wall.set_folder_path("Sugarborough/Boundary")
    sun = spawn(u.DirectionalLight, (0, 0, 5000), -35, -50)
    sun.light_component.set_mobility(u.ComponentMobility.MOVABLE)
    sun.light_component.set_editor_property("intensity", 5.0)
    sky = spawn(u.SkyLight, (0, 0, 5000))
    sky.light_component.set_mobility(u.ComponentMobility.MOVABLE)
    sky.light_component.set_editor_property("intensity", 1.3)
    sky.light_component.set_editor_property("real_time_capture", True)
    spawn(u.SkyAtmosphere)
    fog = spawn(u.ExponentialHeightFog, (0, 0, -1000))
    fog.component.set_editor_property("fog_density", .008)
    fog.component.set_editor_property("fog_height_falloff", .15)
    start = spawn(u.PlayerStart, (0, 0, 120))
    start.set_actor_label("Village arrival")
    # Fixed cameras for review are inactive during play.
    for label, eye, yaw, pitch in [("Terrain overview", (-18500, -18500, 22000), 45, -40), ("Village review", (-3600, -3200, 2300), 42, -27)]:
        cam = spawn(u.CameraActor, eye, yaw, pitch)
        cam.set_actor_label(label)
        cam.camera_component.set_field_of_view(65)
    path = BASE + "/L_Sugarborough"
    if not u.EditorLoadingAndSavingUtils.save_map(world, path):
        raise RuntimeError("Could not save " + path)
    report = dict(map=path, imported_meshes=len(meshes), baked_placements=len(DATA["placements"]), pcg_graphs=graphs,
        source_validation=DATA["validation"], packaged_validation="pending GAME O1 merge and exclusive packaging window")
    (ROOT / "level_report.json").write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    u.log("TERRAIN MAP AUTHORING PASS " + json.dumps(report))


if __name__ == "__main__":
    main()
