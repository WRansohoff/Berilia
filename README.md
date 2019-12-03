# Readme

This is a simplified rewrite of an old 3D OpenGL game engine that I wrote a couple of years ago to learn about graphical programming. I cleaned up the logic a bit and removed some features that I didn't quite complete (particle effects, non-photorealistic rendering) as well as VR support which relied on a hopelessly out-of-date OpenVR library.

I'd like to rewrite it in Rust and/or Vulkan. And you know what they say: if you try to write a game engine, you'll never get around to writing a game :)

# Building

MinGW, MSVC, and GCC compilation toolchains should all be supported by the CMake build system, but I've only tested it with GCC on Linux recently.

CMake will look for the following dependencies on the host system using its `find_package` directive. They are all required to build the application:

* AssImp ( https://github.com/assimp/assimp )

* Bullet ( https://github.com/bulletphysics/bullet3 )

* GLFW ( https://github.com/glfw/glfw )

* GLEW ( https://github.com/nigels-com/glew )

# Usage

`./main [arguments]`

Supported arguments:

`-e`: Start the engine in 'level editor mode'. This sets the physics engine to treat everything as static, and enables a barebones GUI for adding new game objects and modifying their properties. The resulting level can be saved to a JSON format to be loaded later.

`-l <file_path>`: Load a previously-saved file when starting the game.

# Known Issues

* The GUI system does not properly resize each panel's texture buffers when the window resizes. This doesn't seem to cause crashes or serious problems, but it can cause a lot of 'invalid value' OpenGL errors when you resize the window. That shouldn't be too hard to fix, but I'm starting to think that I would be better off using a 3rd-party GUI library instead of writing my own.

* Currently, the game runs with 'zero-gravity' physics; I had wanted to make a game where you worked on a spaceship, salvaging parts from wrecks that you docked with and piecing together the narratives of what doomed them, but then I got distracted. Anyways, the 'gravity' vector is set in the constructor of the `physics_manager` class, and it's easy to change.
