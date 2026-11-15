"""Create Riverlands hot chocolate and optional dressing materials.

Run from an Unreal Editor Python session with the project loaded. This script
creates/updates material assets and assigns the opaque drink material to the
SM_RiverWater mesh. Steam and marshmallows are card materials for separate
geometry/dressing; this script does not load or save a map.
"""

import unreal as u


MATERIAL_PATH = "/Game/World/Expansion/M_RiverWater"
MESH_PATH = "/Game/World/Expansion/SM_RiverWater"
STEAM_PATH = "/Game/World/Expansion/M_HotChocolateSteam"
MARSHMALLOW_PATH = "/Game/World/Expansion/M_Marshmallow"
EAL = u.EditorAssetLibrary
TOOLS = u.AssetToolsHelpers.get_asset_tools()
LIB = u.MaterialEditingLibrary


def load_or_create_material():
    if EAL.does_asset_exist(MATERIAL_PATH):
        mat = u.load_asset(MATERIAL_PATH)
        if not mat:
            raise RuntimeError("Could not load " + MATERIAL_PATH)
        return mat
    mat = TOOLS.create_asset(
        "M_RiverWater",
        "/Game/World/Expansion",
        u.Material,
        u.MaterialFactoryNew(),
    )
    if not mat:
        raise RuntimeError("Could not create " + MATERIAL_PATH)
    return mat


def expression(mat, cls, **properties):
    node = LIB.create_material_expression(mat, cls)
    if not node:
        raise RuntimeError("Could not create material expression " + cls.__name__)
    if properties:
        node.set_editor_properties(properties)
    return node


def connect(source, output, destination, pin):
    if not LIB.connect_material_expressions(source, output, destination, pin):
        raise RuntimeError("Material expression connection failed: " + pin)


def connect_property(source, output, prop):
    if not LIB.connect_material_property(source, output, prop):
        raise RuntimeError("Material property connection failed: " + str(prop))


def scalar(mat, value):
    return expression(mat, u.MaterialExpressionConstant, r=value)


def custom_expression(mat, code, input_names):
    node = expression(mat, u.MaterialExpressionCustom)
    inputs = []
    for name in input_names:
        custom_input = u.CustomInput()
        custom_input.set_editor_property("input_name", name)
        inputs.append(custom_input)
    node.set_editor_properties(
        dict(inputs=inputs, code=code, output_type=u.CustomMaterialOutputType.CMOT_FLOAT3)
    )
    return node


def custom_scalar(mat, code, input_names):
    node = expression(mat, u.MaterialExpressionCustom)
    inputs = []
    for name in input_names:
        custom_input = u.CustomInput()
        custom_input.set_editor_property("input_name", name)
        inputs.append(custom_input)
    node.set_editor_properties(
        dict(inputs=inputs, code=code, output_type=u.CustomMaterialOutputType.CMOT_FLOAT1)
    )
    return node


def load_or_create_named_material(path, name):
    if EAL.does_asset_exist(path):
        mat = u.load_asset(path)
        if not mat:
            raise RuntimeError("Could not load " + path)
        return mat
    mat = TOOLS.create_asset(name, "/Game/World/Expansion", u.Material, u.MaterialFactoryNew())
    if not mat:
        raise RuntimeError("Could not create " + path)
    return mat


def build_material(mat):
    LIB.delete_all_material_expressions(mat)
    mat.set_editor_properties(
        dict(
            blend_mode=u.BlendMode.BLEND_OPAQUE,
            shading_model=u.MaterialShadingModel.MSM_DEFAULT_LIT,
            two_sided=False,
            tangent_space_normal=False,
            used_with_instanced_static_meshes=False,
        )
    )

    world_position = expression(mat, u.MaterialExpressionWorldPosition)
    time = expression(mat, u.MaterialExpressionTime)

    color = custom_expression(
        mat,
        """float x = P.x * .001;
float y = P.y * .001;
float center = 19.0 + .5 * sin(y / 3.5);
float across = x - center;
float flow = y * 3.8 - T * .45;
float a = sin(flow + sin(across * 1.1 + T * .35) * 1.7);
float b = sin(flow * .57 - across * .75 + sin(y * .19 + T * .2));
float swirl = a * b;
float ribbon = pow(max(0, sin(across * 18.0 + flow * .62 + sin(y * .35) * 1.1)), 8);
float3 chocolate = float3(.105, .035, .014);
float3 cocoa = float3(.205, .078, .031);
float3 cream = float3(.34, .17, .075);
return lerp(chocolate, cocoa, .34 + .13 * swirl) + ribbon * cream * .43;""",
        ["P", "T"],
    )
    connect(world_position, "", color, "P")
    connect(time, "", color, "T")
    connect_property(color, "", u.MaterialProperty.MP_BASE_COLOR)

    normal = custom_expression(
        mat,
        """float x = P.x * .001;
float y = P.y * .001;
float flow = y * .72 - T * 1.35;
float nx = .09 * sin(flow + sin(x * 1.5 + T * .35));
float ny = .12 * cos(flow * .75 - x * .8 + sin(y * .19));
return normalize(float3(nx, ny, 1));""",
        ["P", "T"],
    )
    connect(world_position, "", normal, "P")
    connect(time, "", normal, "T")
    connect_property(normal, "", u.MaterialProperty.MP_NORMAL)

    roughness = scalar(mat, .22)
    connect_property(roughness, "", u.MaterialProperty.MP_ROUGHNESS)
    connect_property(scalar(mat, .55), "", u.MaterialProperty.MP_SPECULAR)

    LIB.recompile_material(mat)
    EAL.set_metadata_tag(mat, "Expansion.Authored", "HotChocolate.v1")
    if not EAL.save_loaded_asset(mat):
        raise RuntimeError("Could not save " + MATERIAL_PATH)


def build_marshmallow_material(mat):
    LIB.delete_all_material_expressions(mat)
    mat.set_editor_properties(
        dict(
            blend_mode=u.BlendMode.BLEND_OPAQUE,
            shading_model=u.MaterialShadingModel.MSM_DEFAULT_LIT,
            two_sided=False,
            used_with_instanced_static_meshes=True,
        )
    )
    # Apply to scaled sphere/capsule geometry for actual volume, not a painted surface mask.
    connect_property(expression(mat, u.MaterialExpressionConstant3Vector, constant=(.97, .91, .78)), "", u.MaterialProperty.MP_BASE_COLOR)
    connect_property(scalar(mat, .62), "", u.MaterialProperty.MP_ROUGHNESS)
    connect_property(scalar(mat, .32), "", u.MaterialProperty.MP_SPECULAR)
    # Move each whole object rigidly; per-vertex wrapping could split a mallow
    # when it reaches the loop boundary.
    world_position = expression(mat, u.MaterialExpressionObjectPositionWS)
    time = expression(mat, u.MaterialExpressionTime)
    drift = custom_expression(
        mat,
        """float oldY = P.y;
float newY = fmod(fmod(oldY + T * 45.0 + 11500.0, 23000.0) + 23000.0, 23000.0) - 11500.0;
// Offset the curve by lane, so nearby marshmallows do not follow identical paths.
float lane = P.x - (19000.0 + 500.0 * sin(oldY / 3500.0));
float phase = clamp(lane * 0.0012, -0.2, 0.2);
float oldCenter = sin(oldY / 3500.0 + phase);
float newCenter = sin(newY / 3500.0 + phase);
float dx = 500.0 * (newCenter - oldCenter);
float dz = 5.0 * sin(T * 2.0 + oldY * .001);
return float3(dx, newY - oldY, dz);""",
        ["P", "T"],
    )
    connect(world_position, "", drift, "P")
    connect(time, "", drift, "T")
    connect_property(drift, "", u.MaterialProperty.MP_WORLD_POSITION_OFFSET)
    LIB.recompile_material(mat)
    EAL.set_metadata_tag(mat, "Expansion.Authored", "MarshmallowCard.v1")
    if not EAL.save_loaded_asset(mat):
        raise RuntimeError("Could not save " + MARSHMALLOW_PATH)


def build_steam_material(mat):
    LIB.delete_all_material_expressions(mat)
    mat.set_editor_properties(
        dict(
            blend_mode=u.BlendMode.BLEND_TRANSLUCENT,
            shading_model=u.MaterialShadingModel.MSM_UNLIT,
            two_sided=True,
            used_with_instanced_static_meshes=True,
        )
    )
    uv = expression(mat, u.MaterialExpressionTextureCoordinate)
    time = expression(mat, u.MaterialExpressionTime)
    warped = custom_scalar(
        mat,
        """float2 p = UV;
float sway = sin(p.y * 8.0 + T * .55) * (.045 + p.y * .06);
p.x += sway;
float width = .08 + .12 * p.y;
float side = exp(-pow((p.x-.5)/width,2.0)*2.0);
float top = smoothstep(.0, .12, p.y) * (1.0 - smoothstep(.72, 1.0, p.y));
float pulse = .72 + .18 * sin(T * 1.3 + p.y * 7.0);
return saturate(side * top * pulse)*.085;""",
        ["UV", "T"],
    )
    connect(uv, "", warped, "UV")
    connect(time, "", warped, "T")
    color = expression(mat, u.MaterialExpressionConstant3Vector, constant=(.92, .88, .78))
    connect_property(color, "", u.MaterialProperty.MP_EMISSIVE_COLOR)
    connect_property(warped, "", u.MaterialProperty.MP_OPACITY)
    LIB.recompile_material(mat)
    EAL.set_metadata_tag(mat, "Expansion.Authored", "HotChocolateSteamCard.v1")
    if not EAL.save_loaded_asset(mat):
        raise RuntimeError("Could not save " + STEAM_PATH)


def assign_material():
    mesh = u.load_asset(MESH_PATH)
    if not mesh:
        raise RuntimeError("Missing Riverlands water mesh: " + MESH_PATH)
    slots = mesh.get_editor_property("static_materials")
    if not slots:
        raise RuntimeError("Riverlands water mesh has no material slots")
    mat = load_or_create_material()
    build_material(mat)
    mesh.set_material(0, mat)
    if mesh.get_editor_property("static_materials")[0].get_editor_property("material_interface") != mat:
        raise RuntimeError("River water material assignment did not persist on slot 0")
    if not EAL.save_loaded_asset(mesh):
        raise RuntimeError("Could not save " + MESH_PATH)
    marshmallow = load_or_create_named_material(MARSHMALLOW_PATH, "M_Marshmallow")
    build_marshmallow_material(marshmallow)
    steam = load_or_create_named_material(STEAM_PATH, "M_HotChocolateSteam")
    build_steam_material(steam)
    u.log("CANDY HOT CHOCOLATE MATERIAL PASS " + MATERIAL_PATH + " -> " + MESH_PATH)


if __name__ == '__main__':
    assign_material()
