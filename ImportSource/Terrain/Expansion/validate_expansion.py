"""Run in a full editor session: verify the saved bridge route and ground."""
import unreal as u
import time,json,traceback
from pathlib import Path
root=Path(u.Paths.project_dir()).resolve()/'ImportSource/Terrain/Expansion'
world=u.EditorLoadingAndSavingUtils.load_map('/Game/World/L_Sugarborough')
u.EditorPythonScripting.set_keep_python_script_alive(True)
start=time.monotonic()
def tick(dt):
    if time.monotonic()-start<3:return
    u.unregister_slate_post_tick_callback(handle)
    try:
        samples=[]
        for x in range(10000,24401,100):
            hit=u.SystemLibrary.line_trace_single(world,u.Vector(x,0,1800),u.Vector(x,0,-500),u.TraceTypeQuery.TRACE_TYPE_QUERY1,True,[],u.DrawDebugTrace.NONE)
            assert hit is not None, ('missing ground',x)
            z=hit.to_tuple()[5].z
            assert -20<z<250,('road obstructed or submerged',x,z)
            samples.append([x,z])
        # CharacterMovement's default MaxStepHeight is 45 cm. These 100 cm
        # ground samples are a conservative route screen, not a capsule walk.
        assert max(abs(b[1]-a[1]) for a,b in zip(samples,samples[1:]))<45,('unwalkable step',sorted([(abs(b[1]-a[1]),a,b) for a,b in zip(samples,samples[1:])],reverse=True)[:4])
        comps=[c for a in u.get_editor_subsystem(u.EditorActorSubsystem).get_all_level_actors() for c in a.get_components_by_class(u.HierarchicalInstancedStaticMeshComponent) if c.static_mesh and any(s in c.static_mesh.get_name() for s in ['Birch_Warp','Blossom_Warp','Fir_Warp'])]
        assert comps
        for c in comps:
            assert c.get_collision_response_to_channel(u.CollisionChannel.ECC_PAWN)==u.CollisionResponseType.ECR_BLOCK
            assert c.get_collision_response_to_channel(u.CollisionChannel.ECC_CAMERA)==u.CollisionResponseType.ECR_IGNORE
        c=comps[0];t=c.get_instance_transform(0,True);p=t.translation;s=t.scale3d
        a=p+u.Vector(-150*s.x,0,45*s.z);b=p+u.Vector(150*s.x,0,45*s.z)
        trunk_hit=u.SystemLibrary.line_trace_single_for_objects(world,a,b,[u.ObjectTypeQuery.ECC_WORLD_STATIC],False,[],u.DrawDebugTrace.NONE)
        if isinstance(trunk_hit,tuple):trunk_hit=trunk_hit[-1]
        assert trunk_hit is not None and trunk_hit.to_tuple()[10]==c,'Simple collision query did not hit selected trunk'
        for actor in u.get_editor_subsystem(u.EditorActorSubsystem).get_all_level_actors():
            if actor.get_actor_label().startswith('Riverlands_Trunk') or actor.get_actor_label()=='Riverlands_SquareTrunk':
                assert actor.static_mesh_component.get_collision_response_to_channel(u.CollisionChannel.ECC_PAWN)==u.CollisionResponseType.ECR_BLOCK
        result=dict(status='passed',road_samples=samples,tree_groups=len(comps),selected_trunk_simple_trace='passed',limitations='Ground and trunk traces plus Pawn response checks; does not prove manual pawn traversal or foliage visibility.')
    except Exception:
        result=dict(status='failed',error=traceback.format_exc());u.log_error(result['error'])
    (root/'validation.json').write_text(json.dumps(result,indent=2))
    u.SystemLibrary.quit_editor()
handle=u.register_slate_post_tick_callback(tick)
