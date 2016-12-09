bl_info = {
    "name": "Brick (static) mesh format (.brick)",
    "author": "Chandra Foxglove",
    "version": (0, 1),
    "blender": (2, 76, 0),
    "location": "File > Import-Export > Brick (.brick) ",
    "description": "Import-Export Brick (Static) Mesh Format",
    "warning": "",
    "wiki_url": "",
    "tracker_url": "",
    "category": "Import-Export"}

if "bpy" in locals():
    import imp
    if "import_brick" in locals():
        imp.reload(import_brick)
    if "export_brick" in locals():
        imp.reload(export_brick)
else:
    import bpy

from bpy.props import StringProperty, BoolProperty
from bpy_extras.io_utils import ExportHelper


class BrickImporter(bpy.types.Operator):
    """Load BRICK triangle mesh data"""
    bl_idname = "import_mesh.brick"
    bl_label = "Import BRICK"
    bl_options = {'UNDO'}

    filepath = StringProperty(
            subtype='FILE_PATH',
            )
    filter_glob = StringProperty(default="*.brick", options={'HIDDEN'})

    def execute(self, context):
        from . import import_brick
        import_brick.read(self.filepath)
        return {'FINISHED'}

    def invoke(self, context, event):
        wm = context.window_manager
        wm.fileselect_add(self)
        return {'RUNNING_MODAL'}


class BrickExporter(bpy.types.Operator, ExportHelper):
    """Save BRICK triangle mesh data"""
    bl_idname = "export_mesh.brick"
    bl_label = "Export BRICK"

    filename_ext = ".brick"
    filter_glob = StringProperty(default="*.brick", options={'HIDDEN'})

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
        from . import export_brick
        export_brick.write(self.filepath,
                         self.apply_modifiers,
                         self.triangulate,
                         )

        return {'FINISHED'}


def menu_import(self, context):
    self.layout.operator(BrickImporter.bl_idname, text="BRICK (.brick)")


def menu_export(self, context):
    self.layout.operator(BrickExporter.bl_idname, text="BRICK (.brick)")


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
