import bpy

# Modifier type
modifier_type = 'TRIANGULATE'

# Iterate through all objects in the scene
for obj in bpy.context.scene.objects:
    # Check if the object is a mesh
    if obj.type == 'MESH':
        # Add the Triangulate modifier to the object
        modifier = obj.modifiers.new(name="Triangulate", type=modifier_type)
        # Apply the modifier to the object
        bpy.ops.object.modifier_apply(modifier=modifier.name)