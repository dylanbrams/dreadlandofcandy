"""Live CPU simulation checks for every saved system: particles, BudgetOut, Stop and owner cleanup."""
import json
from pathlib import Path
import unreal
import time
import traceback
ROOT=Path(unreal.Paths.project_dir()).resolve()
actors=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
rows=json.loads((ROOT/"ImportSource/FX/niagara_assets.json").read_text())
results=[]
def snapshot(component, stopped=False):
    cache=unreal.NiagaraSimCacheFunctionLibrary.create_niagara_sim_cache(component)
    unreal.NiagaraSimCacheFunctionLibrary.capture_niagara_sim_cache_immediate(cache,unreal.NiagaraSimCacheCreateParameters(),component)
    if stopped and cache.get_num_frames()==0:
        assert not component.is_active()
        return [],[]
    assert cache.get_num_frames()>0
    positions=[]
    colors=[]
    for emitter in cache.get_emitter_names():
        positions.extend(cache.read_position_attribute("Position",emitter))
        colors.extend(cache.read_color_attribute("Color",emitter))
    return positions,colors
def check(row):
    path=row["asset"]
    owner=actors.spawn_actor_from_class(unreal.load_class(None,"/Script/Niagara.NiagaraActor"),unreal.Vector())
    component=owner.get_component_by_class(unreal.NiagaraComponent)
    component.set_editor_property("wait_for_compilation_on_activate",True)
    component.set_asset(unreal.load_asset(path))
    component.set_force_solo(True)
    component.set_rendering_enabled(True)
    component.set_age_update_mode(unreal.NiagaraAgeUpdateMode.TICK_DELTA_TIME)
    component.activate(True)
    component.advance_simulation(6,1/60)
    positions,colors=snapshot(component)
    assert positions,path+" has no live particles"
    component.set_variable_bool("User.BudgetOut",True)
    component.advance_simulation(2,1/60)
    _,colors=snapshot(component)
    assert colors and all(abs(c.r-0.42)<0.002 and abs(c.g-0.42)<0.002 and abs(c.b-0.42)<0.002 for c in colors),path+" budget tint"
    component.set_variable_bool("User.Stop",True)
    component.advance_simulation(2,1/60)
    stopped,stopped_colors=snapshot(component,stopped=True)
    assert not stopped or (stopped_colors and all(c.a<0.001 for c in stopped_colors)),path+" remained visible after Stop"
    component.deactivate()
    assert actors.destroy_actor(owner)
    assert not unreal.SystemLibrary.is_valid(component),path+" leaked component"
    results.append({"asset":path,"particles":len(positions),"budget_out":"pass","stop_visibility":"pass","owner_cleanup":"pass"})
    unreal.log("CastParticles: PASS "+path)

unreal.EditorPythonScripting.set_keep_python_script_alive(True)
ready=time.monotonic()+4
def tick(delta):
    if time.monotonic()<ready:
        return
    unreal.unregister_slate_post_tick_callback(handle)
    try:
        for row in rows:
            check(row)
        (ROOT/"ImportSource/FX/live_parameter_validation.json").write_text(json.dumps(results,indent=2))
        unreal.log(f"CastParticles: SUCCESS {len(results)} systems")
    except Exception:
        unreal.log_error(traceback.format_exc())
        (ROOT/"Saved/CastDelivery/particle_error.txt").write_text(traceback.format_exc())
    finally:
        unreal.SystemLibrary.quit_editor()
handle=unreal.register_slate_post_tick_callback(tick)
