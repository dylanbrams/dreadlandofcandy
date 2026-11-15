"""Fixes5 #1: creates /Game/FX/M_ComicCard: translucent, unlit, two-sided card for the flying comic action cards.
Card (texture parameter): RGB -> emissive, alpha x Fade (scalar) -> opacity. The authored colours are kept, not tinted.
Run headless:  UnrealEditor-Cmd.exe <project> -run=pythonscript -script=<this file>"""
import unreal

PATH, NAME = "/Game/FX", "M_ComicCard"
lib = unreal.MaterialEditingLibrary
full = f"{PATH}/{NAME}"
if unreal.EditorAssetLibrary.does_asset_exist(full):
    unreal.EditorAssetLibrary.delete_asset(full)

mat = unreal.AssetToolsHelpers.get_asset_tools().create_asset(NAME, PATH, unreal.Material, unreal.MaterialFactoryNew())
mat.set_editor_property("blend_mode", unreal.BlendMode.BLEND_TRANSLUCENT)
mat.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
mat.set_editor_property("two_sided", True)
# Crisp over the depth-of-field blur, and not greyed by height fog.
mat.set_editor_property("translucency_pass", unreal.MaterialTranslucencyPass.MTP_AFTER_DOF)
mat.set_editor_property("allow_negative_emissive_color", False)

card = lib.create_material_expression(mat, unreal.MaterialExpressionTextureSampleParameter2D, -600, 0)
card.set_editor_property("parameter_name", "Card")
card.set_editor_property("texture", unreal.load_asset("/Game/Art/UI/ComicCards/T_Comic_Manners_en"))
fade = lib.create_material_expression(mat, unreal.MaterialExpressionScalarParameter, -600, 250)
fade.set_editor_property("parameter_name", "Fade")
fade.set_editor_property("default_value", 1.0)

# A little overbright so the ivory reads as white after exposure.
bright = lib.create_material_expression(mat, unreal.MaterialExpressionMultiply, -300, -50)
lib.connect_material_expressions(card, "RGB", bright, "A")
gain = lib.create_material_expression(mat, unreal.MaterialExpressionConstant, -450, 50)
gain.set_editor_property("r", 1.8)
lib.connect_material_expressions(gain, "", bright, "B")
lib.connect_material_property(bright, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
mul_a = lib.create_material_expression(mat, unreal.MaterialExpressionMultiply, -300, 150)
lib.connect_material_expressions(card, "A", mul_a, "A")
lib.connect_material_expressions(fade, "", mul_a, "B")
lib.connect_material_property(mul_a, "", unreal.MaterialProperty.MP_OPACITY)

lib.recompile_material(mat)
unreal.EditorAssetLibrary.save_asset(full)
unreal.log(f"ComicCards: created {full}")
