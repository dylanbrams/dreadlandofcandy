"""Make isolated review maps from saved world, without changing gameplay map."""
import unreal as u
world=u.EditorLoadingAndSavingUtils.load_map('/Game/World/L_Sugarborough')
actors=u.get_editor_subsystem(u.EditorActorSubsystem)
world.get_world_settings().set_editor_property('default_game_mode',u.GameModeBase)
for actor in actors.get_all_level_actors():
 if isinstance(actor,u.CameraActor):actor.set_editor_property('auto_activate_for_player',u.AutoReceiveInput.DISABLED)
cam=actors.spawn_actor_from_class(u.CameraActor,u.Vector(-2100,-2200,1700),u.Rotator(pitch=-29,yaw=48))
cam.set_editor_property('auto_activate_for_player',u.AutoReceiveInput.PLAYER0)
assert u.EditorLoadingAndSavingUtils.save_map(world,'/Game/World/CandyPBR/L_VillageReview')
cam.set_actor_location(u.Vector(18100,-1800,700),False,False)
cam.set_actor_rotation(u.Rotator(pitch=-22,yaw=45),False)
assert u.EditorLoadingAndSavingUtils.save_map(world,'/Game/World/CandyPBR/L_ChocolateReview')
u.log('CANDY REVIEW MAPS READY')
