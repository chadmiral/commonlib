bl_info = {
    "name": "Skeleton Rig (.bones)",
    "author": "Chandra Foxglove",
    "version": (0, 1),
    "blender": (2, 76, 0),
    "location": "File > Import-Export > Skeleton (.bones) ",
    "description": "Import-Export Bones Skeleton Format",
    "warning": "",
    "wiki_url": "",
    "tracker_url": "",
    "category": "Import-Export"}

if "bpy" in locals():
    import imp
    if "import_bones" in locals():
        imp.reload(import_brick)
    if "export_bones" in locals():
        imp.reload(export_brick)
else:
    import bpy

from bpy.props import StringProperty, BoolProperty
from bpy_extras.io_utils import ExportHelper


class BonesImporter(bpy.types.Operator):
    """Load BONES skeleton data"""
    bl_idname = "import_mesh.brick"
    bl_label = "Import BONES"
    bl_options = {'UNDO'}

    filepath = StringProperty(
            subtype='FILE_PATH',
            )
    filter_glob = StringProperty(default="*.bones", options={'HIDDEN'})

    def execute(self, context):
        from . import import_brick
        import_brick.read(self.filepath)
        return {'FINISHED'}

    def invoke(self, context, event):
        wm = context.window_manager
        wm.fileselect_add(self)
        return {'RUNNING_MODAL'}


class BonesExporter(bpy.types.Operator, ExportHelper):
    """Save BONES skeleton data"""
    bl_idname = "export_skeleton.bones"
    bl_label = "Export BONES"

    filename_ext = ".bones"
    filter_glob = StringProperty(default="*.bones", options={'HIDDEN'})

    apply_modifiers = BoolProperty(
            name="Apply Modifiers",
            description="Use transformed mesh data from each object",
            default=True,
            )
    triangulate = BoolProperty(
            name="Triangulate",
            description="Triangulate quads",
            default=True,
            )

    def execute(self, context):
        from . import export_bones
        export_bones.write(self.filepath,
                         self.apply_modifiers,
                         self.triangulate,
                         )

        return {'FINISHED'}


def menu_import(self, context):
    self.layout.operator(BonesImporter.bl_idname, text="BONES (.bones)")


def menu_export(self, context):
    self.layout.operator(BonesExporter.bl_idname, text="BONES (.bones)")


def register():
    bpy.utils.register_module(__name__)

    bpy.types.INFO_MT_file_import.append(menu_import)
    bpy.types.INFO_MT_file_export.append(menu_export)


def unregister():
    bpy.utils.unregister_module(__name__)

    bpy.types.INFO_MT_file_import.remove(menu_import)
    bpy.types.INFO_MT_file_export.remove(menu_export)

if __name__ == "__main__":
    register()
