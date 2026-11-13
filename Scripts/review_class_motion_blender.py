"""Blender source-pose review, independent of Unreal engine ownership."""
import argparse
import json
from pathlib import Path
import sys
import bpy
from mathutils import Matrix,Vector

ROOT=Path(__file__).resolve().parents[1]
HERE=Path(__file__).resolve().parent
OUT=ROOT/"ImportSource/_gen/class_motion_review"
OUT.mkdir(parents=True,exist_ok=True)
parser=argparse.ArgumentParser()
parser.add_argument("--only",default="")
args=parser.parse_args(sys.argv[sys.argv.index("--")+1:] if "--" in sys.argv else [])
rows=json.loads((HERE/"class_motion_roster.json").read_text())
if args.only:
    rows=[r for r in rows if r["id"] in args.only.split(",")]
report_path=OUT/"source_pose_validation.json"
results=json.loads(report_path.read_text()) if report_path.exists() else []
for row in rows:
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete(use_global=False)
    who,clip=row["who"],row["clip"]
    src=ROOT/f"ImportSource/Meshy/SK_{who}_Rig/A_SK_{who}_Rig_{clip}.fbx"
    bpy.ops.import_scene.fbx(filepath=str(src))
    arm=next(o for o in bpy.context.scene.objects if o.type=="ARMATURE")
    body=[o for o in bpy.context.scene.objects if o.type=="MESH"]
    assert body and row["hand"] in arm.pose.bones
    bpy.context.scene.frame_set(20)
    deps=bpy.context.evaluated_depsgraph_get()
    points=[obj.matrix_world@Vector(c) for obj in body for c in obj.evaluated_get(deps).bound_box]
    lo=Vector(tuple(min(p[i] for p in points) for i in range(3)))
    hi=Vector(tuple(max(p[i] for p in points) for i in range(3)))
    height=hi.z-lo.z
    hand=(arm.matrix_world@arm.pose.bones[row["hand"]].matrix).translation
    weapon={"Knight":"SM_Sword","Mage":"SM_MageStaff","Archer":"SM_ArcherBow","Barbarian":"SM_BarbarianRollingPin"}[who]
    weapon_file=ROOT/f"ImportSource/CastWeapons/{weapon}.fbx"
    if weapon_file.exists():
        prior=set(bpy.context.scene.objects)
        bpy.ops.import_scene.fbx(filepath=str(weapon_file))
        props=[o for o in bpy.context.scene.objects if o not in prior and o.type=="MESH"]
        for obj in props:
            target_height={"Mage":180,"Archer":140,"Barbarian":105}[who]/190*height
            local_height=max(v.co.z for v in obj.data.vertices)-min(v.co.z for v in obj.data.vertices)
            axis=Vector((0,0,1))
            aim=Vector((0,-1,0))
            upper="upperarm_l" if row["hand"]=="hand_l" else "upperarm_r"
            if upper in arm.pose.bones:
                aim=(hand-(arm.matrix_world@arm.pose.bones[upper].matrix).translation).normalized()
            if who=="Archer":
                axis=(axis-aim*axis.dot(aim)).normalized()
            elif clip in ("BarbarianKnead","BarbarianCyclone"):
                axis=Vector((aim.x,aim.y,0)).normalized()
            x=aim-axis*aim.dot(axis)
            if x.length<0.01:
                x=Vector((1,0,0))
            x.normalize()
            y=axis.cross(x).normalized()
            mat=Matrix((x,y,axis)).transposed().to_4x4()
            mat.translation=hand
            obj.matrix_world=mat@Matrix.Scale(target_height/local_height,4)
    scene=bpy.context.scene
    scene.render.engine="BLENDER_EEVEE"
    scene.eevee.taa_render_samples=24
    scene.render.resolution_x=640
    scene.render.resolution_y=640
    scene.render.resolution_percentage=100
    scene.world.color=(0.065,0.075,0.09)
    target=Vector(((hi.x+lo.x)/2,(hi.y+lo.y)/2,lo.z+height*0.52))
    bpy.ops.object.camera_add(location=target+Vector((height*1.7,-height*2.4,height*0.75)))
    camera=bpy.context.object
    camera.rotation_euler=(target-camera.location).to_track_quat('-Z','Y').to_euler()
    camera.data.type="ORTHO"
    camera.data.ortho_scale=height*1.45
    scene.camera=camera
    for offset,power,size in (((-2,-3,4),800,3),((3,1,3),1000,2),((-2,3,2),600,2)):
        bpy.ops.object.light_add(type="AREA",location=target+Vector(offset)*(height/2))
        light=bpy.context.object
        light.data.energy=power
        light.data.size=size
        light.rotation_euler=(target-light.location).to_track_quat('-Z','Y').to_euler()
    scene.view_settings.view_transform="Standard"
    scene.render.filepath=str(OUT/(row["id"]+".png"))
    bpy.ops.render.render(write_still=True)
    results=[r for r in results if r["id"]!=row["id"]]
    results.append({**row,"frame":20,"height":height,"hand_position":list(hand),"review":"Source pose only; final grip/carry verification remains in Unreal"})
(OUT/"source_pose_validation.json").write_text(json.dumps(results,indent=2))
