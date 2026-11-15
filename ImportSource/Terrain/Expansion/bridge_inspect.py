"""Report the Riverlands bridge's mesh bounds, scale and placement (read-only). Commandlet use."""
import unreal as u
u.EditorLoadingAndSavingUtils.load_map('/Game/World/L_Sugarborough')
actors = u.get_editor_subsystem(u.EditorActorSubsystem)
for a in actors.get_all_level_actors():
    lab = a.get_actor_label()
    if 'Bridge' in lab or 'bridge' in lab:
        line = 'BRIDGE_INSPECT %s loc=%s rot=%s scale=%s' % (lab, a.get_actor_location(), a.get_actor_rotation(), a.get_actor_scale3d())
        if isinstance(a, u.StaticMeshActor) and a.static_mesh_component.static_mesh:
            m = a.static_mesh_component.static_mesh
            b = m.get_bounding_box()
            line += ' mesh=%s min=%s max=%s' % (m.get_path_name(), b.min, b.max)
        u.log(line)
