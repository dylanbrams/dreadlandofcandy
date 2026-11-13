"""Unreal: import normalized cast weapons and validate bounds/materials."""
from pathlib import Path
import json
import unreal
ROOT=Path(unreal.Paths.project_dir()).resolve()
report=[]
for name in ("SM_MageStaff","SM_ArcherBow","SM_BarbarianRollingPin"):
    ui=unreal.FbxImportUI()
    ui.set_editor_properties({"import_mesh":True,"import_as_skeletal":False,
        "import_materials":True,"import_textures":True,"import_animations":False})
    ui.static_mesh_import_data.set_editor_properties({"combine_meshes":True,"auto_generate_collision":False})
    task=unreal.AssetImportTask()
    task.set_editor_properties({"filename":str(ROOT/f"ImportSource/CastWeapons/{name}.fbx"),
        "destination_path":"/Game/Art/Cast/Weapons","automated":True,"save":True,
        "destination_name":name,"replace_existing":True,"options":ui,"factory":unreal.FbxFactory()})
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    mesh=unreal.load_asset(f"/Game/Art/Cast/Weapons/{name}")
    assert isinstance(mesh,unreal.StaticMesh),name
    box=mesh.get_bounding_box()
    size=box.max-box.min
    assert size.z>size.x and size.z>size.y,name
    report.append({"name":name,"bounds_cm":[size.x,size.y,size.z],"materials":len(mesh.static_materials)})
(ROOT/"ImportSource/CastWeapons/validation.json").write_text(json.dumps(report,indent=2))
unreal.log("CastWeapons: SUCCESS "+json.dumps(report))
