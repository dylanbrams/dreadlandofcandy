"""Create isolated fixed-camera review maps for the twelve Niagara attacks.

Unreal Python commandlet. Never changes the default map or gameplay source.
Each effect is held at a representative age for repeatable inspection.
"""
import json
from pathlib import Path
import unreal

BASE = "/Game/FX/Attacks"
ACTORS = unreal.get_default_object(unreal.EditorActorSubsystem)
EAL = unreal.EditorAssetLibrary


def spawn(cls, location=(0, 0, 0), rotation=(0, 0, 0)):
    return ACTORS.spawn_actor_from_class(cls, unreal.Vector(*location), unreal.Rotator(pitch=rotation[0], yaw=rotation[1], roll=rotation[2]))


def static_mesh(name, location, scale, material=None):
    actor = spawn(unreal.StaticMeshActor, location)
    comp = actor.static_mesh_component
    comp.set_static_mesh(unreal.load_asset(name))
    comp.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
    if material:
        comp.set_material(0, material)
    actor.set_actor_scale3d(unreal.Vector(*scale))
    return actor


def main():
    root = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
    assets = json.loads((root / "ImportSource/FX/niagara_assets.json").read_text(encoding="utf-8"))
    records = []
    for entry in assets:
        name = entry["asset"].split(".")[-1]
        if name.startswith("NS_FX_") or "_Layer" in name:
            continue
        world = unreal.EditorLoadingAndSavingUtils.new_blank_map(False)
        world.get_world_settings().set_editor_property("default_game_mode", unreal.GameModeBase)
        floor = static_mesh("/Engine/BasicShapes/Cube", (300, 0, -12), (24, 20, 0.1))
        # An unlit dim backdrop keeps the particle silhouettes visible without a lighting rig.
        floor_mat_path = f"{BASE}/Review/MI_FX_ReviewFloor"
        if EAL.does_asset_exist(floor_mat_path):
            floor_mat = unreal.load_asset(floor_mat_path)
        else:
            floor_mat = unreal.AssetToolsHelpers.get_asset_tools().create_asset("MI_FX_ReviewFloor", f"{BASE}/Review",
                unreal.MaterialInstanceConstant, unreal.MaterialInstanceConstantFactoryNew())
            unreal.MaterialEditingLibrary.set_material_instance_parent(floor_mat, unreal.load_asset(f"{BASE}/Materials/M_FX_Mesh"))
            unreal.MaterialEditingLibrary.set_material_instance_vector_parameter_value(floor_mat, "Tint", unreal.LinearColor(0.035, 0.05, 0.07, 1))
            EAL.save_loaded_asset(floor_mat)
        floor.static_mesh_component.set_material(0, floor_mat)
        marker = static_mesh("/Engine/BasicShapes/Cylinder", (-100, 0, 90), (0.65, 0.65, 1.8), unreal.load_asset(f"{BASE}/Materials/M_FX_Mesh"))
        marker.set_actor_label("180 cm scale marker")
        age = 0.2 if any(s in name for s in ("K01", "B01", "M01", "A01")) else 0.65 if any(s in name for s in ("M02", "A02")) else 0.35
        for layer in [entry["asset"]] + entry.get("layers", []):
            actor = spawn(unreal.load_class(None, "/Script/Niagara.NiagaraActor"))
            actor.set_actor_label(layer.split(".")[-1])
            comp = actor.get_component_by_class(unreal.NiagaraComponent)
            comp.set_asset(unreal.load_asset(layer))
            comp.set_force_solo(True)
            comp.set_age_update_mode(unreal.NiagaraAgeUpdateMode.DESIRED_AGE)
            comp.set_desired_age(age)
        camera = spawn(unreal.CameraActor, (-1000, 0, 1300), (-50, 0, 0))
        camera.camera_component.set_field_of_view(55)
        camera.set_editor_property("auto_activate_for_player", unreal.AutoReceiveInput.PLAYER0)
        path = f"{BASE}/Review/L_FX_{name[3:]}"
        if not unreal.EditorLoadingAndSavingUtils.save_map(world, path):
            raise RuntimeError("Failed to save review map " + path)
        records.append({"map": path, "system": entry["asset"], "desired_age": age,
                        "layers": entry.get("layers", []), "duration_cap": entry["duration_cap"],
                        "camera": {"distance_cm": 1640, "pitch": 50, "fov": 55}})
    (root / "ImportSource/FX/review_maps.json").write_text(json.dumps(records, indent=2) + "\n", encoding="utf-8")
    unreal.log(f"FX REVIEW MAPS PASS: {len(records)} maps")


if __name__ == "__main__":
    main()
