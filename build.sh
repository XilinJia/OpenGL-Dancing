#!/bin/bash

CC=g++
CPPFLAGS=`pkg-config --cflags glew ImageMagick++ freetype2 glfw3`
CPPFLAGS="$CPPFLAGS -I../Include -I../Common/FreetypeGL"
LDFLAGS=`pkg-config --libs glew ImageMagick++ freetype2 glfw3`
LDFLAGS="$LDFLAGS -lglut ../Lib/libAntTweakBar.a -lX11"

$CC dancing.cpp ../Common/ogldev_util.cpp ../Common/pipeline.cpp ../Common/math_3d.cpp ../Common/camera.cpp ../Common/ogldev_atb.cpp ../Common/ogldev_texture.cpp ../Common/3rdparty/stb_image.cpp $CPPFLAGS $LDFLAGS -o dancing
