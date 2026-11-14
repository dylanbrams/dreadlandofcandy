"""UE 5.8 Niagara asset authoring, isolated from slice gameplay.

Run after build_fx_sources.py and import_fx_assets.py in Unreal Python.
After this process exits, run recompile_fx_systems.py in a fresh commandlet with
-dpcvars=fx.ForceCompileOnLoad=1. A successful in-memory audit is not sufficient.
Uses the installed NiagaraToolsets registry for typed parameter values.
Writes only /Game/FX/Attacks and ImportSource/FX/niagara_assets.json.
"""
import json
from pathlib import Path
import unreal

BASE = "/Game/FX/Attacks"
API = unreal.get_default_object(unreal.NiagaraToolset_System)
EAL = unreal.EditorAssetLibrary
TOOLS = unreal.AssetToolsHelpers.get_asset_tools()


def call(name, *args):
    return API.call_method(name, args)


def refpath(path):
    return {"refPath": path}


def tool(name, **args):
    result = unreal.ToolsetRegistry.execute_tool("NiagaraToolsets.NiagaraToolset_System", name, json.dumps(args))
    if not result.is_complete:
        raise RuntimeError(f"Unexpected asynchronous edit: {name}")
    if result.error:
        raise RuntimeError(f"{name}: {result.error}")
    value = json.loads(result.value)
    return value.get("returnValue", value)


def ref(system, emitter="", script="", module="", inputs=None, renderer=-1):
    return {"system": refpath(system.get_path_name()), "emitterName": emitter,
            "scriptName": script, "moduleName": module, "inputNameStack": inputs or [], "rendererIndex": renderer}


def native_ref(system, emitter="", script="", module="", renderer=-1):
    value = unreal.NiagaraExt_StackItemReference()
    value.set_editor_properties({"System": system, "EmitterName": emitter, "ScriptName": script,
                                 "ModuleName": module, "RendererIndex": renderer})
    return value


TYPES = {"float": "/Script/Niagara.NiagaraFloat", "int": "/Script/Niagara.NiagaraInt32",
         "bool": "/Script/Niagara.NiagaraBool", "color": "/Script/CoreUObject.LinearColor",
         "vec": "/Script/CoreUObject.Vector3f", "vec2": "/Script/CoreUObject.Vector2f",
         "position": "/Script/Niagara.NiagaraPosition", "quat": "/Script/CoreUObject.Quat4f"}


def typedef(kind):
    return {"classStructOrEnum": refpath(TYPES[kind])}


def value(kind, data):
    if kind in ("float", "int", "bool"):
        fields = {"value": data}
    else:
        fields = dict(zip("rgba" if kind == "color" else "xyzw", data))
    return {"struct": refpath(TYPES[kind]), "value": fields}


def hlsl(expression):
    return {"struct": refpath("/Script/NiagaraEditor.NiagaraExt_StackInputData_HlslExpression"),
            "value": {"hlslExpression": expression}}


def set_input(system, emitter, script, module, name, data):
    return tool("SetStackInputData", stackInputRef=ref(system, emitter, script, module, [name]), inputData=data)


def assign(system, emitter, script, entries):
    """One SetParameters module; each entry is (name, type, expression)."""
    params = [{"variable": {"name": name, "type": typedef(kind)},
               "defaultValue": value(kind, 0 if kind in ("float", "int", "bool") else (0, 0, 0, 0) if kind in ("color", "quat") else (0, 0) if kind == "vec2" else (0, 0, 0))}
              for name, kind, expression in entries]
    mod = tool("AddSetParametersModule", moduleLocationRef=ref(system, emitter, script), parameters=params)
    module_name = mod["moduleName"]
    for name, kind, expression in entries:
        set_input(system, emitter, script, module_name, name,
                  value(kind, expression) if isinstance(expression, (int, float)) else hlsl(expression))


def add_users(system, color, duration, count, radius=300):
    specs = [("PrimaryColor", "color", color), ("SecondaryColor", "color", (1, 0.9, 0.65, 1)),
             ("Duration", "float", duration), ("PropCount", "int", count), ("Radius", "float", radius),
             ("Length", "float", 800), ("HalfAngleDeg", "float", 35), ("HalfWidth", "float", 100),
             ("Intensity", "float", 1), ("Seed", "int", 2026), ("Stop", "bool", 0), ("BudgetOut", "bool", 0)]
    tool("AddUserVariables", system=refpath(system.get_path_name()), variablesToAdd=[
        {"name": "User." + n, "type": typedef(t), "defaultValue": value(t, v),
         "description": "Attack cosmetic parameter; never controls gameplay"} for n, t, v in specs])


def emitter(system, name, recipe, duration):
    template = unreal.load_asset("/Niagara/DefaultAssets/Templates/Emitters/SimpleSpriteBurst")
    call("AddEmitter", system, template, name)
    topology = call("GetEmitterTopology", native_ref(system, name))
    for stage in ("EmitterUpdateScript", "ParticleSpawnScript", "ParticleUpdateScript"):
        stack = topology.get_editor_property(stage)
        for mod in stack.get_editor_property("Modules"):
            module_name = str(mod.get_editor_property("ModuleName"))
            keep = {"EmitterState", "SpawnBurst_Instantaneous", "InitializeParticle", "ParticleState"}
            if module_name not in keep:
                call("RemoveModule", native_ref(system, name, stage, module_name))
    data = call("GetEmitterData", native_ref(system, name))
    data.set_editor_property("PropertyValues", json.dumps({"bLocalSpace": True, "SimTarget": "CPUSim", "InterpolatedSpawnMode": "RunUpdateScript"}))
    call("SetEmitterData", native_ref(system, name), data)
    # The shared system is a finite one-shot. User.Stop also clears retained particles.
    count_expr = recipe.get("count", f"min({recipe.get('cap', 128)}, max(0, User.PropCount))*{recipe.get('density', 1)}")
    spawn_count = count_expr if "count" in recipe else f"(User.PropCount > 0 && User.Intensity > 0.0 ? max(1, int(({count_expr}) * max(0.0, min(1.0, User.Intensity)))) : 0)"
    set_input(system, name, "EmitterUpdateScript", "SpawnBurst_Instantaneous", "Spawn Count",
              hlsl(f"int({spawn_count})"))
    # Bake the lifetime from the authored row. User.Duration still drives fade;
    # the renderer-side prototype has a hard cap and cannot outlive its preset.
    spawn = [("Particles.Lifetime", "float", max(0.1, duration)),
             ("Particles.FXPhase", "float", "float(Particles.UniqueID) * 2.399963 + float(User.Seed) * 0.01"),
             ("Particles.FXIndex", "float", "float(Particles.UniqueID)"),
             ("Particles.MeshOrientation", "quat", "float4(0.0,0.0,0.0,1.0)")]
    assign(system, name, "ParticleSpawnScript", spawn)
    phase, age = "Particles.FXPhase", "Particles.Age"
    t = "min(1.0, Particles.Age / max(0.1, User.Duration))"
    kind = recipe["kind"]
    mesh = recipe.get("mesh")
    size = recipe.get("size", 40)
    scale = recipe.get("scale", 1)
    if kind == "impact":
        pos = f"float3(cos({phase})*{age}*240.0, sin({phase})*{age}*240.0, 20.0+{age}*140.0-{age}*{age}*180.0)"
    elif kind == "cone":
        spread = "sin(Particles.FXPhase) * User.HalfAngleDeg * 0.017453293"
        pos = f"float3(cos({spread}),sin({spread}),0.0)*User.Length*{t}+float3(0.0,0.0,70.0)"
    elif kind == "ring":
        pos = "float3(0.0,0.0,5.0)"
        scale = f"User.Radius*0.01*(0.7+0.3*{t})"
    elif kind == "rain":
        # delay/spread/height: a late wave of big ones drops after the small ones (Fixes3 #2).
        fall = f"max(0.0, {age}-{recipe.get('delay', 0.0)}-frac(Particles.FXIndex*0.618)*{recipe.get('spread', 0.55)})"
        reach = f"User.Radius*{recipe.get('reach', 1.0)}"
        pos = f"float3(cos({phase})*{reach}*sqrt(frac(Particles.FXIndex*0.754+0.1)),sin({phase})*{reach}*sqrt(frac(Particles.FXIndex*0.754+0.1)),max(0.0,{recipe.get('height', 650.0)}-{fall}*{fall}*1100.0))"
    elif kind == "arrows":
        # Fixes3 #2, archer identity: celery arrows loosed from the bow, arcing into the area ahead, nose first.
        u = f"clamp(({age}-frac(Particles.FXIndex*0.618)*{recipe.get('spread', 0.6)})/{recipe.get('flight', 0.7)},0.0,1.0)"
        land_x = f"User.Length*(0.45+0.55*frac(Particles.FXIndex*0.754+0.1))"
        land_y = f"sin({phase})*User.Radius*0.8"
        pos = f"float3({land_x}*{u},{land_y}*{u},120.0*(1.0-{u})+{recipe.get('arc', 420.0)}*sin({u}*3.141593))"
        pitch = f"atan2({land_x},{recipe.get('arc', 420.0)}*3.141593*cos({u}*3.141593)-120.0)"
    elif kind == "orbit":
        angle = f"{phase}+{age}*9.0"
        pos = f"float3(cos({angle})*User.Radius*0.75,sin({angle})*User.Radius*0.75,80.0+sin({phase}*1.7)*40.0)"
    elif kind == "dust":
        angle = f"{phase}+{age}*5.0"
        pos = f"float3(cos({angle})*User.Radius*frac(Particles.FXIndex*0.618+0.2),sin({angle})*User.Radius*frac(Particles.FXIndex*0.618+0.2),30.0+frac(Particles.FXIndex*0.754+{age}*0.2)*200.0)"
    elif kind == "foam":
        # A finite burst distributed along a moving phase gives an inexpensive stream blockout.
        travel = f"frac(Particles.FXIndex*0.618+{age}*1.6)"
        pos = f"float3({travel}*User.Length, sin({phase})*{travel}*User.Length*0.3,80.0+cos({phase})*{travel}*User.Length*0.25)"
    elif kind == "dome":
        pos = "float3(0.0,0.0,0.0)"
        scale = "User.Radius*0.01"
    elif kind == "strand":
        pos = "float3(0.0,0.0,30.0+Particles.FXIndex*9.0)"
        scale = "1.0"
    elif kind == "volley":
        # Fanned volley: each prop takes its own lane and arc, so a cap of 12+ reads as a spray.
        lane = f"sin({phase})*{recipe.get('fan', 35.0)}*sin({t}*3.141593)"
        lift = f"80.0+{recipe.get('arc', 0.0)}*sin({t}*3.141593)*frac(Particles.FXIndex*0.754+0.3)"
        pos = f"float3(User.Length*{t}*(0.8+0.2*frac(Particles.FXIndex*0.31)),{lane},{lift})"
    else:
        raise ValueError(kind)
    base_color = recipe.get("color", "User.PrimaryColor.rgb")
    rgb = f"(User.BudgetOut ? float3(0.42,0.42,0.42) : {base_color})"
    alpha = f"(User.Stop ? 0.0 : (1.0-{t})*User.PrimaryColor.a)"
    updates = [("Particles.Position", "position", pos),
               ("Particles.Color", "color", f"float4({rgb},{alpha})"),
               ("Particles.SpriteSize", "vec2", f"float2({size},{size})*(0.5+{t})"),
               ("Particles.Scale", "vec", f"float3(1.0,1.0,1.0)*({scale})"),
               ("Particles.Alive", "bool", "Particles.Alive && !User.Stop")]
    if kind == "strand":
        updates[-2] = ("Particles.Scale", "vec", "float3(User.Length*0.01,1.0,1.0)")
    if kind == "orbit":
        updates.append(("Particles.MeshOrientation", "quat", f"float4(0.0,0.0,sin(({angle})*0.5),cos(({angle})*0.5))"))
    elif kind == "rain":
        updates.append(("Particles.MeshOrientation", "quat", "float4(0.0,0.70710678,0.0,0.70710678)"))
    elif kind == "arrows":
        updates.append(("Particles.MeshOrientation", "quat", f"float4(0.0,sin(({pitch})*0.5),0.0,cos(({pitch})*0.5))"))
    assign(system, name, "ParticleUpdateScript", updates)
    for rend in topology.get_editor_property("Renderers"):
        r = native_ref(system, name, renderer=rend.get_editor_property("RendererIndex"))
        call("RemoveRenderer", r)
    rend_cls = unreal.load_class(None, "/Script/Niagara.NiagaraMeshRendererProperties" if mesh else "/Script/Niagara.NiagaraSpriteRendererProperties")
    renderer = call("AddRenderer", native_ref(system, name), rend_cls)
    r = ref(system, name, renderer=renderer.get_editor_property("RendererIndex"))
    if mesh:
        props = {"Meshes": [{"Mesh": refpath(f"{BASE}/Meshes/{mesh}.{mesh}")}], "bCastShadows": False}
    else:
        material = recipe.get("material", "M_FX_Particle")
        props = {"Material": refpath(f"{BASE}/Materials/{material}.{material}"), "bCastShadows": False,
                 "FacingMode": "FaceCamera", "Alignment": "Unaligned"}
    tool("SetRendererData", renderer=r, rendererData={"propertyValues": json.dumps(props)})


GOLD = (0.35, 0.55, 1, 1)
WARM = (1, 0.25, 0.06, 1)
GREEN = (0.3, 0.9, 0.12, 1)
MINT = (0.16, 0.9, 0.8, 0.8)
CREAM = (1, 0.9, 0.7, 0.65)
RING = {"kind": "ring", "mesh": "SM_FX_Ring", "count": "1"}
CONE = {"kind": "cone", "size": 70}
CARDS = {"kind": "cone", "mesh": "SM_FX_Card", "cap": 6, "scale": 1.2}
SUGAR = {"kind": "volley", "mesh": "SM_FX_SugarCube", "cap": 6, "scale": 1.5}
CANDY = {"kind": "rain", "mesh": "SM_FX_Peppermint", "cap": 12, "scale": 2.4}
CELERY = {"kind": "rain", "mesh": "SM_FX_Celery", "cap": 16, "scale": 1.2}
PINS = {"kind": "orbit", "mesh": "SM_FX_Pin", "cap": 20, "color": "float3(0.65,0.32,0.12)"}
FLOUR = {"kind": "dust", "size": 100, "cap": 32, "density": 4, "color": "float3(1.0,0.92,0.78)"}
FOAM = {"kind": "foam", "size": 100, "cap": 64, "color": "float3(0.9,1.0,1.0)"}
BUBBLES = {"kind": "foam", "size": 60, "cap": 32, "material": "M_FX_Bubble"}
DOME = {"kind": "dome", "mesh": "SM_FX_Dome", "count": "1"}
STRANDS = {"kind": "strand", "mesh": "SM_FX_Strand", "cap": 6}
IMPACT = {"kind": "impact", "size": 35}

# Fixes3 #2 signature layers: lots more of each class's own props.
CANDY_SHOWER = dict(CANDY, cap=220, spread=1.0, reach=1.15)
CANDY_BIG = dict(CANDY, cap=5, scale=9.0, delay=0.9, spread=0.35, height=1500.0, reach=0.6)
ARROW_VOLLEY = {"kind": "arrows", "mesh": "SM_FX_Celery", "cap": 48, "scale": 1.1, "spread": 0.7, "flight": 0.75, "arc": 420.0}
CELERY_SPRAY = dict(SUGAR, mesh="SM_FX_Celery", cap=14, scale=1.8, fan=160.0, arc=220.0)
SUGAR_SPRAY = dict(SUGAR, cap=36, scale=1.6, fan=140.0, arc=180.0)
FLOUR_CLOUD = dict(FLOUR, size=120, cap=40, density=2)  # 220/64/4 whited out the camera
CARD_STORM = dict(CARDS, cap=40)
CARD_RAIN = {"kind": "rain", "mesh": "SM_FX_Card", "cap": 120, "scale": 1.6, "spread": 1.0}
PIN_STORM = dict(PINS, cap=48)

RECIPES = [
    ("NS_FX_Impact", CREAM, 0.55, 16, [IMPACT]),
    ("NS_FX_LectureCone", GOLD, 0.6, 16, [CONE]),
    ("NS_FX_ComicCards", WARM, 0.65, 4, [CARDS]),
    ("NS_FX_ProjectileVolley", CREAM, 0.35, 6, [SUGAR]),
    ("NS_FX_ProjectileRain", GREEN, 1.6, 10, [CELERY, RING]),
    ("NS_FX_OrbitProps", CREAM, 2.3, 8, [PINS, FLOUR]),
    ("NS_FX_SoapTorrent", MINT, 1.2, 48, [FOAM, BUBBLES]),
    ("NS_FX_ShieldDome", GOLD, 3.5, 1, [DOME, RING]),
    ("NS_FX_FlossTether", (1, 1, 1, 1), 2.5, 4, [STRANDS]),
    ("NS_K01_PoliteShield", GOLD, 0.8, 10, [dict(RING, mesh="SM_FX_Arc"), IMPACT, CARD_STORM]),
    ("NS_K02_SternInstruction", GOLD, 0.65, 16, [CONE, CARD_STORM]),
    ("NS_K03_GoodManners", GOLD, 3.5, 1, [DOME, RING, CARD_RAIN]),
    ("NS_B01_FuriousKneading", CREAM, 1.2, 12, [IMPACT, FLOUR_CLOUD]),
    ("NS_B02_RollingPinCyclone", CREAM, 2.3, 8, [PIN_STORM, FLOUR_CLOUD]),
    ("NS_B03_AggressiveEncouragement", WARM, 1.0, 20, [CONE, CARD_STORM, FLOUR_CLOUD]),
    ("NS_M01_SpoonfulOfSugar", (1, 0.93, 0.6, 1), 0.5, 36, [SUGAR_SPRAY, IMPACT]),
    ("NS_M02_PeppermintPopfall", (1, 1, 1, 1), 2.6, 220, [CANDY_SHOWER, CANDY_BIG, RING]),
    ("NS_M03_ArcaneSanitization", MINT, 1.2, 48, [FOAM, BUBBLES]),
    ("NS_A01_CeleryShot", GREEN, 0.5, 14, [CELERY_SPRAY, IMPACT]),
    ("NS_A02_CeleryVolley", GREEN, 1.8, 48, [ARROW_VOLLEY, RING]),
    ("NS_A03_FlossSnare", (1, 1, 1, 1), 2.5, 4, [STRANDS]),
]

# One primitive per system. A cast owns a small group of system components,
# represented explicitly in the assembly manifest, rather than hiding its layers.
ASSEMBLIES = {name: [name] + [f"{name}_Layer{i}" for i in range(1, len(parts))]
              for name, color, duration, count, parts in RECIPES}
RECIPES = [(layer_name, color, duration, count, [part])
           for name, color, duration, count, parts in RECIPES
           for layer_name, part in zip(ASSEMBLIES[name], parts)]


def main():
    root = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
    filter_file = root / "Saved/FX/build_filter.txt"
    selected = filter_file.read_text(encoding="utf-8-sig").strip() if filter_file.exists() else ""
    manifest_file = root / "ImportSource/FX/niagara_assets.json"
    records = json.loads(manifest_file.read_text(encoding="utf-8")) if selected and manifest_file.exists() else []
    for name, color, duration, count, parts in RECIPES:
        if selected and name != selected:
            continue
        path = f"{BASE}/Systems/{name}"
        system = unreal.load_asset(path) if EAL.does_asset_exist(path) else call("CreateNiagaraSystem", name, f"{BASE}/Systems",
            unreal.load_asset("/Niagara/DefaultAssets/Templates/Systems/DirectionalBurst"))
        summary = call("GetSystemSummary", system)
        for old in list(summary.get_editor_property("Emitters")):
            call("RemoveEmitter", native_ref(system, str(old.get_editor_property("EmitterName"))))
        old_vars = tool("GetSystemSummary", system=refpath(system.get_path_name()))["userVariables"]
        if old_vars:
            tool("RemoveUserVariables", system=refpath(system.get_path_name()), variablesToRemove=[
                {"name": v["name"], "type": v["type"]} for v in old_vars])
        add_users(system, color, duration, count)
        for i, part in enumerate(parts):
            emitter(system, f"FX_{i}_{part['kind']}", part, duration)
        EAL.set_metadata_tag(system, "FX.Status", "Authored prototype; gameplay integration and visual approval pending")
        EAL.save_loaded_asset(system, only_if_is_dirty=False)
        diag = call("GetSystemCompileState", system)
        record = {"asset": system.get_path_name(), "emitters": len(parts),
                  "layers": [f"{BASE}/Systems/{n}.{n}" for n in ASSEMBLIES.get(name, [])][1:],
                  "duration_cap": duration, "authoring_diagnostics_complete": diag.is_complete,
                  "authoring_error": diag.error, "authoring_diagnostics": json.loads(diag.get_value_as_json_string() or "{}"),
                  "compile_validation": "Requires external forced bytecode audit; authoring state may be stale", "visual_review": "pending"}
        records = [r for r in records if r["asset"] != record["asset"]] + [record]
        unreal.log("FX SYSTEM SAVED: " + name)
    (root / "ImportSource/FX/niagara_assets.json").write_text(json.dumps(records, indent=2) + "\n", encoding="utf-8")
    (root / "ImportSource/FX/attack_presets.json").write_text(json.dumps({
        name.split("_")[1]: {"systems": [f"{BASE}/Systems/{n}.{n}" for n in layers], "cosmetic_only": True}
        for name, layers in ASSEMBLIES.items() if not name.startswith("NS_FX_")}, indent=2) + "\n", encoding="utf-8")
    unreal.log(f"FX NIAGARA: {len(records)} systems authored")
    unreal.log("FX NEXT STEP: recompile_fx_systems.py in a fresh process with -dpcvars=fx.ForceCompileOnLoad=1")


if __name__ == "__main__":
    main()
