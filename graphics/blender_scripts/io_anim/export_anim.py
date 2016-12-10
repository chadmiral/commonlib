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
	anim_length = 100 #TODO
	file.write("<animation version=\"1\" length_frames=\"%.6f\">\n" % anim_length)

	if bpy.context.active_object:
		armature = bpy.context.active_object
		for curve in armature.animation_data.action.fcurves:
			bone_name = curve.data_path.split('"')[1]

			file.write("\t<curve bone=\"")
			file.write(bone_name)
			file.write("\">\n")
			for frame in range(1, 10):
				y = curve.evaluate(frame)
				file.write("\t\t<frame>%.6f %.6f</frame>\n" % (frame, y))

			#for kp = curve.keyframe_points:
				#kp.co : vec2
				#file.write("\t\t<keyframe>")

			file.write("\t</curve>\n")

	file.write("</animation>\n")
	file.close()
