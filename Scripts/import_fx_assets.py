"""Import original FX sources and author renderer materials. Run in Unreal Python.

Only writes /Game/FX/Attacks. Reruns reuse existing assets; never deletes assets.
"""
import json
from pathlib import Path
import unreal

DEST = "/Game/FX/Attacks"
ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
SRC = ROOT / "ImportSource" / "FX"
ASSETS = unreal.EditorAssetLibrary
TOOLS = unreal.AssetToolsHelpers.get_asset_tools()
LIB = unreal.MaterialEditingLibrary


def prop(obj, name, value):
    obj.set_editor_property(name, value)


def import_file(path, subfolder):
    wanted = f"{DEST}/{subfolder}/{path.stem}"
    if ASSETS.does_asset_exist(wanted):
        return unreal.load_asset(wanted)
    task = unreal.AssetImportTask()
    task.set_editor_properties({"filename": str(path), "destination_path": f"{DEST}/{subfolder}",
                               "automated": True, "save": True, "replace_existing": False})
    if path.suffix == ".obj":
        options = unreal.FbxImportUI()
        options.set_editor_properties({"import_mesh": True, "import_as_skeletal": False,
            "import_materials": False, "import_textures": False})
        options.static_mesh_import_data.set_editor_properties({"auto_generate_collision": False,
            "combine_meshes": True, "import_uniform_scale": 1.0})
        task.set_editor_property("options", options)
    TOOLS.import_asset_tasks([task])
    paths = task.get_editor_property("imported_object_paths")
    matching = [unreal.load_asset(p) for p in paths]
    expected = unreal.StaticMesh if path.suffix == ".obj" else unreal.Texture2D
    found = [a for a in matching if isinstance(a, expected)]
    if len(found) != 1:
        raise RuntimeError(f"Expected one {expected} importing {path}: {paths}")
    return found[0]


def expr(mat, cls, x=0, y=0, **values):
    node = LIB.create_material_expression(mat, cls, x, y)
    node.set_editor_properties(values)
    return node


def connect(a, output, b, pin):
    if not LIB.connect_material_expressions(a, output, b, pin):
        raise RuntimeError(f"Could not connect material {pin}")


def material(name, mask=None, translucent=False, mesh=False, peppermint=False):
    full = f"{DEST}/Materials/{name}"
    if ASSETS.does_asset_exist(full):
        existing = unreal.load_asset(full)
        for node in LIB.get_material_expressions(existing):
            if isinstance(node, unreal.MaterialExpressionTextureSample) and node.texture and not node.texture.get_editor_property("srgb"):
                node.set_editor_property("sampler_type", unreal.MaterialSamplerType.SAMPLERTYPE_LINEAR_COLOR)
        LIB.recompile_material(existing)
        ASSETS.save_loaded_asset(existing)
        return existing
    mat = TOOLS.create_asset(name, f"{DEST}/Materials", unreal.Material, unreal.MaterialFactoryNew())
    mat.set_editor_properties({"two_sided": True, "shading_model": unreal.MaterialShadingModel.MSM_UNLIT,
                              "blend_mode": unreal.BlendMode.BLEND_TRANSLUCENT if translucent else unreal.BlendMode.BLEND_OPAQUE})
    prop(mat, "used_with_niagara_mesh_particles" if mesh else "used_with_niagara_sprites", True)
    pc = expr(mat, unreal.MaterialExpressionParticleColor, -600, 0)
    tint = expr(mat, unreal.MaterialExpressionVectorParameter, -600, 180,
                parameter_name="Tint", default_value=unreal.LinearColor(1, 1, 1, 1))
    mul = expr(mat, unreal.MaterialExpressionMultiply, -350, 0)
    connect(pc, "RGB", mul, "A")
    connect(tint, "RGB", mul, "B")
    color = mul
    if mask:
        sample = expr(mat, unreal.MaterialExpressionTextureSample, -600, -200, texture=mask)
        if not mask.get_editor_property("srgb"):
            sample.set_editor_property("sampler_type", unreal.MaterialSamplerType.SAMPLERTYPE_LINEAR_COLOR)
        if peppermint:
            detail = expr(mat, unreal.MaterialExpressionMultiply, -150, 0)
            connect(sample, "RGB", detail, "A")
            connect(color, "", detail, "B")
            color = detail
        if translucent:
            alpha = expr(mat, unreal.MaterialExpressionMultiply, -150, 200)
            connect(pc, "A", alpha, "A")
            connect(sample, "A", alpha, "B")
            LIB.connect_material_property(alpha, "", unreal.MaterialProperty.MP_OPACITY)
    elif translucent:
        opacity = expr(mat, unreal.MaterialExpressionScalarParameter, -300, 250,
                       parameter_name="Opacity", default_value=0.18)
        alpha = expr(mat, unreal.MaterialExpressionMultiply, -100, 250)
        connect(pc, "A", alpha, "A")
        connect(opacity, "", alpha, "B")
        LIB.connect_material_property(alpha, "", unreal.MaterialProperty.MP_OPACITY)
    LIB.connect_material_property(color, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    LIB.recompile_material(mat)
    ASSETS.save_loaded_asset(mat)
    ASSETS.set_metadata_tag(mat, "FX.Source", "Original procedural attack FX; Scripts/import_fx_assets.py")
    return mat


def main():
    masks = {p.stem: import_file(p, "Textures") for p in sorted(SRC.glob("*.png"))}
    for texture in masks.values():
        prop(texture, "srgb", texture.get_name() == "T_FX_Peppermint")
        prop(texture, "address_x", unreal.TextureAddress.TA_CLAMP)
        prop(texture, "address_y", unreal.TextureAddress.TA_CLAMP)
        ASSETS.save_loaded_asset(texture)
    mats = {
        "mesh": material("M_FX_Mesh", mesh=True),
        "soft": material("M_FX_Particle", masks["T_FX_SoftDisc"], True),
        "bubble": material("M_FX_Bubble", masks["T_FX_Bubble"], True),
        "star": material("M_FX_Star", masks["T_FX_Star"], True),
        "rim": material("M_FX_Rim", translucent=True, mesh=True),
        "candy": material("M_FX_Peppermint", masks["T_FX_Peppermint"], mesh=True, peppermint=True),
    }
    mesh_editor = unreal.get_default_object(unreal.StaticMeshEditorSubsystem)
    records = []
    for p in sorted(SRC.glob("*.obj")):
        mesh = import_file(p, "Meshes")
        mesh_editor.remove_collisions(mesh)
        key = "candy" if "Peppermint" in p.stem else "rim" if any(s in p.stem for s in ("Ring", "Arc", "Dome", "Strand")) else "mesh"
        mesh.set_material(0, mats[key])
        ASSETS.set_metadata_tag(mesh, "FX.Source", "Original procedural geometry; Scripts/build_fx_sources.py")
        ASSETS.save_loaded_asset(mesh)
        records.append({"source": p.name, "asset": mesh.get_path_name(), "material": mats[key].get_path_name()})
    report = {"meshes": records, "textures": [a.get_path_name() for a in masks.values()],
              "materials": [a.get_path_name() for a in mats.values()], "visual_review": "pending"}
    (SRC / "unreal_assets.json").write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    unreal.log("FX ASSETS PASS: 10 meshes, 4 textures, 6 materials")


if __name__ == "__main__":
    main()
