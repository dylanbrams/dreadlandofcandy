"""Locally decimate approved NPC GLBs for Meshy humanoid rigging.

Run with Blender 3.3: blender.exe -b --python-exit-code 1 -P Scripts/prepare_npc_rigs.py
The source GLBs are read-only; outputs go under ImportSource/Meshy/NpcRigInputs/.
"""
from pathlib import Path
import bpy

ROOT = Path(__file__).resolve().parent.parent
OUT = ROOT / "ImportSource" / "Meshy" / "NpcRigInputs"
SOURCES = {
    "Villager": ROOT / "ImportSource" / "Hyper3D" / "SM_Villager.glb",
    "Goblin": ROOT / "ImportSource" / "Hyper3D" / "SM_Goblin.glb",
    "Rat": ROOT / "ImportSource" / "Hyper3D" / "SM_Rat_Rodin.glb",
}
TARGET_TRIANGLES = 25000


def triangle_count(objects):
    total = 0
    for obj in objects:
        if obj.type != "MESH":
            continue
        obj.data.calc_loop_triangles()
        total += len(obj.data.loop_triangles)
    return total


def prepare(name, source):
    if not source.is_file():
        raise FileNotFoundError(source)
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete(use_global=False)
    bpy.ops.import_scene.gltf(filepath=str(source))
    meshes = [obj for obj in bpy.context.scene.objects if obj.type == "MESH"]
    if not meshes:
        raise RuntimeError(f"{name}: GLB contains no mesh objects")
    before = triangle_count(meshes)
    ratio = min(1.0, TARGET_TRIANGLES / max(1, before))
    for obj in meshes:
        mod = obj.modifiers.new("RigInputDecimate", "DECIMATE")
        mod.decimate_type = "COLLAPSE"
        mod.ratio = ratio
        mod.use_collapse_triangulate = True
        bpy.context.view_layer.objects.active = obj
        bpy.ops.object.modifier_apply(modifier=mod.name)
    meshes = [obj for obj in bpy.context.scene.objects if obj.type == "MESH"]
    after = triangle_count(meshes)
    OUT.mkdir(parents=True, exist_ok=True)
    dest = OUT / f"{name}.glb"
    bpy.ops.export_scene.gltf(
        filepath=str(dest), export_format="GLB", export_image_format="AUTO",
        export_apply=True, export_texcoords=True, export_normals=True,
        export_materials="EXPORT", export_animations=False,
    )
    print(f"NPC_RIG_INPUT {name} source={source.relative_to(ROOT)} before_triangles={before} "
          f"after_triangles={after} target={TARGET_TRIANGLES} output={dest.relative_to(ROOT)} "
          f"bytes={dest.stat().st_size}")


def main():
    for name, source in SOURCES.items():
        prepare(name, source)


if __name__ == "__main__":
    main()
