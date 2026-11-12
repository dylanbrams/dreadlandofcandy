"""Creates /Game/FX/M_FxUnlit: translucent, unlit, two-sided, Color x vertex colour, Opacity x vertex alpha.
Run headless:  UnrealEditor-Cmd.exe <project> -run=pythonscript -script=<this file>
Used by the skill effects (T19), which build their shapes as procedural meshes and tint them per cast."""
import unreal

PATH, NAME = "/Game/FX", "M_FxUnlit"
lib = unreal.MaterialEditingLibrary
full = f"{PATH}/{NAME}"
if unreal.EditorAssetLibrary.does_asset_exist(full):
    unreal.EditorAssetLibrary.delete_asset(full)

mat = unreal.AssetToolsHelpers.get_asset_tools().create_asset(NAME, PATH, unreal.Material, unreal.MaterialFactoryNew())
mat.set_editor_property("blend_mode", unreal.BlendMode.BLEND_TRANSLUCENT)
mat.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
mat.set_editor_property("two_sided", True)

color = lib.create_material_expression(mat, unreal.MaterialExpressionVectorParameter, -600, -100)
color.set_editor_property("parameter_name", "Color")
color.set_editor_property("default_value", unreal.LinearColor(1.0, 0.55, 0.2, 1.0))
opacity = lib.create_material_expression(mat, unreal.MaterialExpressionScalarParameter, -600, 150)
opacity.set_editor_property("parameter_name", "Opacity")
opacity.set_editor_property("default_value", 0.6)
vcol = lib.create_material_expression(mat, unreal.MaterialExpressionVertexColor, -600, 300)

mul_rgb = lib.create_material_expression(mat, unreal.MaterialExpressionMultiply, -300, -50)
lib.connect_material_expressions(color, "", mul_rgb, "A")
lib.connect_material_expressions(vcol, "", mul_rgb, "B")
# A little overbright so the flash reads on the green field.
glow = lib.create_material_expression(mat, unreal.MaterialExpressionMultiply, -150, -50)
lib.connect_material_expressions(mul_rgb, "", glow, "A")
glow_amount = lib.create_material_expression(mat, unreal.MaterialExpressionConstant, -300, 50)
glow_amount.set_editor_property("r", 1.6)
lib.connect_material_expressions(glow_amount, "", glow, "B")
lib.connect_material_property(glow, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)

mul_a = lib.create_material_expression(mat, unreal.MaterialExpressionMultiply, -300, 200)
lib.connect_material_expressions(opacity, "", mul_a, "A")
lib.connect_material_expressions(vcol, "A", mul_a, "B")
lib.connect_material_property(mul_a, "", unreal.MaterialProperty.MP_OPACITY)

lib.recompile_material(mat)
unreal.EditorAssetLibrary.save_asset(full)
unreal.log(f"JamFx: created {full}")
