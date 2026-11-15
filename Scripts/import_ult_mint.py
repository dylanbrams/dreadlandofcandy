from pathlib import Path
import unreal
root=Path(unreal.Paths.project_dir()).resolve()
ui=unreal.FbxImportUI()
ui.set_editor_properties({'import_mesh':True,'import_as_skeletal':False,'import_materials':True,'import_textures':True,'import_animations':False})
ui.static_mesh_import_data.set_editor_properties({'combine_meshes':True,'auto_generate_collision':False})
task=unreal.AssetImportTask()
task.set_editor_properties({'filename':str(root/'ImportSource/FX/MeshyMint/SM_FX_Peppermint.fbx'),'destination_path':'/Game/FX/Attacks/Meshes','destination_name':'SM_FX_Peppermint','automated':True,'save':True,'replace_existing':True,'replace_existing_settings':True,'options':ui,'factory':unreal.FbxFactory()})
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
mesh=unreal.load_asset('/Game/FX/Attacks/Meshes/SM_FX_Peppermint')
assert isinstance(mesh,unreal.StaticMesh)
tex_task=unreal.AssetImportTask()
tex_task.set_editor_properties({'filename':str(root/'ImportSource/FX/MeshyMint/texture_0.png'),'destination_path':'/Game/FX/Attacks/Textures','destination_name':'T_FX_MeshyMint','automated':True,'save':True,'replace_existing':True})
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([tex_task])
texture=unreal.load_asset('/Game/FX/Attacks/Textures/T_FX_MeshyMint')
assert texture
mat=unreal.load_asset('/Game/FX/Attacks/Materials/M_FX_Peppermint')
assert isinstance(mat,unreal.Material)
mat.set_editor_property('used_with_niagara_mesh_particles',True)
samples=[n for n in unreal.MaterialEditingLibrary.get_material_expressions(mat) if isinstance(n,unreal.MaterialExpressionTextureSample)]
assert samples
samples[0].set_editor_property('texture',texture)
unreal.MaterialEditingLibrary.connect_material_property(samples[0], 'RGB', unreal.MaterialProperty.MP_EMISSIVE_COLOR)
unreal.MaterialEditingLibrary.recompile_material(mat)
unreal.EditorAssetLibrary.save_loaded_asset(mat)
mesh.set_material(0,mat)
unreal.EditorAssetLibrary.save_loaded_asset(mesh)
print('MINT IMPORT OK',mesh.get_bounding_box(),[str(x.get_editor_property('material_interface').get_path_name()) for x in mesh.get_editor_property('static_materials')])
