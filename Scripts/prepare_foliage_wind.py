"""Prepare a small, reversible foliage wind pass in the Unreal editor.

Default is inspection only. Set JAM_FOLIAGE_WIND_APPLY=1 to modify assets,
after reviewing the printed preflight and obtaining exclusive engine access.
The pass targets the existing Polish broadleaf trees and spun-sugar bush;
it does not alter collision, grass, pines, or the expansion foliage.
"""
import os
import unreal as u

ROOT = '/Game/World/Polish'
MATERIALS = {
    'M_PeppermintBirch_0': 300.0,
    'M_BubblegumBlossom_0': 300.0,
    'M_SpunSugar_0': 120.0,
    'M_SpunSugar_1': 120.0,
}
MESHES = ([f'SM_PeppermintBirch_Warp{i}' for i in range(1, 4)]
          + [f'SM_BubblegumBlossom_Warp{i}' for i in range(1, 4)]
          + ['SM_SpunSugarThicket'])
APPLY = os.environ.get('JAM_FOLIAGE_WIND_APPLY') == '1'
LIB = u.MaterialEditingLibrary
EAL = u.EditorAssetLibrary


def require(path):
    asset = u.load_asset(path)
    if not asset:
        raise RuntimeError('Missing asset: ' + path)
    return asset


def inspect():
    materials = {name: require(f'{ROOT}/Materials/{name}') for name in MATERIALS}
    meshes = {name: require(f'{ROOT}/Meshes/{name}') for name in MESHES}
    # Never overwrite an existing WPO graph, including one from another session.
    get_input = getattr(LIB, 'get_material_property_input_node', None)
    if not get_input:
        raise RuntimeError('Cannot inspect existing WPO with this editor API; stop for manual review')
    for name, mat in materials.items():
        existing = get_input(mat, u.MaterialProperty.MP_WORLD_POSITION_OFFSET)
        if existing:
            raise RuntimeError(f'{name} already has WPO: {existing}; no assets changed')
    for name, mesh in meshes.items():
        box = mesh.get_bounding_box()
        print(f'WIND PREFLIGHT mesh={name} bounds={box} '
              f'positive={mesh.get_editor_property("positive_bounds_extension")} '
              f'negative={mesh.get_editor_property("negative_bounds_extension")}')
    print('WIND PREFLIGHT materials=' + ', '.join(materials))
    return materials, meshes


def add_wind(mat, height):
    # World -> per-instance coordinates provides a stable, height-based root
    # mask for HISMs. These meshes were authored with their roots at local Z=0.
    world = LIB.create_material_expression(mat, u.MaterialExpressionWorldPosition, -900, 300)
    local = LIB.create_material_expression(mat, u.MaterialExpressionTransformPosition, -700, 300)
    local.set_editor_property('transform_source_type',
                              u.MaterialPositionTransformSource.TRANSFORMPOSSOURCE_WORLD)
    local.set_editor_property('transform_type',
                              u.MaterialPositionTransformSource.TRANSFORMPOSSOURCE_INSTANCE)
    if not LIB.connect_material_expressions(world, '', local, ''):
        raise RuntimeError('Could not connect world position to transform')
    clock = LIB.create_material_expression(mat, u.MaterialExpressionTime, -700, 500)
    variation = LIB.create_material_expression(mat, u.MaterialExpressionPerInstanceRandom, -700, 650)
    wind = LIB.create_material_expression(mat, u.MaterialExpressionCustom, -400, 300)
    inputs = []
    for name in ('P', 'T', 'R'):
        item = u.CustomInput()
        item.set_editor_property('input_name', name)
        inputs.append(item)
    wind.set_editor_property('inputs', inputs)
    wind.set_editor_property('output_type', u.CustomMaterialOutputType.CMOT_FLOAT3)
    wind.set_editor_property('description', 'Feedback6 E5 anchored foliage wind')
    wind.set_editor_property('code',
        f'float mask=saturate((P.z-8.0)/{height:.1f}); '
        'mask*=mask; float phase=T*0.65+R*6.283185; '
        'return float3(8.0,3.0,0.0)*sin(phase)*mask;')
    for source, pin in ((local, 'P'), (clock, 'T'), (variation, 'R')):
        if not LIB.connect_material_expressions(source, '', wind, pin):
            raise RuntimeError('Could not connect wind input ' + pin)
    if not LIB.connect_material_property(wind, '', u.MaterialProperty.MP_WORLD_POSITION_OFFSET):
        raise RuntimeError('Could not connect WPO')
    LIB.recompile_material(mat)


materials, meshes = inspect()
if not APPLY:
    print('WIND PREFLIGHT ONLY: no assets modified')
else:
    # 60 local units each way accommodates the <=8.6 world-unit offset even
    # for small instances, subject to checking the actual instance scales.
    for mesh in meshes.values():
        for prop in ('positive_bounds_extension', 'negative_bounds_extension'):
            old = mesh.get_editor_property(prop)
            mesh.set_editor_property(prop, u.Vector(max(old.x, 60), max(old.y, 60), max(old.z, 10)))
    for name, mat in materials.items():
        add_wind(mat, MATERIALS[name])
    for asset in list(meshes.values()) + list(materials.values()):
        if not EAL.save_loaded_asset(asset):
            raise RuntimeError('Failed to save ' + asset.get_path_name())
    print('WIND APPLIED: inspect roots, bounds and near/far motion before shipping')
