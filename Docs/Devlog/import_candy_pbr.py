"""Import selected PATINA maps and build world-aligned PBR materials.

Run in exclusive Unreal window after reviewing CandyPBR/selection.json.
Does not change existing assets except its own /Game/World/CandyPBR folder.
"""
from pathlib import Path
import json
import unreal as u
HERE=Path(__file__).resolve().parent
choices=json.loads((HERE/'CandyPBR/selection.json').read_text())
E=u.MaterialEditingLibrary
A=u.EditorAssetLibrary
tools=u.AssetToolsHelpers.get_asset_tools()
BASE='/Game/World/CandyPBR'
for kind,variant in choices.items():
 folder=HERE/'CandyPBR'/variant
 result=json.loads((folder/'result.json').read_text())
 maps={}
 for i,item in enumerate(result['images']):
  label=item.get('map_type')
  if label not in ('basecolor','normal','roughness','height'):continue
  task=u.AssetImportTask()
  task.set_editor_properties(dict(filename=str(folder/f'{label}_{i}.png'),destination_path=BASE+'/Textures',destination_name=f'T_{kind}_{label}',automated=True,save=True,replace_existing=True))
  tools.import_asset_tasks([task])
  tex=u.load_asset(task.imported_object_paths[0])
  tex.set_editor_property('srgb',label=='basecolor')
  tex.set_editor_property('compression_settings',u.TextureCompressionSettings.TC_NORMALMAP if label=='normal' else u.TextureCompressionSettings.TC_DEFAULT)
  A.save_loaded_asset(tex)
  maps[label]=tex
 assert all(k in maps for k in ('basecolor','normal','roughness')),kind
 name='M_'+kind
 mat=u.load_asset(BASE+'/'+name) or tools.create_asset(name,BASE,u.Material,u.MaterialFactoryNew())
 E.delete_all_material_expressions(mat)
 def node(cls,**props):
  n=E.create_material_expression(mat,cls);n.set_editor_properties(props);return n
 pos=node(u.MaterialExpressionWorldPosition)
 mask=node(u.MaterialExpressionComponentMask,r=True,g=True)
 scale=node(u.MaterialExpressionMultiply,const_b=1/350.)
 assert E.connect_material_expressions(pos,'',mask,'')
 assert E.connect_material_expressions(mask,'',scale,'A')
 for label,prop in [('basecolor',u.MaterialProperty.MP_BASE_COLOR),('roughness',u.MaterialProperty.MP_ROUGHNESS),('normal',u.MaterialProperty.MP_NORMAL)]:
  sample=node(u.MaterialExpressionTextureSample,texture=maps[label],sampler_type=u.MaterialSamplerType.SAMPLERTYPE_NORMAL if label=='normal' else (u.MaterialSamplerType.SAMPLERTYPE_COLOR if label=='basecolor' else u.MaterialSamplerType.SAMPLERTYPE_LINEAR_COLOR))
  assert E.connect_material_expressions(scale,'',sample,'UVs')
  assert E.connect_material_property(sample,'RGB' if label!='roughness' else 'R',prop)
 # Texture normals describe world XY ground, rather than arbitrary imported UVs.
 mat.set_editor_property('tangent_space_normal',False)
 E.recompile_material(mat)
 A.save_loaded_asset(mat)
u.log('CandyPBR candidate materials imported; existing world unmodified.')
