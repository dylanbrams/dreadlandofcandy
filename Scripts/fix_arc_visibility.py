"""Give the short shield arc its own readable opacity instead of the faint dome material."""
import unreal
path="/Game/FX/Attacks/Materials/MI_FX_ArcReadable"
mat=unreal.load_asset(path) if unreal.EditorAssetLibrary.does_asset_exist(path) else unreal.AssetToolsHelpers.get_asset_tools().create_asset(
    "MI_FX_ArcReadable","/Game/FX/Attacks/Materials",unreal.MaterialInstanceConstant,unreal.MaterialInstanceConstantFactoryNew())
unreal.MaterialEditingLibrary.set_material_instance_parent(mat,unreal.load_asset("/Game/FX/Attacks/Materials/M_FX_Rim"))
unreal.MaterialEditingLibrary.set_material_instance_scalar_parameter_value(mat,"Opacity",1.0)
unreal.EditorAssetLibrary.save_loaded_asset(mat)
mesh=unreal.load_asset("/Game/FX/Attacks/Meshes/SM_FX_Arc")
mesh.set_material(0,mat)
unreal.EditorAssetLibrary.save_loaded_asset(mesh)
unreal.log("CastParticles: shield arc opacity corrected")
