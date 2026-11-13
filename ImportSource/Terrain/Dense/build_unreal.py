"""Import, run and bake the layered candy PCG graph in a full editor session."""
import json
import hashlib
from pathlib import Path
import time
import traceback
import unreal as u

ROOT = Path(u.Paths.convert_relative_path_to_full(u.Paths.project_dir())) / "ImportSource/Terrain"
HERE = ROOT / "Dense"
DATA = json.loads((HERE / "ecology.json").read_text())
FOLIAGE = json.loads((ROOT / "Foliage/prepared_manifest.json").read_text())["assets"]
BASE = "/Game/World/Dense"
EAL = u.EditorAssetLibrary
TOOLS = u.AssetToolsHelpers.get_asset_tools()
ACTORS = u.get_default_object(u.EditorActorSubsystem)


def asset(name, folder, cls, factory):
    path = f"{BASE}/{folder}/{name}"
    return u.load_asset(path) if EAL.does_asset_exist(path) else TOOLS.create_asset(name, f"{BASE}/{folder}", cls, factory)


def expr(mat, cls, **properties):
    node = u.MaterialEditingLibrary.create_material_expression(mat, cls)
    node.set_editor_properties(properties)
    return node


def connect(a, output, b, pin):
    if not u.MaterialEditingLibrary.connect_material_expressions(a, output, b, pin):
        raise RuntimeError("Material connection failed: " + pin)


def scalar(mat, value):
    return expr(mat, u.MaterialExpressionConstant, r=value)


def shader(name, color=None, texture=None, grass=False, snow=False):
    mat = asset(name, "Materials", u.Material, u.MaterialFactoryNew())
    mat.set_editor_property("used_with_instanced_static_meshes", True)
    if EAL.get_metadata_tag(mat, "Dense.Authored") == "v2":
        EAL.save_loaded_asset(mat)
        return mat
    u.MaterialEditingLibrary.delete_all_material_expressions(mat)
    mat.set_editor_property("two_sided", True)
    base = expr(mat, u.MaterialExpressionTextureSample, texture=texture) if texture else expr(mat, u.MaterialExpressionConstant3Vector, constant=u.LinearColor(*color, 1))
    if texture and not texture.get_editor_property("srgb"):
        base.set_editor_property("sampler_type", u.MaterialSamplerType.SAMPLERTYPE_LINEAR_COLOR)
    rough = scalar(mat, .9)
    u.MaterialEditingLibrary.connect_material_property(rough, "", u.MaterialProperty.MP_ROUGHNESS)
    if grass:
        vertex = expr(mat, u.MaterialExpressionVertexColor)
        mul = expr(mat, u.MaterialExpressionMultiply)
        connect(base, "RGB" if texture else "", mul, "A")
        connect(vertex, "", mul, "B")
        base = mul
        clock = expr(mat, u.MaterialExpressionTime)
        speed = expr(mat, u.MaterialExpressionMultiply)
        connect(clock, "", speed, "A")
        connect(scalar(mat, .23), "", speed, "B")
        sine = expr(mat, u.MaterialExpressionSine)
        connect(speed, "", sine, "")
        weight = expr(mat, u.MaterialExpressionMultiply)
        connect(sine, "", weight, "A")
        connect(vertex, "A", weight, "B")
        strength = expr(mat, u.MaterialExpressionMultiply)
        connect(weight, "", strength, "A")
        connect(scalar(mat, 7), "", strength, "B")
        xyz = expr(mat, u.MaterialExpressionMultiply)
        connect(strength, "", xyz, "A")
        connect(expr(mat, u.MaterialExpressionConstant3Vector, constant=u.LinearColor(1, .35, 0, 1)), "", xyz, "B")
        u.MaterialEditingLibrary.connect_material_property(xyz, "", u.MaterialProperty.MP_WORLD_POSITION_OFFSET)
        fill = expr(mat, u.MaterialExpressionMultiply)
        connect(base, "", fill, "A")
        connect(scalar(mat, .12), "", fill, "B")
        u.MaterialEditingLibrary.connect_material_property(fill, "", u.MaterialProperty.MP_EMISSIVE_COLOR)
    if snow:
        mat.set_editor_property("shading_model", u.MaterialShadingModel.MSM_UNLIT)
        mat.set_editor_property("blend_mode", u.BlendMode.BLEND_TRANSLUCENT)
        clock = expr(mat, u.MaterialExpressionTime)
        speed = expr(mat, u.MaterialExpressionMultiply)
        connect(clock, "", speed, "A")
        connect(scalar(mat, .085), "", speed, "B")
        add = expr(mat, u.MaterialExpressionAdd)
        connect(speed, "", add, "A")
        connect(expr(mat, u.MaterialExpressionPerInstanceRandom), "", add, "B")
        frac = expr(mat, u.MaterialExpressionFrac)
        connect(add, "", frac, "")
        fall = expr(mat, u.MaterialExpressionMultiply)
        connect(frac, "", fall, "A")
        connect(expr(mat, u.MaterialExpressionConstant3Vector, constant=u.LinearColor(70, 35, -1700, 1)), "", fall, "B")
        u.MaterialEditingLibrary.connect_material_property(fall, "", u.MaterialProperty.MP_WORLD_POSITION_OFFSET)
        u.MaterialEditingLibrary.connect_material_property(base, "", u.MaterialProperty.MP_EMISSIVE_COLOR)
        u.MaterialEditingLibrary.connect_material_property(scalar(mat, .8), "", u.MaterialProperty.MP_OPACITY)
    else:
        u.MaterialEditingLibrary.connect_material_property(base, "", u.MaterialProperty.MP_BASE_COLOR)
    u.MaterialEditingLibrary.recompile_material(mat)
    EAL.set_metadata_tag(mat, "Dense.Authored", "v2")
    EAL.save_loaded_asset(mat)
    return mat


def import_file(path, folder, texture=False):
    wanted = f"{BASE}/{folder}/{path.stem}"
    source_hash = hashlib.sha256(path.read_bytes()).hexdigest()
    if EAL.does_asset_exist(wanted) and EAL.get_metadata_tag(u.load_asset(wanted), "Dense.SourceHash") == source_hash:
        return u.load_asset(wanted)
    task = u.AssetImportTask()
    task.set_editor_properties(dict(filename=str(path), destination_path=f"{BASE}/{folder}", destination_name=path.stem, automated=True, save=True, replace_existing=True))
    if not texture:
        options = u.FbxImportUI()
        options.set_editor_properties(dict(import_mesh=True, import_as_skeletal=False, import_materials=False, import_textures=False))
        options.static_mesh_import_data.set_editor_properties(dict(combine_meshes=True, auto_generate_collision=False, import_uniform_scale=1.0, vertex_color_import_option=u.VertexColorImportOption.REPLACE))
        task.set_editor_property("options", options)
    TOOLS.import_asset_tasks([task])
    found = u.load_asset(wanted)
    if not found:
        raise RuntimeError("Import failed " + str(path))
    EAL.set_metadata_tag(found, "Dense.SourceHash", source_hash)
    EAL.save_loaded_asset(found)
    return found


def import_assets():
    result = {}
    for record in FOLIAGE:
        if "LicoriceWillow" in record["name"]:
            continue
        mesh = import_file(ROOT / "Foliage/FBX" / (record["name"] + ".fbx"), "Meshes")
        for slot in record["materials"]:
            texture = import_file(ROOT / "Foliage/Textures" / (slot["base_color"] + ".png"), "Textures", True)
            texture.set_editor_property("max_texture_size", 1024)
            EAL.save_loaded_asset(texture)
            mat = shader("M_" + record["name"][3:] + "_" + str(slot["slot"]), texture=texture)
            mesh.set_material(slot["slot"], mat)
        EAL.save_loaded_asset(mesh)
        result[record["name"]] = mesh
    colors = {"Mint": (.16, .46, .25), "Pink": (.65, .16, .36), "White": (.66, .79, .86), "Cream": (.92, .64, .77)}
    for record in DATA["meshes"]:
        mesh = import_file(HERE / "FBX" / (record["name"] + ".fbx"), "Meshes")
        for index, name in enumerate(record["materials"]):
            mat = shader("M_Dense_" + record["name"][3:] + "_" + name, color=colors[name], grass="Grass" in record["name"], snow="Snowflake" in record["name"])
            mesh.set_material(index, mat)
        if "Snowflake" in record["name"]:
            mesh.set_editor_property("negative_bounds_extension", u.Vector(80, 80, 1800))
            mesh.set_editor_property("positive_bounds_extension", u.Vector(80, 80, 1800))
        EAL.save_loaded_asset(mesh)
        result[record["name"]] = mesh
    texture = import_file(HERE / "Sources/T_CandySnowGround.png", "Textures", True)
    texture.set_editor_property("srgb", False)
    EAL.save_loaded_asset(texture)
    return result, shader("M_CandySnowTerrain", texture=texture)


def graph(meshes):
    graph = asset("PCG_CandyEcology", "PCG", u.PCGGraph, u.PCGGraphFactory())
    if graph.get_editor_property("nodes"):
        for node in list(graph.get_editor_property("nodes")):
            graph.remove_node(node)
    for index, group in enumerate(DATA["groups"]):
        source, settings = graph.add_node_of_type(u.PCGCreatePointsSettings)
        source.set_node_position(0, index * 190)
        source.set_editor_property("node_title", group["biome"] + " " + group["layer"] + " exclusion mask")
        points = []
        for i, p in enumerate(group["instances"]):
            point = u.PCGPoint()
            point.set_editor_properties(dict(transform=u.Transform(location=u.Vector(*p[:3]), rotation=u.Rotator(yaw=p[3]), scale=u.Vector(p[4], p[4], p[4])), seed=DATA["seed"] + i))
            points.append(point)
        settings.set_editor_property("points_to_create", points)
        spawner, spawn_settings = graph.add_node_of_type(u.PCGStaticMeshSpawnerSettings)
        spawner.set_node_position(450, index * 190)
        spawn_settings.set_mesh_selector_type(u.PCGMeshSelectorWeighted)
        spawn_settings.set_editor_property("synchronous_load", True)
        entry = u.PCGMeshSelectorWeightedEntry()
        desc = entry.get_editor_property("descriptor")
        desc.set_editor_property("static_mesh", meshes[group["mesh"]])
        desc.set_editor_property("component_class", u.HierarchicalInstancedStaticMeshComponent)
        body = desc.get_editor_property("body_instance")
        body.set_editor_property("collision_profile_name", "NoCollision")
        desc.set_editor_property("body_instance", body)
        small = group["layer"] in ("grass", "flower", "snow")
        desc.set_editor_property("cast_shadow", not small)
        desc.set_editor_property("instance_start_cull_distance", 3500 if small else 10000)
        desc.set_editor_property("instance_end_cull_distance", 6000 if small else 16000)
        entry.set_editor_property("descriptor", desc)
        spawn_settings.get_editor_property("mesh_selector_parameters").set_editor_property("mesh_entries", [entry])
        graph.add_edge(source, "Out", spawner, "In")
        graph.add_edge(spawner, "Out", graph.get_output_node(), "Out")
    EAL.save_loaded_asset(graph)
    return graph


def finish():
    baked = component.clear_pcg_link()
    if not baked:
        raise RuntimeError("PCG did not produce a baked actor")
    baked.set_actor_label("Baked_CandyEcology")
    baked.set_folder_path("Sugarborough/Dense PCG")
    # Keep the source graph editable, with manual generation to avoid duplicates.
    volume.set_editor_property("is_editor_only_actor", True)
    components = list(baked.get_components_by_class(u.InstancedStaticMeshComponent))
    count = sum(c.get_instance_count() for c in components)
    expected = sum(DATA["counts"].values())
    if count != expected:
        raise RuntimeError(f"PCG instance count mismatch {count} vs {expected}")
    for c in components:
        c.set_collision_profile_name("NoCollision")
        c.set_collision_enabled(u.CollisionEnabled.NO_COLLISION)
    if not u.EditorLoadingAndSavingUtils.save_map(world, "/Game/World/L_Sugarborough"):
        raise RuntimeError("Dense world save failed")
    # Camera copies are made by capture_maps.py, which chooses fresh package
    # names instead of trying to overwrite another loaded World object.
    report = dict(status="passed", pcg_graph=BASE + "/PCG/PCG_CandyEcology", baked_instances=count, groups=len(components), layers=DATA["counts"], main_map="/Game/World/L_Sugarborough", review_maps=[], review_maps_status="run capture_maps.py after this bake")
    (HERE / "engine_report.json").write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    (HERE / "failure.txt").unlink(missing_ok=True)
    u.log("DENSE PCG BAKE PASS " + json.dumps(report))


def tick(delta):
    global phase
    try:
        if time.monotonic() - started > 240:
            raise RuntimeError("PCG generation timed out")
        if component.get_editor_property("generated"):
            u.unregister_slate_post_tick_callback(callback)
            finish()
            u.SystemLibrary.quit_editor()
    except Exception:
        u.unregister_slate_post_tick_callback(callback)
        (HERE / "failure.txt").write_text(traceback.format_exc(), encoding="utf-8")
        u.log_error(traceback.format_exc())
        u.SystemLibrary.quit_editor()


def main():
    global meshes, terrain_material, ecology_graph, world, volume, component, started, callback
    (HERE / "engine_report.json").write_text(json.dumps({"status": "building"}), encoding="utf-8")
    u.EditorPythonScripting.set_keep_python_script_alive(True)
    try:
        meshes, terrain_material = import_assets()
        ecology_graph = graph(meshes)
        world = u.EditorLoadingAndSavingUtils.load_map("/Game/World/L_Sugarborough")
        for actor in list(ACTORS.get_all_level_actors()):
            if actor.get_actor_label().startswith(("Scatter_", "Baked_CandyEcology", "PCG_CandyEcology")):
                ACTORS.destroy_actor(actor)
            elif isinstance(actor, u.StaticMeshActor) and actor.static_mesh_component.static_mesh and actor.static_mesh_component.static_mesh.get_name().startswith("SM_Terrain"):
                actor.static_mesh_component.set_material(0, terrain_material)
            elif isinstance(actor, u.DirectionalLight):
                actor.light_component.set_editor_property("intensity", 3.5)
            elif isinstance(actor, u.SkyLight):
                actor.light_component.set_editor_property("real_time_capture", False)
                actor.light_component.set_editor_property("source_type", u.SkyLightSourceType.SLS_SPECIFIED_CUBEMAP)
                actor.light_component.set_editor_property("cubemap", u.load_asset("/Engine/MapTemplates/Sky/DaylightAmbientCubemap"))
                actor.light_component.set_editor_property("intensity", .9)
        volume = ACTORS.spawn_actor_from_class(u.PCGVolume, u.Vector(0, 0, 0))
        volume.set_actor_label("PCG_CandyEcology_Source")
        component = volume.get_component_by_class(u.PCGComponent)
        component.set_editor_property("generation_trigger", u.PCGComponentGenerationTrigger.GENERATE_ON_DEMAND)
        component.set_editor_property("regenerate_in_editor", False)
        component.set_graph(ecology_graph)
        started = time.monotonic()
        callback = u.register_slate_post_tick_callback(tick)
        component.generate(True)
    except Exception:
        (HERE / "failure.txt").write_text(traceback.format_exc(), encoding="utf-8")
        u.log_error(traceback.format_exc())
        u.SystemLibrary.quit_editor()


if __name__ == "__main__":
    main()
