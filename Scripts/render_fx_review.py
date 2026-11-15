"""Render FX review maps in an offscreen editor viewport, using their saved cameras."""
import json
from pathlib import Path
import time
import traceback
import unreal

ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
unreal.EditorPythonScripting.set_keep_python_script_alive(True)
request = json.loads((ROOT / "Saved/FX/render_request.json").read_text(encoding="utf-8-sig"))
entries = json.loads((ROOT / "ImportSource/FX/review_maps.json").read_text(encoding="utf-8"))
entries = [e for e in entries if not request["attack"] or e["system"].split(".")[-1].split("_")[1] == request["attack"]]
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
state = {"index": 0, "phase": "load", "task": None, "deadline": time.monotonic() + 180}
results = []


def tick(delta):
    if state.get("busy"):
        return
    state["busy"] = True
    try:
        if time.monotonic() > state["deadline"]:
            raise RuntimeError("FX screenshot timed out")
        if state["phase"] == "load":
            if state["index"] == len(entries):
                (ROOT / "Saved/FX/render_results.json").write_text(json.dumps(results, indent=2), encoding="utf-8")
                unreal.unregister_slate_post_tick_callback(handle)
                unreal.SystemLibrary.quit_editor()
                return
            entry = entries[state["index"]]
            unreal.EditorLoadingAndSavingUtils.load_map(entry["map"])
            current = actors.get_all_level_actors()
            camera = next(a for a in current if isinstance(a, unreal.CameraActor))
            camera.set_actor_rotation(unreal.Rotator(pitch=-50, yaw=0, roll=0), False)
            exposure = camera.camera_component.get_editor_property("post_process_settings")
            exposure.set_editor_properties({"override_auto_exposure_method": True,
                "auto_exposure_method": unreal.AutoExposureMethod.AEM_MANUAL,
                "override_auto_exposure_apply_physical_camera_exposure": True,
                "auto_exposure_apply_physical_camera_exposure": False,
                "override_auto_exposure_bias": True, "auto_exposure_bias": 0})
            camera.camera_component.set_editor_property("post_process_settings", exposure)
            for actor in current:
                for component in actor.get_components_by_class(unreal.NiagaraComponent):
                    component.set_visibility(False)
                    component.deactivate()
            unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).set_level_viewport_camera_info(camera.get_actor_location(), camera.get_actor_rotation())
            # Prime the mint mesh/material before Niagara's paused first-frame capture.
            # A newly imported mesh otherwise appears black while its render resources initialize.
            if "NS_M02_PeppermintPopfall" in entry["system"]:
                warm = actors.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(0, 0, -10000))
                warm.static_mesh_component.set_static_mesh(unreal.load_asset("/Game/FX/Attacks/Meshes/SM_FX_Peppermint"))
                state["warm_actor"] = warm
            state.update(phase="wait", camera=camera, ready=time.monotonic() + 3, deadline=time.monotonic() + 180)
        elif state["phase"] == "wait" and time.monotonic() >= state["ready"]:
            state["phase"] = "preparing_capture"
            entry = entries[state["index"]]
            counts = {}
            attributes = {}
            components = []
            for system_path in [entry["system"]] + entry.get("layers", []):
                actor = actors.spawn_actor_from_class(unreal.load_class(None, "/Script/Niagara.NiagaraActor"), unreal.Vector(0,0,0))
                component = actor.get_component_by_class(unreal.NiagaraComponent)
                components.append(component)
                component.set_editor_property("wait_for_compilation_on_activate", True)
                component.set_asset(unreal.load_asset(system_path))
                component.set_visibility(True)
                component.set_rendering_enabled(True)
                component.set_system_fixed_bounds(unreal.Box(min=unreal.Vector(-1000,-1000,-100), max=unreal.Vector(1400,1000,1400)))
                component.set_force_solo(True)
                component.set_age_update_mode(unreal.NiagaraAgeUpdateMode.TICK_DELTA_TIME)
                component.activate(True)
                component.advance_simulation(max(1, round(entry["desired_age"] * 60)), 1.0 / 60.0)
                cache = unreal.NiagaraSimCacheFunctionLibrary.create_niagara_sim_cache(component)
                unreal.NiagaraSimCacheFunctionLibrary.capture_niagara_sim_cache_immediate(cache, unreal.NiagaraSimCacheCreateParameters(), component)
                if cache.get_num_frames() == 0:
                    raise RuntimeError("No live simulation frames: " + system_path)
                layer_count = 0
                for emitter_name in cache.get_emitter_names():
                    positions = cache.read_position_attribute("Position", emitter_name)
                    counts[system_path.split(".")[-1] + ":" + str(emitter_name)] = len(positions)
                    colors = cache.read_color_attribute("Color", emitter_name)
                    sizes = cache.read_vector2_attribute("SpriteSize", emitter_name)
                    attributes[system_path.split(".")[-1]] = {
                        "position_samples": [[p.x, p.y, p.z] for p in positions[:2]],
                        "color_samples": [[c.r, c.g, c.b, c.a] for c in colors[:2]],
                        "sprite_size_samples": [[s.x, s.y] for s in sizes[:2]]}
                    layer_count += len(positions)
                    unreal.log("FX PARTICLES: " + system_path + " count=" + str(len(positions)) + " sample=" + str(positions[:2]))
                if not layer_count:
                    raise RuntimeError("No live layer particles: " + system_path)
                component.set_paused(True)
            state["components"] = components
            if not sum(counts.values()):
                raise RuntimeError("No live particles: " + entry["system"])
            attack = entry["system"].split(".")[-1].split("_")[1]
            filename = str(ROOT / f"Saved/Screenshots/WindowsEditor/fx-{attack.lower()}.png")
            state["task"] = unreal.AutomationLibrary.take_high_res_screenshot(1280, 720, filename, camera=state["camera"], delay=0.25)
            state["phase"] = "capture"
            results.append({"attack": attack, "screenshot": filename, "map": entry["map"], "particles": counts,
                            "attributes": attributes})
        elif state["phase"] == "capture" and state["task"].is_task_done():
            if state.get("warm_actor"):
                actors.destroy_actor(state.pop("warm_actor"))
            for component in state["components"]:
                owner = component.get_owner()
                if not actors.destroy_actor(owner) or unreal.SystemLibrary.is_valid(component):
                    raise RuntimeError("Cast owner/component survived destruction")
            results[-1]["owner_destruction_cleanup"] = "pass"
            unreal.log("FX OWNER CLEANUP PASS: " + results[-1]["attack"])
            state.update(index=state["index"] + 1, phase="load", task=None)
    except Exception:
        (ROOT / "Saved/FX/render_error.txt").write_text(traceback.format_exc(), encoding="utf-8")
        unreal.log_error(traceback.format_exc())
        unreal.unregister_slate_post_tick_callback(handle)
        unreal.SystemLibrary.quit_editor()
    finally:
        state["busy"] = False


handle = unreal.register_slate_post_tick_callback(tick)
