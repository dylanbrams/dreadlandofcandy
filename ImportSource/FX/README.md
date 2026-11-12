# Original attack FX sources

Generated locally from geometry and analytic texture masks. No paid generation,
external source art or runtime service is required. Units are centimetres.

Production specification: [Niagara attack pipeline](../../Docs/Design/Niagara_Attack_Pipeline.md).
Delivery status and remaining work: [FX handoff](../../Docs/Workers/fx.md).

Rebuild in this order from the project root:

For the complete sequence, run `Scripts/build_attack_fx.ps1 -Review`. Add
`-Sources` only when regenerating the original procedural meshes/masks. The
wrapper runs every Unreal stage sequentially and includes the required repair.

1. `python Scripts/build_fx_sources.py` creates ten OBJ meshes and four PNG masks.
2. Run `Scripts/import_fx_assets.py` through Unreal's PythonScript commandlet.
   It creates/reuses assets in `/Game/FX/Attacks`, repairs mask sampler types,
   sets Niagara material usage, and removes mesh collision.
3. Run `Scripts/build_niagara_attacks.py` through the same commandlet. It authors
   nine shared assemblies and twelve presets (33 systems, including their layers), rebuilding only that library's emitters
   and user parameters. It does not alter gameplay code or register abilities.
4. **Required: force-compile and save in a fresh process.** Run
   `Scripts/recompile_fx_systems.py` with the PythonScript commandlet and
   `-dpcvars=fx.ForceCompileOnLoad=1`. This repairs saved compiled scripts that
   Niagara can otherwise incorrectly consider UpToDate. It must run after the
   builder exits and before rendering/cooking. It is also the cheapest repair
   for existing stale assets; rebuilding their graphs is unnecessary.
5. Audit with Unreal's `-run=NiagaraDumpBytecode
   -PackagePaths=/Game/FX/Attacks/Systems -BAKED
   -AuditOutputFolder=<project>/Saved/FX/Bytecode`. This is a separate validation;
   a successful asset save can still leave a stale compile state.
6. Run `Scripts/build_fx_review.py` with the PythonScript commandlet to create
   twelve isolated review maps. Then `powershell -ExecutionPolicy Bypass -File
   Scripts/review_attack_fx.ps1` captures each with an offscreen editor camera.
   Add `-Attack B02` to inspect just the cyclone.

Python commandlet invocation, using the installed engine:

```powershell
& 'D:/Program Files/Epic/UE_5.8/Engine/Binaries/Win64/UnrealEditor-Cmd.exe' `
  'D:/Source/Unreal/MegaJam2026/MegaJam.uproject' -run=pythonscript `
  -script='D:/Source/Unreal/MegaJam2026/Scripts/import_fx_assets.py' `
  -unattended -nullrhi -nosound -nop4
```

`manifest.json` records source triangles, bounds and hashes. `unreal_assets.json`
records imported meshes/materials/textures. `niagara_assets.json` records authored
systems and the authoring-time diagnostics, which may still be stale;
`validation.json` records the external compile audit. `review_maps.json` records
the camera scenes. Screenshots and verbose engine logs remain in `Saved/`.

`attack_presets.json` lists all Niagara systems belonging to each of the twelve
attacks. Spawn/attach and clean up every listed layer together. Single-emitter
layer assets make each primitive independently reusable and inspectable.
Particle lifetimes are baked from the authored preset; `User.Duration` controls
fade within that cap. Rebuild when extending a preset's lifetime.

The importer reuses existing assets. To intentionally update imported geometry,
reimport its source in the editor; it does not silently overwrite an artist's mesh.
The Niagara builder intentionally reconstructs generated system stacks, so copy a
system outside this generated library before making independent hand edits.

These are first-pass effects. Plain comic cards require approved artwork;
straight floss strands need the gameplay endpoint adapter; stream/dome duration
and cancellation need the cast coordinator. Animation, voice, sound and Resolve
remain owned by the game. See the handoff for visual and cook coverage.

UE 5.8 authoring details: use named `pitch`, `yaw`, `roll` arguments when creating
Python Rotators. Soft-mask textures require Linear Color samplers after disabling
sRGB. Offscreen editor capture needs Keep Python Script Alive, explicit camera
exposure and a reentrancy guard around Slate callbacks (map loading and capture
can tick the editor recursively).
