M3PW Engine
July 23, 2022

This is NoobieII's new game engine, which uses the SDL libraries, OpenGL and GLEW. It is designed to be used for both 2D and 3D settings.

Features:
3D Graphics
Shaders
Image Loading
Sound Loading (OGG Vorbis)
TCP Client Networking

The engine is a work in progress. Much of it was originally written in June 2022.

Installation Instructions

Windows: The following are needed:
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

Build the GLEW source code found online. It can probably be installed with the following commands:
make
sudo make install
make clean

You may have to relocate the lib files located in usr/lib64 to usr/lib.
