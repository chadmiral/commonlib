#!/bin/zsh

if [[ `uname` == `Darwin` ]]
  export BLENDER_SCRIPT_PATH=/Applications/blender/blender.app/Contents/Resources/2.76/scripts/addons/
else
  export BLENDER_SCRIPT_PATH=/cygdrive/c/Program\ Files/Blender\ Foundation/Blender/2.77/scripts/addons/
fi

echo $BLENDER_SCRIPT_PATH

mkdir $BLENDER_SCRIPT_PATH/io_brick
cp ./io_mesh/export_brick.py $BLENDER_SCRIPT_PATH/io_brick
cp ./io_mesh/import_brick.py $BLENDER_SCRIPT_PATH/io_brick
cp ./io_mesh/__init__.py $BLENDER_SCRIPT_PATH/io_brick

mkdir $BLENDER_SCRIPT_PATH/io_skeleton
cp ./io_skeleton/* $BLENDER_SCRIPT_PATH/io_skeleton
