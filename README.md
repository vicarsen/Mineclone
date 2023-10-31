# Mineclone
Mineclone is a personal project for me to explore voxel game design.

# Features
Currently, the game/game engine has the following features:
 - 32x32x32 chunks
 - chunk lazy meshing
 - multithreading - separate threads for game logic, rendering, and world generation
 - 2D perlin noise generation with a compute shader
 - basic finite world generation
 - basic profiling system

Next, are the features that are a work in progress:
 - realtime visualization of data collected by the profiler

Next, are the features that are planned for the near future:
 - infinite world generation
 - separate thread for mesh generation (currently handled by the world generator)
 - ambient occlusion
 - transparent voxels
 - lighting and colored lights

Note: these lists may change with time as I make progress. For certain "checkpoints" of my progress you can check the branches named after the date when the last commit was made.

# Build
## CMake
The build system I am using is CMake and the easiest to build and run the project is using CMake.
To build the project for yourself clone the repository (whatever branch you like) with the command:
```bash
git clone https://github.com/vicarsen/Mineclone.git
```
Then make sure to initialize the submodules by cd-ing into the cloned repository and running the command:
```bash
git submodule update --init
```
Then, you can proceed to build the project like any other CMake project.
## Other build system
The sources of the project are in the "./src" directory, and the include files in the "./include" directory. Make sure to link against all the libraries found in "./vendor". Make sure that the folder "./assets" is, relative to the executable, at "../assets".

