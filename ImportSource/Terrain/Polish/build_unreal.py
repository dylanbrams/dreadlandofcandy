"""Import polish assets, rebuild instanced ecology and replace six cottages.

Runs in a full editor; imports the dense baker without executing it. Original
geometry/generator textures remain untouched. No gameplay or config changes.
"""
import importlib.util
import json
from pathlib import Path
import unreal as u

ROOT = Path(u.Paths.convert_relative_path_to_full(u.Paths.project_dir())) / 'ImportSource/Terrain'
HERE = ROOT / 'Polish'
spec = importlib.util.spec_from_file_location('dense_baker', ROOT / 'Dense/build_unreal.py')
d = importlib.util.module_from_spec(spec)
spec.loader.exec_module(d)
d.HERE = HERE
d.BASE = '/Game/World/Polish'
ASSETS = json.loads((HERE / 'assets.json').read_text())['assets']
LIB = u.MaterialEditingLibrary
EAL = u.EditorAssetLibrary
report = dict(status='building', paid_generation=False)


def material(name, kind, color=None, texture=None, variant=0):
    mat = d.asset(name, 'Materials', u.Material, u.MaterialFactoryNew())
    LIB.delete_all_material_expressions(mat)
    transparent=kind in ('clearwater','crystal')
    mat.set_editor_properties(dict(two_sided=kind not in ('house','clearwater','crystal'),
        used_with_instanced_static_meshes=True,
        blend_mode=u.BlendMode.BLEND_TRANSLUCENT if transparent else u.BlendMode.BLEND_OPAQUE,
        shading_model=u.MaterialShadingModel.MSM_DEFAULT_LIT))
    base = d.expr(mat, u.MaterialExpressionTextureSample, texture=texture) if texture else d.expr(mat, u.MaterialExpressionConstant3Vector, constant=u.LinearColor(*color, 1))
    node = d.expr(mat, u.MaterialExpressionCustom)
    inputs = []
    pins = [('C', base, 'RGB' if texture else ''),
        ('P', d.expr(mat, u.MaterialExpressionWorldPosition), ''),
        ('UV', d.expr(mat, u.MaterialExpressionTextureCoordinate), ''),
        ('T', d.expr(mat, u.MaterialExpressionTime), ''),
        ('Variation', d.expr(mat, u.MaterialExpressionPerInstanceRandom), '')]
    for key, expression, output in pins:
        item = u.CustomInput()
        item.set_editor_property('input_name', key)
        inputs.append(item)
    node.set_editor_property('inputs', inputs)
    for key, expression, output in pins:
        d.connect(expression, output, node, key)
    node.set_editor_property('output_type', u.CustomMaterialOutputType.CMOT_FLOAT3)
    if kind == 'grass':
        code = '''float t = UV.y * 5.0 + UV.x * .65;
float wave = sin(t * 6.2831853);
float aa = max(fwidth(t) * 6.28, .08);
float band = smoothstep(-aa, aa, wave);
return lerp(C, float3(.94,.96,.89), band) * lerp(.87,1.05,Variation);'''
    elif kind in ('floss', 'blossom'):
        # Analytically filtered threads fade into soft marbling at distance.
        # Opaque subsurface avoids translucent canopy sorting/overdraw.
        code = '''float coarse = sin(P.x*.027 + sin(P.z*.036)*2 + P.y*.019);
float t = P.z*1.1 + P.x*.34 + sin(P.y*.09)*2 + coarse*.55 + sin(P.x*.43+P.y*.28)*.6;
float fine = sin(t*6.2831853);
float fade = 1-saturate(fwidth(t)*1.2);
float fiber = fine*fade;
float crossThread = sin((P.y*.78-P.x*.19+sin(P.z*.17))*6.28)*fade;
float clump = .5+.5*sin(P.y*.13+sin(P.x*.11)*2+P.z*.08);
float3 spun = C*(.85+clump*.19+fiber*.08+crossThread*.035) + float3(1,.79,.88)*max(0,fiber)*.09;
float pink = smoothstep(.03,.18,C.r-C.g)*smoothstep(.005,.12,C.b-C.g);
return lerp(C,spun,WEIGHT);'''.replace('WEIGHT', 'pink' if kind == 'blossom' else '1')
        mat.set_editor_property('shading_model', u.MaterialShadingModel.MSM_SUBSURFACE if kind=='floss' else u.MaterialShadingModel.MSM_DEFAULT_LIT)
        if kind=='floss':
            sub = d.expr(mat, u.MaterialExpressionConstant3Vector, constant=u.LinearColor(.8,.23,.38,1))
            LIB.connect_material_property(sub, '', u.MaterialProperty.MP_SUBSURFACE_COLOR)
            LIB.connect_material_property(d.scalar(mat,.38), '', u.MaterialProperty.MP_OPACITY)
    elif kind == 'house':
        # Shift the existing pink candy trim to mint or raspberry; retain icing,
        # gingerbread, window and baked ornamental detail from the source atlas.
        code = '''float pink=smoothstep(.07,.24,C.r-C.g)*smoothstep(.01,.12,C.b-C.g);
float3 trim=TINT;
float sugar=.975+.025*sin(P.x*1.7+P.y*.8)*sin(P.z*2.1);
return lerp(C,trim*max(.32,max(C.r,max(C.g,C.b))),pink*.7)*sugar;'''.replace('TINT', ['float3(1,.16,.25)', 'float3(.15,.8,.51)', 'float3(.8,.19,.62)'][variant])
    elif kind in ('water','clearwater'):
        code = '''float ripple=sin(P.x*.055+T*1.4+sin(P.y*.047))*sin(P.y*.06-T*.8);
return C*(.91+ripple*.07)+max(0,ripple-.72)*.12;'''
    elif kind=='pine_trunk':
        code='float band=smoothstep(-.12,.12,sin((P.z*.015+(P.x+P.y)*.006)*6.283)); return lerp(C,float3(.95,.94,.86),band);'
    else:
        code = 'return C;'
    node.set_editor_property('code', code)
    LIB.connect_material_property(node, '', u.MaterialProperty.MP_BASE_COLOR)
    roughness = .085 if transparent else .18 if kind=='water' else .62 if kind == 'house' else .92
    LIB.connect_material_property(d.scalar(mat,roughness), '', u.MaterialProperty.MP_ROUGHNESS)
    if transparent:
        mat.set_editor_property('translucency_lighting_mode',u.TranslucencyLightingMode.TLM_SURFACE_PER_PIXEL_LIGHTING)
        fresnel=d.expr(mat,u.MaterialExpressionFresnel)
        edge=d.expr(mat,u.MaterialExpressionMultiply,const_b=.32 if kind=='crystal' else .3)
        d.connect(fresnel,'',edge,'A')
        opacity=d.expr(mat,u.MaterialExpressionAdd,const_b=.48 if kind=='crystal' else .28)
        d.connect(edge,'',opacity,'A')
        LIB.connect_material_property(opacity,'',u.MaterialProperty.MP_OPACITY)
        LIB.connect_material_property(d.scalar(mat,1.04 if kind=='crystal' else 1.004),'',u.MaterialProperty.MP_REFRACTION)
        if kind=='crystal':
            tint=d.expr(mat,u.MaterialExpressionMultiply,const_b=.045)
            d.connect(base,'',tint,'A')
            LIB.connect_material_property(tint,'',u.MaterialProperty.MP_EMISSIVE_COLOR)
        if kind=='clearwater':
            normal=d.expr(mat,u.MaterialExpressionCustom,code='return normalize(float3(.055*sin(P.x*.065+T*1.2),.045*cos(P.y*.07-T),1));',output_type=u.CustomMaterialOutputType.CMOT_FLOAT3)
            normal_inputs=[]
            for key in ('P','T'):
                item=u.CustomInput()
                item.set_editor_property('input_name',key)
                normal_inputs.append(item)
            normal.set_editor_property('inputs',normal_inputs)
            d.connect(pins[1][1],'',normal,'P')
            d.connect(pins[3][1],'',normal,'T')
            LIB.connect_material_property(normal,'',u.MaterialProperty.MP_NORMAL)
    if kind == 'grass':
        # Retain the earlier anchored wind and ambient fill.
        vertex = d.expr(mat, u.MaterialExpressionVertexColor)
        clock = d.expr(mat, u.MaterialExpressionTime)
        speed = d.expr(mat, u.MaterialExpressionMultiply, const_b=.23)
        d.connect(clock,'',speed,'A')
        sine = d.expr(mat, u.MaterialExpressionSine)
        d.connect(speed,'',sine,'')
        weight = d.expr(mat, u.MaterialExpressionMultiply)
        d.connect(vertex,'A',weight,'A')
        d.connect(sine,'',weight,'B')
        wind = d.expr(mat, u.MaterialExpressionMultiply)
        d.connect(weight,'',wind,'A')
        d.connect(d.expr(mat,u.MaterialExpressionConstant3Vector,constant=u.LinearColor(7,2.45,0,1)),'',wind,'B')
        LIB.connect_material_property(wind,'',u.MaterialProperty.MP_WORLD_POSITION_OFFSET)
        fill = d.expr(mat,u.MaterialExpressionMultiply,const_b=.08)
        d.connect(node,'',fill,'A')
        LIB.connect_material_property(fill,'',u.MaterialProperty.MP_EMISSIVE_COLOR)
    LIB.recompile_material(mat)
    EAL.save_loaded_asset(mat)
    return mat


def import_assets():
    meshes = {}
    foliage = {r['name']:r for r in d.FOLIAGE}
    # Keep all smaller plants and animated snow from the previous pass.
    for group in d.DATA['groups']:
        if group['layer'] not in ('tree','bush'):
            meshes[group['mesh']] = u.load_asset('/Game/World/Dense/Meshes/'+group['mesh'])
    for suffix, color in [('Mint',(.035,.32,.09)), ('Rose',(.62,.035,.12)), ('Ice',(.7,.018,.035))]:
        name = 'SM_CandyGrass'+suffix
        mesh = d.import_file(ROOT / 'Dense/FBX' / (name+'.fbx'), 'Meshes')
        mesh.set_material(0, material('M_StripedGrass'+suffix,'grass',color=color))
        mesh.set_editor_property('positive_bounds_extension',u.Vector(10,10,5))
        mesh.set_editor_property('negative_bounds_extension',u.Vector(10,10,5))
        EAL.save_loaded_asset(mesh)
        meshes[name] = mesh
    for r in ASSETS:
        name = r['name']
        mesh = d.import_file(HERE/'FBX'/(name+'.fbx'), 'Meshes')
        extent = mesh.get_bounds().box_extent
        source_height = r['bounds_cm'][2][1]-r['bounds_cm'][2][0]
        assert abs(extent.z*2-source_height) < 2, (name,extent,source_height)
        if r['category'] == 'tree':
            if 'procedural_materials' in r:
                for i,slot in enumerate(mesh.get_editor_property('static_materials')):
                    key=str(slot.get_editor_property('imported_material_slot_name'))
                    mat=material('M_'+key,'pine_trunk' if key=='PineTrunk' else 'tree',color=r['procedural_materials'][key])
                    mesh.set_material(i,mat)
            else:
                original = foliage['SM_Rodin_'+r['species']]
                for slot in original['materials']:
                    tex = u.load_asset('/Game/World/Dense/Textures/'+slot['base_color'])
                    mat = material('M_'+r['species']+'_'+str(slot['slot']), 'blossom' if r['species']=='BubblegumBlossom' else 'tree', texture=tex)
                    mesh.set_material(slot['slot'],mat)
        elif r['category'] == 'bush':
            for i, slot in enumerate(mesh.get_editor_property('static_materials')):
                cream = str(slot.get_editor_property('imported_material_slot_name')) == 'Cream'
                mesh.set_material(i,material('M_SpunSugar_'+str(i),'floss',color=(.95,.63,.77) if cream else (.78,.21,.42)))
        else:
            for i, tex_name in enumerate(r['textures']):
                tex = d.import_file(HERE/'Textures'/(tex_name+'.png'),'Textures',True)
                mesh.set_material(i,material('M_CandyHouse_'+str(r['variant'])+'_'+str(i),'house',texture=tex,variant=r['variant']))
            body = mesh.get_editor_property('body_setup')
            body.set_editor_property('collision_trace_flag',u.CollisionTraceFlag.CTF_USE_COMPLEX_AS_SIMPLE)
            sm = u.get_default_object(u.StaticMeshEditorSubsystem)
            for section in range(mesh.get_num_sections(0)):
                sm.enable_section_collision(mesh,True,0,section)
            sm.remove_collisions(mesh)
        reduction = u.StaticMeshReductionOptions()
        reduction.set_editor_property('auto_compute_lod_screen_size', False)
        levels = []
        for fraction, size in [(1.,1.),(.45,.35),(.16,.12)]:
            level = u.StaticMeshReductionSettings()
            level.set_editor_properties(dict(percent_triangles=fraction,screen_size=size))
            levels.append(level)
        reduction.set_editor_property('reduction_settings',levels)
        assert u.get_default_object(u.StaticMeshEditorSubsystem).set_lods(mesh,reduction)==3
        EAL.save_loaded_asset(mesh)
        meshes[name] = mesh
    assert all(meshes.values()), 'Missing source meshes'
    return meshes, u.load_asset('/Game/World/Dense/Materials/M_CandySnowTerrain')


# Split existing tree placements across the available mesh variants. Positions,
# scale, exclusions and instance count are preserved exactly.
groups = []
for group in d.DATA['groups']:
    if group['layer'] == 'tree':
        species = group['mesh'].replace('SM_Rodin_','')
        variants=sorted(r['variant'] for r in ASSETS if r['category']=='tree' and r['species']==species)
        assert variants==list(range(len(variants))), (species,variants)
        for variant in variants:
            groups.append(dict(group,mesh='SM_'+species+'_Warp'+str(variant+1), instances=group['instances'][variant::len(variants)]))
    else:
        groups.append(dict(group,mesh='SM_SpunSugarThicket' if group['layer']=='bush' else group['mesh']))
d.DATA['groups'] = groups

# Landmark footprints remain separate from the gameplay location table. Remove
# foliage, including crown radius, so the new destinations have open approaches.
landmark_source=HERE/'landmarks.json'
if landmark_source.exists():
    landmarks=json.loads(landmark_source.read_text())['assets']
    for group in d.DATA['groups']:
        if group['layer']=='snow':
            continue
        kept=[]
        for point in group['instances']:
            clearance=300*point[4] if group['layer']=='tree' else 160*point[4] if group['layer']=='bush' else 90 if group['layer']=='flower' else 32
            if all((point[0]-r['location'][0])**2+(point[1]-r['location'][1])**2 >= (r['radius']+clearance)**2 for r in landmarks if r['radius']>0):
                kept.append(point)
        group['instances']=kept
    d.DATA['counts']={layer:sum(len(g['instances']) for g in d.DATA['groups'] if g['layer']==layer) for layer in d.DATA['counts']}
    (HERE/'ecology.json').write_text(json.dumps(d.DATA,separators=(',',':'))+'\n')

original_finish = d.finish
def finish():
    houses = sorted([a for a in d.ACTORS.get_all_level_actors() if isinstance(a,u.StaticMeshActor) and a.static_mesh_component.static_mesh and a.static_mesh_component.static_mesh.get_name().startswith(('SM_GingerbreadCottage','SM_CandyHouse_'))], key=lambda a:a.get_actor_label())
    assert len(houses)==6, len(houses)
    for i, actor in enumerate(houses):
        actor.static_mesh_component.set_static_mesh(d.meshes['SM_CandyHouse_'+str(i%3+1)])
        # Clear inherited material overrides from the simple cottage.
        actor.static_mesh_component.set_editor_property('override_materials',[])
        actor.static_mesh_component.set_collision_profile_name('BlockAll')
        actor.static_mesh_component.set_collision_enabled(u.CollisionEnabled.QUERY_AND_PHYSICS)
    if landmark_source.exists():
        spec=importlib.util.spec_from_file_location('landmark_import',HERE/'landmarks_unreal.py')
        module=importlib.util.module_from_spec(spec)
        spec.loader.exec_module(module)
        module.build(d,material)
    original_finish()
    report.update(status='passed', houses=len(houses), assets=len(ASSETS), instances=sum(d.DATA['counts'].values()), tree_variants=sum(r['category']=='tree' for r in ASSETS),
        materials='blade UV stripes, opaque subsurface fiber, tinted candy house atlas, translucent crystals and lemonade',
        performance='42-triangle grass; foliage cleared around four new landmarks. Mesh LODs and HISM culling retained. See performance.json for stationary measurements.')
    (HERE/'polish_report.json').write_text(json.dumps(report,indent=2)+'\n')
    u.log('CANDY POLISH PASS '+json.dumps(report))

d.import_assets = import_assets
d.finish = finish
if __name__ == '__main__':
    d.main()
