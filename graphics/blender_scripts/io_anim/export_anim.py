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
	anim_length = scene.frame_end - scene.frame_start + 1
	fps = scene.render.fps
	file.write("<animation version=\"1\" length_frames=\"%i\" fps=\"%i\">\n" % (anim_length, fps))

	if bpy.context.active_object:
		armature = bpy.context.active_object
		for curve in armature.animation_data.action.fcurves:
			bone_name = curve.group.name #curve.data_path.split('"')[1]

			tmp = curve.data_path.split('.')
			transform_name = tmp[-1] #last element

			axis_name = ['x', 'y', 'z', 'w'][curve.array_index]

			file.write("\t<anim_curve bone=\"")
			file.write(bone_name)
			file.write("\" data_type=\"")
			file.write(transform_name)
			file.write("\" axis=\"")
			file.write(axis_name)
			file.write("\">\n")

			#TODO
			"""
			#write out the keyframes
			for kp in curve.keyframe_points:
				#kp.co : vec2
				file.write("\t\t<key_frame>\n")
				file.write("\t\t\t<pos>%.6f %.6f</pos>\n" % (kp.co[0], kp.co[1]))
				file.write("\t\t\t<handle_left type=")
				file.write(handle_left_type)
			"""

			#evaluate the fcurve at every frame (bake it down)
			for frame in range(scene.frame_start, scene.frame_end + 1):
				y = curve.evaluate(frame)

				#transform into [0,1] range
				x = (frame - scene.frame_start) / (scene.frame_end - scene.frame_start)
				file.write("\t\t<frame>%.6f %.6f</frame>\n" % (x, y))
			file.write("\t</anim_curve>\n")

	file.write("</animation>\n")
	file.close()
