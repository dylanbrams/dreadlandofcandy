"""Asset-specific reversible preparation. Source files are never edited."""
import bpy,bmesh,json,math
import numpy as np
from pathlib import Path
from mathutils import Vector
OUT=Path(__file__).resolve().parent


def geometry(obj,name):
    actions=['Welded glTF attribute splits before Collapse; retained per-corner UVs.']
    pedestal={'SM_Boulder':(.21,(.55,.30,.70,1)), 'SM_CandyCrop':(.125,(.65,.45,.30,1)),
        'SM_CandyTree':(.18,(.40,.18,.07,1)), 'SM_Molar':(.18,(.88,.85,.75,1)),
        'SM_Signpost':(.205,(.40,.18,.07,1)), 'SM_Crook':(.20,(.42,.24,.12,1)),
        'SM_Peppermint_Fal':(.10,(.82,.75,.60,1)), 'SM_HouseFront_Fal':(.105,(.45,.20,.06,1)),
        'SM_CandyFence':(.08,(.85,.79,.65,1))}
    if name in pedestal:
        height,color=pedestal[name]
        if name=='SM_Signpost':
            _,pixels=image_samples(obj.data);uv=obj.data.uv_layers.active.data
            green=[]
            for face in obj.data.polygons:
                r,g,b=color_at(face,uv,pixels)
                if face.center.z<.35 and g>r*1.05 and g>b*1.10:
                    green.extend(obj.data.vertices[i].co.z for i in face.vertices)
            if green:height=max(height,max(green)+.005)
        clip(obj,(0,0,1),height,color)
        actions.append('Removed generated ground/pedestal geometry and capped exposed contact surfaces.')
        if name in ('SM_CandyTree','SM_HouseFront_Fal'):keep_central_component(obj)
    if name=='SM_Table':
        keep_central_component(obj)
        actions.append('Removed all four separate chair components; retained only the table and its four legs.')
    elif name=='SM_Sword':
        low=min(v.co.z for v in obj.data.vertices);high=max(v.co.z for v in obj.data.vertices)
        height=high-low;guard=low+height*.29
        obj['grip_anchor_source_z']=low+height*.15
        for v in obj.data.vertices:
            if v.co.z>guard:
                blend=min(1,(v.co.z-guard)/(height*.06))
                v.co.x*=1+.45*blend
                v.co.z=guard+(v.co.z-guard)*1.45
        newhigh=max(v.co.z for v in obj.data.vertices)
        scale=height/(newhigh-low)
        for v in obj.data.vertices:
            v.co.x*=scale;v.co.y*=scale;v.co.z=low+(v.co.z-low)*scale
        obj['grip_anchor_source_z']=low+(obj['grip_anchor_source_z']-low)*scale
        actions.append('Lengthened and broadened blade relative to grip, retaining original overall height and atlas UVs; no paid regeneration.')
    elif name=='SM_Clapperboard':
        clip(obj,(0,0,1),.71,(.025,.025,.035,1))
        clip(obj,(1,0,0),-.34,(.025,.025,.035,1));clip(obj,(-1,0,0),-.34,(.025,.025,.035,1))
        keep_central_component(obj)
        bpy.ops.mesh.primitive_cube_add(size=1,location=(0,-.01,.49))
        board=bpy.context.object;board.scale=(.68,.12,.47)
        board.data.materials.append(obj.data.materials[cut_material(obj,(.025,.025,.035,1))])
        bpy.ops.object.select_all(action='DESELECT');board.select_set(True);obj.select_set(True)
        bpy.context.view_layer.objects.active=obj;bpy.ops.object.join();obj.select_set(False)
        actions.append('Retained textured striped clapper/hinge and rebuilt the blank slate without limbs, stubs or floating glove fragments.')
    elif name=='SM_FortGate':
        subtract(obj,'ARCH',(0,0,0),(1,1,1),(.85,.65,.65,1))
        actions.append('Removed embedded knight and opened a continuous arched passage through the gate.')
    elif name=='SM_Hat_Straw':
        rebuild_accessory(obj,'hat')
        actions.append('Rebuilt a complete broad-brim hollow straw hat without the head, using the embedded straw atlas and a Villager-sized crown.')
    elif name=='SM_HeadMirror':
        rebuild_accessory(obj,'mirror')
        actions.append('Rebuilt a continuous hollow headband and perforated mirror disc without the head, sized to the actual Villager head.')
    elif name=='SM_LeafMegaphone':
        rebuild_accessory(obj,'megaphone')
        actions.append('Rebuilt the rolled leaf as a continuous hollow cone with handle and raised veins, without the interior face/ornament.')
    elif name=='SM_LightStand':
        rebuild_stand(obj)
        actions.append('Retained textured lamp housing/barn doors, removed wired lower assembly, and rebuilt a clean cable-free tripod and pole.')
    elif name=='SM_LollyShard':
        remove_stick(obj)
        actions.append('Removed the white lollipop stick, keeping the pink low-poly candy fragment.')
    # Keep approved faces and character silhouettes; these are not geometry defects.
    return actions


def cut_material(obj,color):
    name=obj.name+'_CutInterior'
    material=bpy.data.materials.get(name)
    if material is None:
        material=bpy.data.materials.new(name);material.use_nodes=True
        shader=material.node_tree.nodes.get('Principled BSDF')
        shader.inputs['Base Color'].default_value=color
        shader.inputs['Roughness'].default_value=.85
        material.diffuse_color=color
    for index,existing in enumerate(obj.data.materials):
        if existing==material:return index
    index=len(obj.data.materials);obj.data.materials.append(material);return index


def cap(bm,material):
    edges=[e for e in bm.edges if e.is_boundary]
    if not edges:return
    result=bmesh.ops.holes_fill(bm,edges=edges,sides=0)
    uv=bm.loops.layers.uv.active
    for face in result['faces']:
        face.material_index=material
        if uv:
            for loop in face.loops:
                loop[uv].uv=(loop.vert.co.x*.1+.5,loop.vert.co.y*.1+.5)
    bmesh.ops.recalc_face_normals(bm,faces=list(bm.faces))


def clip(obj,normal,offset,color):
    material=cut_material(obj,color)
    bm=bmesh.new();bm.from_mesh(obj.data)
    normal=Vector(normal)
    bmesh.ops.bisect_plane(bm,geom=list(bm.verts)+list(bm.edges)+list(bm.faces),
        dist=1e-7,plane_co=normal*offset,plane_no=normal,clear_inner=True,clear_outer=False)
    cap(bm,material)
    bm.to_mesh(obj.data);bm.free();obj.data.update()


def subtract(obj,kind,center,dimensions,color,rotation=(0,0,0),radius2=None):
    material=cut_material(obj,color)
    if kind=='ARCH':
        outline=[(.115,-.2)]+[(.115*math.cos(i*math.pi/32),.17+.115*math.sin(i*math.pi/32)) for i in range(33)]+[(-.115,-.2)]
        n=len(outline);verts=[(x,y,z) for y in (-.5,.5) for x,z in outline]
        faces=[tuple(reversed(range(n))),tuple(range(n,2*n))]
        faces += [(i,(i+1)%n,(i+1)%n+n,i+n) for i in range(n)]
        mesh=bpy.data.meshes.new('Arched passage');mesh.from_pydata(verts,[],faces);mesh.update()
        cutter=bpy.data.objects.new('Arched passage',mesh);bpy.context.collection.objects.link(cutter)
        bm=bmesh.new();bm.from_mesh(mesh);bmesh.ops.recalc_face_normals(bm,faces=list(bm.faces));bm.to_mesh(mesh);bm.free()
        bpy.context.view_layer.objects.active=cutter
    elif kind=='CUBE':bpy.ops.mesh.primitive_cube_add(size=1,location=center,rotation=rotation)
    elif kind=='CYLINDER':bpy.ops.mesh.primitive_cylinder_add(vertices=64,radius=.5,depth=1,location=center,rotation=rotation)
    else:bpy.ops.mesh.primitive_cone_add(vertices=64,radius1=.5,radius2=radius2 or 0,depth=1,location=center,rotation=rotation)
    cutter=bpy.context.view_layer.objects.active
    # Dimensions refer to the cutter's local axes, before its rotation.
    cutter.scale=dimensions
    for mat in obj.data.materials:cutter.data.materials.append(mat)
    for face in cutter.data.polygons:face.material_index=material
    bpy.context.view_layer.objects.active=obj
    mod=obj.modifiers.new('Remove unwanted geometry','BOOLEAN');mod.operation='DIFFERENCE';mod.solver='EXACT';mod.object=cutter
    bpy.ops.object.modifier_apply(modifier=mod.name)
    bpy.data.objects.remove(cutter,do_unlink=True)
    obj.data.update()


def keep_central_component(obj):
    bm=bmesh.new();bm.from_mesh(obj.data)
    seen=set();parts=[]
    for vertex in bm.verts:
        if vertex in seen:continue
        part=set([vertex]);stack=[vertex];seen.add(vertex)
        while stack:
            v=stack.pop()
            for edge in v.link_edges:
                other=edge.other_vert(v)
                if other not in seen:seen.add(other);part.add(other);stack.append(other)
        parts.append(part)
    def distance(part):
        center=sum((v.co for v in part),Vector())/len(part)
        return center.x**2+center.y**2
    keep=min(parts,key=distance)
    bmesh.ops.delete(bm,geom=[v for v in bm.verts if v not in keep],context='VERTS')
    bm.to_mesh(obj.data);bm.free();obj.data.update()


def remove_wire(obj):
    samples=image_samples(obj.data)
    if not samples:return
    _,pixels=samples;uv=obj.data.uv_layers.active.data
    marked=[]
    for face in obj.data.polygons:
        r,g,b=color_at(face,uv,pixels);c=face.center
        if c.z<.55 and c.z>.045 and c.y>.08 and b>r*1.6 and b>g*1.08:
            marked.append(face.index)
    bm=bmesh.new();bm.from_mesh(obj.data);bm.faces.ensure_lookup_table()
    bmesh.ops.delete(bm,geom=[bm.faces[i] for i in marked],context='FACES')
    # Remove the exposed trailing region; do not alter the tripod's nearby feet.
    bmesh.ops.delete(bm,geom=[v for v in bm.verts if v.co.y>.28 and v.co.z<.55],context='VERTS')
    loose=[v for v in bm.verts if not v.link_faces]
    if loose:bmesh.ops.delete(bm,geom=loose,context='VERTS')
    material=cut_material(obj,(.06,.15,.25,1))
    edges=[e for e in bm.edges if e.is_boundary]
    result=bmesh.ops.triangle_fill(bm,edges=edges,use_beauty=True,use_dissolve=False)
    for face in result['geom']:
        if isinstance(face,bmesh.types.BMFace):face.material_index=material
    bmesh.ops.recalc_face_normals(bm,faces=list(bm.faces))
    bm.to_mesh(obj.data);bm.free();obj.data.update()


def rebuild_stand(obj):
    clip(obj,(0,0,1),.53,(.08,.25,.32,1))
    clip(obj,(0,-1,0),-.17,(.08,.25,.32,1))
    keep_central_component(obj)
    created=[]
    colors={'Wood':(.42,.18,.065,1),'Metal':(.055,.22,.30,1)}
    mats={}
    for name,color in colors.items():
        mat=bpy.data.materials.new('LightStand_'+name);mat.use_nodes=True;mat.diffuse_color=color
        shader=mat.node_tree.nodes.get('Principled BSDF');shader.inputs['Base Color'].default_value=color
        shader.inputs['Roughness'].default_value=.7;mats[name]=mat
    def pole(a,b,radius,material):
        a,b=Vector(a),Vector(b);direction=b-a
        bpy.ops.mesh.primitive_cylinder_add(vertices=12,radius=radius,depth=direction.length,location=(a+b)/2)
        part=bpy.context.object;part.rotation_euler=direction.to_track_quat('Z','Y').to_euler()
        part.data.materials.append(mats[material]);created.append(part)
    pole((0,0,.25),(0,0,.58),.035,'Metal')
    for i in range(3):
        angle=2*math.pi*i/3+math.pi/2
        foot=(.20*math.cos(angle),.20*math.sin(angle),.025)
        pole((0,0,.40),foot,.025,'Wood')
        pole((foot[0],foot[1],0),(foot[0],foot[1],.045),.030,'Metal')
    bpy.ops.object.select_all(action='DESELECT')
    for part in created:part.select_set(True)
    obj.select_set(True);bpy.context.view_layer.objects.active=obj
    bpy.ops.object.join();obj.select_set(False);obj.data.update()


def remove_stick(obj):
    samples=image_samples(obj.data)
    if not samples:return
    _,pixels=samples;uv=obj.data.uv_layers.active.data
    marked=[]
    for face in obj.data.polygons:
        r,g,b=color_at(face,uv,pixels)
        if min(r,g,b)>.65 and max(r,g,b)-min(r,g,b)<.09:marked.append(face.index)
    bm=bmesh.new();bm.from_mesh(obj.data);bm.faces.ensure_lookup_table()
    bmesh.ops.delete(bm,geom=[bm.faces[i] for i in marked],context='FACES')
    cap(bm,cut_material(obj,(.85,.30,.60,1)))
    bm.to_mesh(obj.data);bm.free();obj.data.update()


def image_samples(mesh):
    material=mesh.materials[0]
    shader=next(n for n in material.node_tree.nodes if n.type=='BSDF_PRINCIPLED')
    socket=shader.inputs['Base Color']
    image=socket.links[0].from_node.image if socket.is_linked and socket.links[0].from_node.type=='TEX_IMAGE' else None
    if not image or not mesh.uv_layers.active:return None
    pixels=np.empty(len(image.pixels),dtype=np.float32);image.pixels.foreach_get(pixels)
    return image,pixels.reshape(image.size[1],image.size[0],4)


def rebuild_accessory(obj,kind):
    """Rebuild fused head/ornament failures; retain source atlas for matched paint."""
    original=obj.data
    samples=image_samples(original)
    _,pixels=samples
    uvdata=original.uv_layers.active.data
    # Select a real atlas patch on the intended source part, avoiding head colours.
    candidates=[]
    for face in original.polygons:
        rgb=color_at(face,uvdata,pixels);r,g,b=rgb;c=face.center
        if kind=='hat':
            okay=c.z>.66 and r>.45 and g>.35 and r>=g and g>b*1.12
        elif kind=='mirror':
            okay=r>.65 and g>.65 and b>.60 and max(rgb)-min(rgb)<.12
        else:
            okay=g>r*1.10 and g>b*.80 and g>.35 and c.z>.30
        if okay:
            uv=sum((uvdata[i].uv for i in face.loop_indices),Vector((0,0)))/len(face.loop_indices)
            candidates.append((float(min(rgb)),uv.copy()))
    assert candidates,'No valid atlas patch for '+kind
    candidates.sort(key=lambda c:-c[0]);patch=candidates[len(candidates)//2][1]
    source_material=original.materials[0]
    # Old tangent normals describe the fused head/ornament, not these new surfaces.
    source_material=source_material.copy();source_material.name=obj.name+'_AtlasPaint'
    shader=next(n for n in source_material.node_tree.nodes if n.type=='BSDF_PRINCIPLED')
    for link in list(shader.inputs['Normal'].links):source_material.node_tree.links.remove(link)
    for socket,value in [('Metallic',0.0),('Roughness',.85),('Specular',.20)]:
        for link in list(shader.inputs[socket].links):source_material.node_tree.links.remove(link)
        shader.inputs[socket].default_value=value
    vertices=[];faces=[];materials=[]
    def surface(profile,n=48,axis='Z',center=(0,0,0),ellipse=1.0,material=0):
        offset=len(vertices)
        for radius,height in profile:
            for i in range(n):
                angle=2*math.pi*i/n
                if axis=='Z':point=(radius*math.cos(angle),ellipse*radius*math.sin(angle),height)
                else:point=(radius*math.cos(angle),height,radius*math.sin(angle))
                vertices.append(tuple(point[j]+center[j] for j in range(3)))
        for ring in range(len(profile)):
            other=(ring+1)%len(profile)
            for i in range(n):
                faces.append((offset+ring*n+i,offset+ring*n+(i+1)%n,offset+other*n+(i+1)%n,offset+other*n+i));materials.append(material)
    def box(center,size,material=0):
        offset=len(vertices)
        vertices.extend([(center[0]+x*size[0]/2,center[1]+y*size[1]/2,center[2]+z*size[2]/2) for z in (-1,1) for y in (-1,1) for x in (-1,1)])
        for face in ((0,2,3,1),(4,5,7,6),(0,1,5,4),(2,6,7,3),(0,4,6,2),(1,3,7,5)):
            faces.append(tuple(offset+i for i in face));materials.append(material)
    if kind=='hat':
        # Profile closes above the hollow crown; tiny central radius is welded below.
        surface([(.00001,.92),(.19,.92),(.27,.85),(.28,.64),(.40,.54),(.46,.52),
                 (.46,.49),(.40,.51),(.255,.61),(.235,.83),(.00001,.85)],ellipse=.94)
        # Cream band is distinct from the straw paint, with a broad readable strip.
        surface([(.283,.645),(.275,.710),(.279,.710),(.287,.645)],ellipse=.94,material=1)
        for height in (.735,.780,.825):
            radius=.28-(height-.64)*.01/.21
            surface([(radius+.003,height-.002),(radius+.003,height+.002),
                     (radius-.001,height+.002),(radius-.001,height-.002)],ellipse=.94,material=2)
    elif kind=='mirror':
        surface([(.48,-.065),(.48,.065),(.445,.065),(.445,-.065)],ellipse=1.0)
        surface([(.190,-.555),(.190,-.510),(.030,-.510),(.030,-.555)],axis='Y',center=(0,0,.10))
        surface([(.193,-.556),(.193,-.525),(.177,-.525),(.177,-.556)],axis='Y',center=(0,0,.10),material=1)
        box((0,-.49,-.05),(.12,.12,.08))
    else:
        surface([(.255,-.35),(.085,.45),(.060,.45),(.225,-.35)],axis='Y',center=(0,0,.51))
        box((0,.27,.23),(.105,.115,.43))
        box((0,.22,.055),(.11,.21,.075))
        # Simple raised leaf veins running back from the wide opening.
        for angle in (0,math.pi/2,math.pi,3*math.pi/2):
            start=Vector((.258*math.cos(angle),-.33,.51+.258*math.sin(angle)))
            end=Vector((.090*math.cos(angle),.41,.51+.090*math.sin(angle)))
            direction=end-start;mid=(start+end)/2
            offset=len(vertices);side=direction.cross(Vector((0,1,0))).normalized()*.009
            normal=direction.cross(side).normalized()*.006
            for p in (start,end):
                for a,b in ((-1,-1),(1,-1),(1,1),(-1,1)):vertices.append(tuple(p+side*a+normal*b))
            for face in ((0,3,2,1),(4,5,6,7),(0,1,5,4),(1,2,6,5),(2,3,7,6),(3,0,4,7)):
                faces.append(tuple(offset+i for i in face));materials.append(1)
    mesh=bpy.data.meshes.new(obj.name+'_Rebuilt');mesh.from_pydata(vertices,[],faces);mesh.update()
    mesh.materials.append(source_material)
    detail=bpy.data.materials.new(obj.name+'_Detail');detail.use_nodes=True
    detail.diffuse_color=(.88,.84,.69,1) if kind=='hat' else ((.16,.48,.65,1) if kind=='mirror' else (.18,.42,.10,1))
    shader=detail.node_tree.nodes.get('Principled BSDF');shader.inputs['Base Color'].default_value=detail.diffuse_color
    shader.inputs['Roughness'].default_value=.75;mesh.materials.append(detail)
    if kind=='hat':
        straw=detail.copy();straw.name=obj.name+'_StrawRidges'
        straw.diffuse_color=(.62,.43,.18,1)
        straw.node_tree.nodes.get('Principled BSDF').inputs['Base Color'].default_value=straw.diffuse_color
        mesh.materials.append(straw)
    uv=mesh.uv_layers.new(name='UVMap')
    # Tiny nondegenerate patches retain the original base-colour atlas as paint.
    delta=.20/max(pixels.shape[:2])
    for face,material in zip(mesh.polygons,materials):
        face.material_index=material;face.use_smooth=True
        for i,loop in enumerate(face.loop_indices):
            angle=2*math.pi*i/len(face.loop_indices)
            uv.data[loop].uv=(patch.x+delta*math.cos(angle),patch.y+delta*math.sin(angle))
    bm=bmesh.new();bm.from_mesh(mesh)
    if kind=='hat':bmesh.ops.remove_doubles(bm,verts=list(bm.verts),dist=.00003)
    bmesh.ops.recalc_face_normals(bm,faces=list(bm.faces))
    bm.to_mesh(mesh);bm.free();mesh.update();obj.data=mesh
    if kind=='hat':
        for vert in mesh.vertices:vert.co*=1.85
    obj['procedural_repair']=kind


def color_at(face,uv,pixels):
    coords=sum((uv[i].uv for i in face.loop_indices),Vector((0,0)))/len(face.loop_indices)
    height,width=pixels.shape[:2]
    return pixels[min(height-1,max(0,int(coords.y*height))),min(width-1,max(0,int(coords.x*width))),:3]


def regions(obj,name):
    if name not in ('SM_Goblin','SM_LollyBug','SM_Molar'):return []
    mesh=obj.data
    if name=='SM_Molar':
        mesh.materials[0].name='SM_Molar_Enamel'
        obj['enamel_material_slot']=0
        return ['Named enamel material for independent plaque/treatment material setup.']
    samples=image_samples(mesh)
    if not samples:return []
    image,pixels=samples;uv=mesh.uv_layers.active.data
    slots={};counts={}
    for face in mesh.polygons:
        r,g,b=color_at(face,uv,pixels)
        center=face.center
        label=None
        if name=='SM_Goblin':
            if center.z>.10 and center.y<0 and r>.35 and g>.30 and b<.30 and g>b*1.5:label='Teeth'
            elif g>r*1.12 and g>b*.95:label='Skin'
        elif abs(center.x)>.35 and center.z>-.30 and r>.45 and g>.45 and b>.35 and abs(r-g)<.20:
            label='Wings'
        if not label:continue
        if label not in slots:
            mat=mesh.materials[0].copy();mat.name=name+'_'+label
            slots[label]=len(mesh.materials);mesh.materials.append(mat);counts[label]=0
        face.material_index=slots[label];counts[label]+=1
    obj['material_regions']=json.dumps(counts)
    return ['Separated texture-derived material regions without changing atlas pixels: '+json.dumps(counts)]


def finish(obj,name):
    actions=[]
    mesh=obj.data
    actions+=regions(obj,name)
    # The collapsed mesh needs fresh split normals after its topology changes.
    bpy.context.view_layer.objects.active=obj
    obj.select_set(True)
    if mesh.has_custom_normals:
        bpy.ops.mesh.customdata_custom_splitnormals_clear()
    mesh.use_auto_smooth=True
    mesh.auto_smooth_angle=math.radians(50)
    mesh.update()
    obj.select_set(False)
    if name=='SM_Hat_Straw':
        anchor=Vector((0,0,.64*1.85));purpose='Villager head-top mount'
    elif name=='SM_HeadMirror':
        anchor=Vector((0,0,0));purpose='Villager head-centre mount'
    elif name=='SM_LeafMegaphone':
        anchor=Vector((0,.27,.23));purpose='hand grip'
    elif name=='SM_Crook':
        low=min(v.co.z for v in mesh.vertices);high=max(v.co.z for v in mesh.vertices)
        anchor=Vector((0,0,low+(high-low)*.20));purpose='hand grip'
    elif name=='SM_LollyShard':
        anchor=sum((v.co for v in mesh.vertices),Vector())/len(mesh.vertices);purpose='particle centre'
    elif name in ('SM_Sword','SM_Toothbrush'):
        low=min(v.co.z for v in mesh.vertices); high=max(v.co.z for v in mesh.vertices)
        anchor=Vector((0,0,low+(high-low)*.19))
        if name=='SM_Sword':anchor.z=obj['grip_anchor_source_z']
        purpose='hand grip'
    elif name=='SM_Fenella':
        low=min(v.co.z for v in mesh.vertices); high=max(v.co.z for v in mesh.vertices)
        anchor=Vector((0,0,low+(high-low)*.52))
        purpose='flight anchor'
    else:
        anchor=Vector((0,0,min(v.co.z for v in mesh.vertices)))
        purpose='ground contact'
    for vert in mesh.vertices:vert.co-=anchor
    obj.name=name; mesh.name=name
    obj['pivot_purpose']=purpose
    obj['source_anchor_m']=list(anchor)
    actions.append('Set origin at '+purpose+'; retained source scale for explicit engine fitting.')
    return actions


def record(source,destination,before,after,target,actions,topology,source_hash):
    report=OUT/'cleanup_results.json'
    rows=json.loads(report.read_text(encoding='utf-8')) if report.exists() else {}
    rows[source.parent.name+'/'+source.name]={'triangles_before':before,'triangles_after':after,
        'target':target,'actions':actions,'topology':topology,'source_sha256':source_hash,
        'output':str(destination.relative_to(OUT)),
        'objects':[{'name':o.name,'pivot_purpose':o.get('pivot_purpose'),
                    'source_anchor_m':list(o.get('source_anchor_m',[]))} for o in bpy.context.scene.objects if o.type=='MESH']}
    report.write_text(json.dumps(rows,indent=2),encoding='utf-8')
