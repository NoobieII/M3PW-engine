M3PW Engine
July 23, 2022

This is NoobieII's new game engine, which uses the SDL libraries, OpenGL and GLEW. It is designed to be used for both 2D and 3D settings.

Features:
3D Graphics
Shaders
Image Loading
Sound Loading (OGG Vorbis)
TCP Client Networking

Included are some test programs for editing 3D objects, playing sounds, and more. The engine is a work in progress. Much of it was originally written in June 2022.

The compilation commands are found in the editor1.c and editor2.c files.

Check out the game design document for this engine at https://drive.google.com/file/d/1Kr2dFUpHN9cRMbu158PVaX_MBrUGQgwr/view?usp=sharing

Installation Instructions

Windows:
The 32-bit EXE is already available if you only want to run the program.

The following are needed if you want to build the application yourself:
  SDL2
  SDL2 Image
  SDL2 TTF
  SDL2 Net
  GLEW

Linux:
Check your package distributor and install the following:
  libsdl2
  libsdl2-dev
  libsdl2-image-dev
  libsdl2-net-dev
  libsdl2-ttf-dev
  libglu1-mesa-dev

Build the GLEW source code found online https://glew.sourceforge.net/. It can be installed with the following commands:
  make
  sudo make install
  make clean

You may have to relocate the lib files located in usr/lib64 to usr/lib.

The compilation command is found in the editor1.c and editor2.c files.
