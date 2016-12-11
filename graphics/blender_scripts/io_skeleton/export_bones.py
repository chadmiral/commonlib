"""
Export the selected object to a bones xml file

"""

import bpy

def write(filepath,
		  applyMods=True,
		  triangulate=True,
		  ):

	# grab the scene
	scene = bpy.context.scene

	# write the header of the file
	file = open(filepath, "w")
	file.write("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\" ?>\n")
	file.write("<skeleton_rig version=\"1\">\n")

	for obj in bpy.context.selected_objects:
		if obj.type == 'ARMATURE':
			file.write("\t<skeleton name=\"")
			file.write(obj.name)
			file.write("\">\n")
			for bone in obj.data.bones:
				file.write("\t\t<bone name=\"")
				file.write(bone.name)
				if bone.parent:
					file.write("\" parent=\"")
					file.write(bone.parent.name)
				file.write("\">\n")

				#note: bone.head / bone.tail will give coordinates relative to the bone's parent
				file.write("\t\t\t<head>%.6f %.6f %.6f</head>\n" % (bone.head_local[0], bone.head_local[1], bone.head_local[2]))
				file.write("\t\t\t<tail>%.6f %.6f %.6f</tail>\n" % (bone.tail_local[0], bone.tail_local[1], bone.tail_local[2]))

				file.write("\t\t</bone>\n")
			file.write("\t</skeleton>\n")

	file.write("</skeleton_rig>\n")
	file.close()
