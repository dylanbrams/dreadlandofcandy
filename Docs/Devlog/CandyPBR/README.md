# Candy environment preparation

Generated with fal `fal-ai/patina/material`, Nov 15, 2026. Fifteen 1024-square
seamless material sets: three variants each of rock candy gravel, peanut brittle
bricks, peppermint cobbles, chocolate mint cobbles, and mint sugar grass.
Every set contains preview, basecolor, normal, roughness, height and metalness.
Inputs, seeds, request IDs and original output metadata are preserved per folder.

Open `review.html` for all maps; `contact.jpg` compares the variants.
`selection.json` records the initial reviewed choices. These favor coherent
paving, readable shapes, sparse gravel shoulders and larger mint grass blades.

Remote generation completed; 75 PBR maps decoded as valid 1024x1024 PNGs.
Preview images are additional to those 75 maps. No generated model code executes.

Applied during exclusive Unreal handoff: run `../apply_candy_environment.py` with Unreal
Python. It imports chosen maps, builds blended ground materials, assigns them
to existing terrain assets, creates the hot chocolate river materials, then
loads and dresses `/Game/World/L_Sugarborough`. The last step saves the map.
The earlier procedural `candy_paths.py` is superseded and is not called.

Unreal execution and asset/map saves passed. First runtime river capture rendered
the PBR terrain and hot chocolate without persisted material compiler errors.
The first capture prompted softer steam, blended banks and scaled-down mountains.
Final 1600x1000 village capture confirms peppermint plaza, brittle roads, mint
cobbles and grass after updating the Dense terrain actor material overrides.
Final river capture confirms chocolate, floating marshmallows and faint steam.
Traversal, performance, cooking and packaged delivery remain unverified. Check
normal direction, transitions into other biomes, bridge clearance, animated
marshmallow bounds, steam brightness and end-of-terrain silhouettes.
Height maps are retained for tuning; the initial ground pass uses basecolor,
normal and roughness without geometry displacement.

No C++ build, packaging or upload has been performed for this pass.
