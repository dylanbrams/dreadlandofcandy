"""Import the four already-generated skill images into the cooked art folder."""
from pathlib import Path
import unreal as u

root = Path(u.Paths.convert_relative_path_to_full(u.Paths.project_dir()))
names = ['T_Skill_Lecture','T_Skill_FormALine','T_Skill_DisappointedSigh','T_Skill_LastJudgment']
for name in names:
    task = u.AssetImportTask()
    task.set_editor_properties(dict(filename=str(root/'Assets/Incoming/2D'/name/(name+'.png')),
        destination_path='/Game/Art/UI/Icons', destination_name=name, automated=True,
        replace_existing=True, save=True))
    u.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    texture = u.load_asset('/Game/Art/UI/Icons/'+name)
    assert isinstance(texture,u.Texture2D), name
    texture.set_editor_properties(dict(lod_group=u.TextureGroup.TEXTUREGROUP_UI,
        compression_settings=u.TextureCompressionSettings.TC_EDITOR_ICON,
        mip_gen_settings=u.TextureMipGenSettings.TMGS_NO_MIPMAPS, srgb=True,
        max_texture_size=256))
    u.EditorAssetLibrary.save_loaded_asset(texture)
u.log('R7 SKILL ICON IMPORT PASS: 4 textures')

# One UI shader serves every class/icon. At cooldown=1 it is all grey;
# clockwise color returns from 12 o'clock as cooldown approaches zero.
path = '/Game/Art/UI/Materials/M_SkillCooldown'
mat = u.load_asset(path) if u.EditorAssetLibrary.does_asset_exist(path) else u.AssetToolsHelpers.get_asset_tools().create_asset(
    'M_SkillCooldown','/Game/Art/UI/Materials',u.Material,u.MaterialFactoryNew())
u.MaterialEditingLibrary.delete_all_material_expressions(mat)
mat.set_editor_properties(dict(material_domain=u.MaterialDomain.MD_UI,blend_mode=u.BlendMode.BLEND_TRANSLUCENT))
lib = u.MaterialEditingLibrary
def node(cls, **properties):
    expression = lib.create_material_expression(mat,cls)
    expression.set_editor_properties(properties)
    return expression
def connect(a,output,b,pin):
    if lib.connect_material_expressions(a,output,b,pin):
        return
    names=lib.get_material_expression_input_names(b)
    assert (len(names)==1 or pin=='Input') and lib.connect_material_expressions(a,output,b,names[0]),(a,b,pin,list(names))
def indexed(a,output,b,index):
    names=lib.get_material_expression_input_names(b)
    connect(a,output,b,names[index])
sample=node(u.MaterialExpressionTextureSampleParameter2D,parameter_name='IconTexture',
    texture=u.load_asset('/Game/Art/UI/Icons/T_Skill_Lecture'))
uv=node(u.MaterialExpressionTextureCoordinate)
center=node(u.MaterialExpressionSubtract,const_b=.5)
connect(uv,'',center,'A')
x=node(u.MaterialExpressionComponentMask,r=True,g=False,b=False,a=False)
y=node(u.MaterialExpressionComponentMask,r=False,g=True,b=False,a=False)
connect(center,'',x,'Input');connect(center,'',y,'Input')
negative_y=node(u.MaterialExpressionMultiply,const_b=-1.)
connect(y,'',negative_y,'A')
angle=node(u.MaterialExpressionArctangent2)
connect(x,'',angle,'Y');connect(negative_y,'',angle,'X')
turn=node(u.MaterialExpressionMultiply,const_b=1/(2*3.141592653589793))
connect(angle,'',turn,'A')
positive=node(u.MaterialExpressionAdd,const_b=1.)
connect(turn,'',positive,'A')
clock=node(u.MaterialExpressionFrac)
connect(positive,'',clock,'Input')
cooldown=node(u.MaterialExpressionScalarParameter,parameter_name='Cooldown',default_value=0.)
ready=node(u.MaterialExpressionOneMinus)
connect(cooldown,'',ready,'Input')
unlocked=node(u.MaterialExpressionScalarParameter,parameter_name='Unlocked',default_value=1.)
amount=node(u.MaterialExpressionMultiply)
connect(ready,'',amount,'A');connect(unlocked,'',amount,'B')
white=node(u.MaterialExpressionConstant,r=1.)
grey=node(u.MaterialExpressionDesaturation,luminance_factors=u.LinearColor(.2126,.7152,.0722,0))
connect(sample,'RGB',grey,'Input');connect(white,'',grey,'Fraction')
dim=node(u.MaterialExpressionMultiply,const_b=.32)
connect(grey,'',dim,'A')
reveal=node(u.MaterialExpressionIf)
indexed(clock,'',reveal,0);indexed(amount,'',reveal,1)
indexed(dim,'',reveal,2);indexed(dim,'',reveal,3);indexed(sample,'RGB',reveal,4)
assert lib.connect_material_property(reveal,'',u.MaterialProperty.MP_EMISSIVE_COLOR)
assert lib.connect_material_property(sample,'A',u.MaterialProperty.MP_OPACITY)
lib.recompile_material(mat)
u.EditorAssetLibrary.save_loaded_asset(mat)
u.log('R7 RADIAL COOLDOWN MATERIAL PASS')
