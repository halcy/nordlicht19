import bpy
import sys
import bmesh
import mathutils

file = open("C:\\Users\\halcy\\Desktop\\3D\\nordlicht19\\motion_test.txt", "w")
sys.stdout = file

currentScene = bpy.context.scene
rig = bpy.context.object
obj = bpy.data.objects['Guy']
 
# meshCopy = obj.to_mesh(currentScene, False, 'PREVIEW')

# This triangulates the object. not a problem here but just for the future
bm = bmesh.new()
bm.from_mesh(obj.data)
bmesh.ops.triangulate(bm, faces=bm.faces)
bm.to_mesh(obj.data)
bm.free()
del bm

obj.data.calc_normals_split()
loops = obj.data.loops
uv_layer = obj.data.uv_layers.active.data[:]

print("VERTICES:")
for v in obj.data.vertices:
    print("%d: %f %f %f" % (v.index, v.co.x, v.co.y, v.co.z))

print("POLYGONS:")
for p in obj.data.polygons:
    normals = []
    uvs = []
    for l in p.loop_indices:
        normals.append(" ".join(map(lambda x: str(x), loops[l].normal)))
        uvs.append(" ".join(map(lambda x: str(x), uv_layer[l].uv)))
    print(" ".join(map(lambda x: str(x), p.vertices)) + " " + " ".join(normals) + " " + " ".join(uvs))
    
obj_verts = obj.data.vertices
obj_group_names = [g.name for g in obj.vertex_groups]

print("RIG:")
for bone in rig.pose.bones:
    if bone.name not in obj_group_names:
        continue

    gidx = obj.vertex_groups[bone.name].index

    bone_verts = [v for v in obj_verts if gidx in [g.group for g in v.groups]]

    for v in bone_verts:
       for g in v.groups:
          if g.group == gidx: 
             w = g.weight
             print('Vertex ' + str(v.index) + ' Bone ' + bone.name + ': ' + str(w))


print("ANIMATION:")
startFrame = currentScene.frame_start
endFrame = currentScene.frame_end

for frame in range(startFrame, endFrame+1, 1):
    print("Frame " + str(frame))
    currentScene.frame_set(frame)
    allBones = rig.pose.bones
    for bone in allBones:
        print("Bone " + bone.name)
        #sys.stderr.write(str(bone.bone.matrix_local.copy().inverted()))
        mat = bone.matrix @ bone.bone.matrix_local.copy().inverted()
        mat.transpose()
        bone_space = mathutils.Matrix(((1,0,0,0),(0,0,1,0),(0,1,0,0),(0,0,0,1)))
        #mat = rig.convert_space(bone, bone.matrix , 'POSE', 'WORLD')
        print(mat @ bone_space)
        
sys.stdout = sys.__stdout__
file.close()

# obj.data = meshCopy
