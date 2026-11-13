"""Bind explicit per-weapon materials; FBX's automatic material import is unreliable."""
import json
import struct
from pathlib import Path
import unreal
ROOT=Path(unreal.Paths.project_dir()).resolve()
DEST="/Game/Art/Cast/Weapons"
TOOLS=unreal.AssetToolsHelpers.get_asset_tools()
LIB=unreal.MaterialEditingLibrary
EAL=unreal.EditorAssetLibrary

def material(name,color,metallic,texture=None):
    path=DEST+"/"+name
    mat=unreal.load_asset(path) if EAL.does_asset_exist(path) else TOOLS.create_asset(name,DEST,unreal.Material,unreal.MaterialFactoryNew())
    LIB.delete_all_material_expressions(mat)
    if texture:
        node=LIB.create_material_expression(mat,unreal.MaterialExpressionTextureSample)
        node.set_editor_property("texture",texture)
        LIB.connect_material_property(node,"RGB",unreal.MaterialProperty.MP_BASE_COLOR)
    else:
        node=LIB.create_material_expression(mat,unreal.MaterialExpressionConstant3Vector)
        node.set_editor_property("constant",unreal.LinearColor(*color))
        LIB.connect_material_property(node,"",unreal.MaterialProperty.MP_BASE_COLOR)
    for value,prop in ((metallic,unreal.MaterialProperty.MP_METALLIC),(0.48,unreal.MaterialProperty.MP_ROUGHNESS)):
        node=LIB.create_material_expression(mat,unreal.MaterialExpressionConstant)
        node.set_editor_property("r",value)
        LIB.connect_material_property(node,"",prop)
    LIB.recompile_material(mat)
    EAL.save_loaded_asset(mat)
    return mat

for name in ("SM_MageStaff","SM_ArcherBow"):
    raw=(ROOT/f"ImportSource/Meshy/{name}.glb").read_bytes()
    length=struct.unpack_from("<I",raw,12)[0]
    doc=json.loads(raw[20:20+length])
    binary=raw[28+length:]
    texture_index=doc["materials"][0]["pbrMetallicRoughness"]["baseColorTexture"]["index"]
    image=doc["images"][doc["textures"][texture_index]["source"]]
    view=doc["bufferViews"][image["bufferView"]]
    start=view.get("byteOffset",0)
    suffix=".jpg" if image.get("mimeType")=="image/jpeg" else ".png"
    source=ROOT/f"ImportSource/CastWeapons/{name}_BaseColor{suffix}"
    source.write_bytes(binary[start:start+view["byteLength"]])
    task=unreal.AssetImportTask()
    task.set_editor_properties({"filename":str(source),"destination_path":DEST,"automated":True,"replace_existing":True,"save":True})
    TOOLS.import_asset_tasks([task])
    texture=unreal.load_asset(DEST+"/"+source.stem)
    assert isinstance(texture,unreal.Texture2D),name
    texture.set_editor_property("max_texture_size",2048)
    EAL.save_loaded_asset(texture)
    mat=material("M_"+name,[0.2,0.2,0.2,1],0.65 if name=="SM_MageStaff" else 0.25,texture)
    mesh=unreal.load_asset(DEST+"/"+name)
    mesh.set_material(0,mat)
    EAL.save_loaded_asset(mesh)
pin=unreal.load_asset(DEST+"/SM_BarbarianRollingPin")
pin.set_material(0,material("M_RollingPinOak",[0.30,0.12,0.035,1],0.0))
pin.set_material(1,material("M_RollingPinIron",[0.11,0.12,0.14,1],0.8))
EAL.save_loaded_asset(pin)
for unused in ("SM_MageStaff1","Material_0"):
    path=DEST+"/"+unused
    if EAL.does_asset_exist(path):
        assert EAL.delete_asset(path),path
unreal.log("CastWeapons: explicit textured materials SUCCESS")
