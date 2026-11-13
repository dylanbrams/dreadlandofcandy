"""Unreal: class-specific clips, rigid weapon grips and all twelve transparent UI icons."""
import json
import math
from pathlib import Path
import shutil
import unreal

ROOT=Path(unreal.Paths.project_dir()).resolve()
ROWS=json.loads((ROOT/"Scripts/class_motion_roster.json").read_text())
EAL=unreal.EditorAssetLibrary
TOOLS=unreal.AssetToolsHelpers.get_asset_tools()
OUT=ROOT/"ImportSource/RigCorrections/ClassSkills"
OUT.mkdir(parents=True,exist_ok=True)

def mul(a,b):
    x,y,z,w=a; X,Y,Z,W=b
    return [w*X+x*W+y*Z-z*Y,w*Y-x*Z+y*W+z*X,w*Z+x*Y-y*X+z*W,w*W-x*X-y*Y-z*Z]
def inv(q): return [-q[0],-q[1],-q[2],q[3]]
def unit(v):
    n=math.sqrt(sum(x*x for x in v))
    return [x/n for x in v] if n>1e-8 else [0,0,1]
def dot(a,b): return sum(x*y for x,y in zip(a,b))
def cross(a,b): return [a[1]*b[2]-a[2]*b[1],a[2]*b[0]-a[0]*b[2],a[0]*b[1]-a[1]*b[0]]
def rotate(q,v): return mul(mul(q,[*v,0]),inv(q))[:3]
def xyz(v): return [v.x,v.y,v.z]
def quat(q): return [q.x,q.y,q.z,q.w]
def basis_quat(x,y,z):
    m=[[x[i],y[i],z[i]] for i in range(3)]
    trace=sum(m[i][i] for i in range(3))
    if trace>0:
        s=math.sqrt(trace+1)*2
        q=[(m[2][1]-m[1][2])/s,(m[0][2]-m[2][0])/s,(m[1][0]-m[0][1])/s,s/4]
    else:
        i=max(range(3),key=lambda k:m[k][k]); j=(i+1)%3; k=(i+2)%3
        s=math.sqrt(1+m[i][i]-m[j][j]-m[k][k])*2
        q=[0,0,0,0]; q[i]=s/4; q[j]=(m[j][i]+m[i][j])/s; q[k]=(m[k][i]+m[i][k])/s; q[3]=(m[k][j]-m[j][k])/s
    return unit(q)

def grip(sequence,row):
    # The attachment's UE roll -90 maps weapon +Z to hand -Y.
    relative=[math.sqrt(0.5),0,0,math.sqrt(0.5)]
    hand=row["hand"]
    parent="lowerarm_l" if hand=="hand_l" else "lowerarm_r"
    shoulder="upperarm_l" if hand=="hand_l" else "upperarm_r"
    model=sequence.get_editor_property("data_model_interface")
    controller=sequence.get_editor_property("controller")
    rate=model.get_frame_rate(); fps=rate.numerator/rate.denominator
    times=[i/fps for i in range(model.get_number_of_frames()+1)]
    options=unreal.AnimPoseEvaluationOptions()
    options.set_editor_property("evaluation_type",unreal.AnimDataEvalType.SOURCE)
    options.set_editor_property("should_retarget",False)
    positions=[]; scales=[]; rotations=[]; originals=[]; targets=[]
    for t in times:
        pose=unreal.AnimPoseExtensions.get_anim_pose_at_time(sequence,t,options)
        def world(b): return unreal.AnimPoseExtensions.get_bone_pose(pose,b,unreal.AnimPoseSpaces.WORLD)
        local=unreal.AnimPoseExtensions.get_bone_pose(pose,hand,unreal.AnimPoseSpaces.LOCAL)
        positions.append(local.translation); scales.append(local.scale3d)
        originals.append({"p":xyz(local.translation),"q":quat(local.rotation),"s":xyz(local.scale3d)})
        hand_pos=xyz(world(hand).translation); shoulder_pos=xyz(world(shoulder).translation)
        aim=unit([a-b for a,b in zip(hand_pos,shoulder_pos)])
        z=[0,0,1]; x=[0,1,0]
        if row["who"]=="Archer":
            x=aim
            z=unit([a-aim[i]*dot([0,0,1],aim) for i,a in enumerate([0,0,1])])
        elif row["clip"] in ("BarbarianKnead","BarbarianCyclone"):
            z=unit([aim[0],aim[1],0]); x=[0,0,1]
        y=unit(cross(z,x)); x=unit(cross(y,z))
        target=basis_quat(x,y,z)
        hand_world=mul(target,inv(relative))
        local_q=unit(mul(inv(quat(world(parent).rotation)),hand_world))
        rotations.append(unreal.Quat(*local_q)); targets.append(z)
    backup=OUT/(sequence.get_name()+"_grip_source.json")
    backup.write_text(json.dumps({"asset":sequence.get_path_name(),"hand":hand,"fps":fps,"keys":originals},indent=2))
    controller.open_bracket("Align class weapon grip",False)
    try:
        assert controller.set_bone_track_keys(hand,positions,rotations,scales,False)
    finally:
        controller.close_bracket(False)
    error=0
    for t,target in zip(times,targets):
        pose=unreal.AnimPoseExtensions.get_anim_pose_at_time(sequence,t,options)
        actual=unreal.AnimPoseExtensions.get_bone_pose(pose,hand,unreal.AnimPoseSpaces.WORLD)
        axis=unit(rotate(mul(quat(actual.rotation),relative),[0,0,1]))
        error=max(error,math.degrees(math.acos(max(-1,min(1,dot(axis,target))))))
    assert error<0.1,(row["id"],error)
    EAL.save_loaded_asset(sequence,only_if_is_dirty=False)
    return {"bone":hand,"keys":len(times),"max_axis_error_degrees":error,"other_tracks":"untouched"}

report=[]
for row in ROWS:
    who,clip=row["who"],row["clip"]
    name=f"A_SK_{who}_Rig_{clip}"
    dest=f"/Game/Art/Cast/{who}/Anims"
    asset=dest+"/"+name
    mesh=unreal.load_asset(f"/Game/Art/Cast/{who}/SK_{who}_Rig")
    skeleton=mesh.get_editor_property("skeleton")
    if row["id"]!="K02" or not EAL.does_asset_exist(asset):
        ui=unreal.FbxImportUI()
        ui.set_editor_properties({"import_mesh":False,"import_animations":True,"import_materials":False,
            "import_textures":False,"skeleton":skeleton,"mesh_type_to_import":unreal.FBXImportType.FBXIT_ANIMATION})
        task=unreal.AssetImportTask()
        task.set_editor_properties({"filename":str(ROOT/f"ImportSource/Meshy/SK_{who}_Rig/{name}.fbx"),
            "destination_path":dest,"automated":True,"replace_existing":True,"save":True,"options":ui})
        TOOLS.import_asset_tasks([task])
        assert task.get_editor_property("imported_object_paths"),name
    sequence=unreal.load_asset(asset)
    assert sequence and sequence.get_editor_property("skeleton")==skeleton and sequence.get_play_length()>1
    entry={**row,"asset":asset,"seconds":sequence.get_play_length()}
    if row["id"]!="K02":
        original=ROOT/("Content/"+asset.removeprefix("/Game/")+".uasset")
        backup=OUT/(name+"_before_grip.uasset")
        if not backup.exists(): shutil.copy2(original,backup)
        entry["grip"]=grip(sequence,row)
    report.append(entry)
    unreal.log("ClassPresentation: validated "+row["id"]+" "+clip)

icons=[]
for row in json.loads((ROOT/"Assets/Incoming/2D/ClassSkills/manifest.json").read_text()):
    name="T_Skill_"+row["id"]
    task=unreal.AssetImportTask()
    task.set_editor_properties({"filename":str(ROOT/row["project_source"]),"destination_path":"/Game/Art/UI/Skills",
        "destination_name":name,"automated":True,"replace_existing":True,"save":True})
    TOOLS.import_asset_tasks([task])
    texture=unreal.load_asset("/Game/Art/UI/Skills/"+name)
    assert isinstance(texture,unreal.Texture2D),name
    texture.set_editor_properties({"lod_group":unreal.TextureGroup.TEXTUREGROUP_UI,
        "compression_settings":unreal.TextureCompressionSettings.TC_EDITOR_ICON,
        "mip_gen_settings":unreal.TextureMipGenSettings.TMGS_NO_MIPMAPS,"max_texture_size":512,"srgb":True})
    EAL.save_loaded_asset(texture)
    icons.append({"id":row["id"],"asset":texture.get_path_name(),"primary":row["primary"],"secondary":row["secondary"]})
assert len(report)==12 and len(icons)==12
(OUT/"validation.json").write_text(json.dumps({"clips":report,"icons":icons},indent=2))
unreal.log("ClassPresentation: SUCCESS 12 class clips/grips and 12 icons")
