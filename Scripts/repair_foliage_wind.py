"""Repair the missing transform input from the first E5 wind application."""
import unreal as u

lib = u.MaterialEditingLibrary
for name in ('M_PeppermintBirch_0', 'M_BubblegumBlossom_0', 'M_SpunSugar_0', 'M_SpunSugar_1'):
    mat = u.load_asset('/Game/World/Polish/Materials/' + name)
    expressions = lib.get_material_expressions(mat)
    transforms = [x for x in expressions if isinstance(x, u.MaterialExpressionTransformPosition)]
    assert len(transforms) == 1, (name, len(transforms))
    transform = transforms[0]
    print('WIND REPAIR', name, 'input names', lib.get_material_expression_input_names(transform))
    world = lib.create_material_expression(mat, u.MaterialExpressionWorldPosition, -1000, 400)
    assert lib.connect_material_expressions(world, '', transform, ''), name
    lib.recompile_material(mat)
    assert u.EditorAssetLibrary.save_loaded_asset(mat), name
print('WIND REPAIR complete')
