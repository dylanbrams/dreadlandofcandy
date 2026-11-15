"""Blend selected PBR sets onto existing terrain and extension. Exclusive UE use.

Requires import_candy_pbr.py first. Saves material/mesh assets, never the map.
"""
import unreal as u
E=u.MaterialEditingLibrary
A=u.EditorAssetLibrary
BASE='/Game/World/CandyPBR'
tools=u.AssetToolsHelpers.get_asset_tools()
def build(name,extended=False,bank=False):
 mat=u.load_asset(BASE+'/'+name) or tools.create_asset(name,BASE,u.Material,u.MaterialFactoryNew())
 E.delete_all_material_expressions(mat)
 def n(cls,**props):
  v=E.create_material_expression(mat,cls);v.set_editor_properties(props);return v
 def link(a,b,pin,out=''):
  assert E.connect_material_expressions(a,out,b,pin),pin
 def custom(code,inputs,output=u.CustomMaterialOutputType.CMOT_FLOAT3):
  items=[]
  for key in inputs:
   v=u.CustomInput();v.set_editor_property('input_name',key);items.append(v)
  v=n(u.MaterialExpressionCustom,code=code,inputs=items,output_type=output)
  for key,node in inputs.items():link(node,v,key)
  return v
 pos=n(u.MaterialExpressionWorldPosition)
 xy=n(u.MaterialExpressionComponentMask,r=True,g=True);link(pos,xy,'')
 uv=n(u.MaterialExpressionMultiply,const_b=1/400.);link(xy,uv,'A')
 weights=custom('''float2 p=P.xy; float r=length(p);
float d=r>2700?abs(r-3600):100000;
for(int k=0;k<3;k++){float a=k*2.0943951;float along=dot(p,float2(cos(a),sin(a)));if(along>=0&&along<=10500)d=min(d,abs(dot(p,float2(-sin(a),cos(a)))));}
float path=max(1-smoothstep(200,280,d),1-smoothstep(950,1050,r));
path*=1-smoothstep(10300,10700,max(abs(p.x),abs(p.y)));
float extension=smoothstep(11200,12200,p.x);
float extpath=1-smoothstep(200,340,abs(p.y));
path=lerp(path,extpath,extension);
float plaza=1-smoothstep(950,1100,r);
float mint=smoothstep(300,700,p.y)*(1-plaza);
float brittle=(1-plaza)*(1-mint);
float shoulder=smoothstep(160,210,lerp(d,abs(p.y),extension))*(1-smoothstep(240,340,lerp(d,abs(p.y),extension)))*(1-plaza);
return float4(plaza*path,brittle*path,mint*path,shoulder);''',{'P':pos},u.CustomMaterialOutputType.CMOT_FLOAT4)
 def channel(index):
  node=n(u.MaterialExpressionComponentMask,r=index==0,g=index==1,b=index==2,a=index==3);link(weights,node,'');return node
 w=[channel(i) for i in range(4)]
 def sample(kind,label):
  tex=u.load_asset(BASE+'/Textures/T_'+kind+'_'+label);assert tex,kind+label
  typ=u.MaterialSamplerType.SAMPLERTYPE_NORMAL if label=='normal' else u.MaterialSamplerType.SAMPLERTYPE_COLOR if label=='basecolor' else u.MaterialSamplerType.SAMPLERTYPE_LINEAR_COLOR
  s=n(u.MaterialExpressionTextureSample,texture=tex,sampler_type=typ,sampler_source=u.SamplerSourceMode.SSM_WRAP_WORLD_GROUP_SETTINGS);link(uv,s,'UVs');return s
 for label,prop in [('basecolor',u.MaterialProperty.MP_BASE_COLOR),('normal',u.MaterialProperty.MP_NORMAL),('roughness',u.MaterialProperty.MP_ROUGHNESS)]:
  samples={k:sample(k,label) for k in ('candy_grass','peppermint','peanut_brittle','chocolate_mint','rock_candy')}
  inputs={'G':samples['candy_grass'],'P':samples['peppermint'],'B':samples['peanut_brittle'],'M':samples['chocolate_mint'],'R':samples['rock_candy'],'W':weights}
  # All samples are float3; roughness is grayscale replicated across RGB.
  code='float3 v=G'+('*.72' if label=='basecolor' else '')+'; v=lerp(v,P,W.r); v=lerp(v,B,W.g); v=lerp(v,M,W.b); v=lerp(v,R,W.a); return '+('normalize(v);' if label=='normal' else 'v;')
  if bank:code='return lerp(G,R,.12);'
  output=custom(code,inputs)
  if label=='basecolor' and not extended:
   atlas=n(u.MaterialExpressionTextureSample,texture=u.load_asset('/Game/World/Textures/T_SugarboroughGround'),sampler_type=u.MaterialSamplerType.SAMPLERTYPE_LINEAR_COLOR)
   # Keep snow, chocolate and orchard palettes beyond the central green ground.
   output=custom('float preserve=smoothstep(2200,2900,length(P.xy))*(1-smoothstep(10800,12000,P.x)); float road=saturate(W.r+W.g+W.b+W.a); return lerp(C,Ground,preserve*(1-road));',{'P':pos,'W':weights,'C':output,'Ground':atlas})
  if label=='roughness':
   scalar=n(u.MaterialExpressionComponentMask,r=True);link(output,scalar,'');output=scalar
  assert E.connect_material_property(output,'',prop)
 mat.set_editor_property('tangent_space_normal',False)
 E.recompile_material(mat);assert A.save_loaded_asset(mat)
 return mat
terrain=build('M_CandyTerrain')
extension=build('M_CandyRiverlands',True)
bankmat=build('M_CandyBanks',True,True)
count=0
for folder in ('/Game/World/Meshes','/Game/World/Expansion'):
 for path in A.list_assets(folder,recursive=False):
  mesh=u.load_asset(path)
  if not isinstance(mesh,u.StaticMesh):continue
  name=mesh.get_name()
  if not (name.startswith('SM_Terrain_') or name.startswith('SM_EastOpening_') or name=='SM_Riverlands'):continue
  for i,slot in enumerate(mesh.get_editor_property('static_materials')):
   old=slot.get_editor_property('material_interface')
   if name=='SM_Riverlands':
    slotname=str(slot.get_editor_property('material_slot_name')).lower()
    mesh.set_material(i,bankmat if 'bank' in slotname else extension)
   elif old and old.get_name() in ('M_Terrain','M_CandyTerrain','M_CandyPaving'):mesh.set_material(i,terrain)
  A.save_loaded_asset(mesh);count+=1
u.log('CANDY GROUND applied: %d meshes'%count)
