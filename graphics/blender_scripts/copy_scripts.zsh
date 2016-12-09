#!/bin/zsh

#assumes the script path is already set (.bashrc / .zshrc / etc)
echo $BLENDER_SCRIPT_PATH

#mkdir $BLENDER_SCRIPT_PATH/io_brick
#mkdir $BLENDER_SCRIPT_PATH/io_skeleton
#mkdir $BLENDER_SCRIPT_PATH/io_anim

cp -f ./io_mesh/* $BLENDER_SCRIPT_PATH/io_brick
cp -f ./io_skeleton/* $BLENDER_SCRIPT_PATH/io_skeleton
cp -f ./io_anim/* $BLENDER_SCRIPT_PATH/io_anim
