"""Re-aim the Fixes4 review cameras (review maps only)."""
import unreal as u
A=u.get_editor_subsystem(u.EditorActorSubsystem)
for m,loc,rot in [('L_RiverReview',(18400,-1500,260),(-14,62)),('L_RoadReview',(7600,-2600,1500),(-24,28))]:
    w=u.EditorLoadingAndSavingUtils.load_map('/Game/World/Expansion/'+m)
    for a in A.get_all_level_actors():
        if isinstance(a,u.CameraActor) and a.get_editor_property('auto_activate_for_player')==u.AutoReceiveInput.PLAYER0:
            a.set_actor_location(u.Vector(*loc),False,False);a.set_actor_rotation(u.Rotator(pitch=rot[0],yaw=rot[1]),False)
        if a.get_actor_label()=='Riverlands_SM_Riverlands':
            o,e=a.get_actor_bounds(False);u.log('FIXES4 riverlands actor bounds %s %s mesh %s'%(o,e,a.static_mesh_component.static_mesh.get_path_name()))
    assert u.EditorLoadingAndSavingUtils.save_map(w,'/Game/World/Expansion/'+m)
