"""
Export the selected object to an anim xml file

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
	file.write("<animation version=\"1\">\n")

	if bpy.context.active_object:
		armature = bpy.context.active_object
		for curve in armature.animation_data.action.fcurves:
			bone_name = curve.data_path.split('"')[1]

			file.write("\t<curve name=\"")
			file.write(bone_name)
			file.write("\">\n")

			file.write("\t</curve>\n")

	file.write("</animation>\n")
	file.close()
