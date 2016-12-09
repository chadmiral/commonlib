"""
Export the selected object to a brick mesh (hopefully it's already a mesh and ready for export!)

Requirements: transforms applied... (world matrix, etc not taken into account)

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
	file.write("<static_mesh version=\"1\">\n")

	for obj in bpy.context.selected_objects:
		if obj.type == 'MESH':
			mesh = obj.data
			if not mesh.tessfaces and mesh.polygons:
				mesh.calc_tessface()

			#if mesh is not None:
			#matrix = obj.matrix_world.copy()
			#file.write("<render_geo>\n")
			for m in mesh.materials:
				file.write("\t<material>\n")
				file.write("\t\t<diffuse_color>%.6f %.6f %.6f</diffuse_color>\n" % (m.diffuse_color[0], m.diffuse_color[1], m.diffuse_color[2]))

				#for ts in m.texture_slots:
				#	if ts:
				#		file.write("\t\t\t<texture>%s</texture>\n" % (ts.texture.image.filepath))

				file.write("\t</material>\n\n")

			for v in mesh.vertices:
				file.write("\t<vertex>\n");
				file.write("\t\t<vert_pos>%.6f %.6f %.6f</vert_pos>\n" % (v.co[0], v.co[1], v.co[2]))
				file.write("\t\t<normal>%.6f %.6f %.6f</normal>\n" % (v.normal[0], v.normal[1], v.normal[2]))
				file.write("\t</vertex>\n\n")

			file.write("\n")

			uv_tex = mesh.tessface_uv_textures['UVMap'].data
			vertex_colors = mesh.tessface_vertex_colors['Col'].data

			# write out all the faces - vertex indices, uvs, material ids, smoothing info... surface normals
			for face in mesh.tessfaces:
				file.write("\t<face>\n")

				file.write("\t\t<face_idx>%d</face_idx>\n" % face.index)

				#material index
				file.write("\t\t<mat_idx>%d</mat_idx>\n" % face.material_index)

				#uv coordinates
				uv_face = uv_tex[face.index]
				#file.write("\t\t<uv_face>\n")
				file.write("\t\t<uv>%.6f %.6f</uv>\n" % (uv_face.uv1[0], uv_face.uv1[1]))
				file.write("\t\t<uv>%.6f %.6f</uv>\n" % (uv_face.uv2[0], uv_face.uv2[1]))
				file.write("\t\t<uv>%.6f %.6f</uv>\n" % (uv_face.uv3[0], uv_face.uv3[1]))
				#file.write("\t\t</uv_face>\n")

				vert_col_face = vertex_colors[face.index]
				file.write("\t\t<col>%.6f %.6f %.6f</col>\n" % (vert_col_face.color1[0], vert_col_face.color1[1], vert_col_face.color1[2]))
				file.write("\t\t<col>%.6f %.6f %.6f</col>\n" % (vert_col_face.color2[0], vert_col_face.color2[1], vert_col_face.color2[2]))
				file.write("\t\t<col>%.6f %.6f %.6f</col>\n" % (vert_col_face.color3[0], vert_col_face.color3[1], vert_col_face.color3[2]))

				# loop through all the vertex indices
				for v_idx in face.vertices:
					file.write("\t\t<v_idx>%d</v_idx>\n" % v_idx)
					#file.write("\t\t<uvs>%.6f %.6f</uvs>\n" % (uv_face))

				#face normal
				file.write("\t\t<normal>%.6f %.6f %.6f</normal>\n" % (face.normal[0], face.normal[1], face.normal[2]))



				file.write("\t</face>\n\n")
			#file.write("</render_geo>\n\n")

		# write out the physics geometry

		elif obj.type == 'CURVE':
			curve = obj.data
			file.write("<physics_shape>\n")
			for bp in curve.splines[0].bezier_points:
				file.write("\t<pos>%.6f %.6f</pos>\n" % (bp.co[0], bp.co[1]))
			file.write("</physics_shape>\n")


	file.write("</static_mesh>\n")
	file.close()
