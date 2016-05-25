README
---------------------------

Name: Colin Cammarano

Email: cammaran@usc.edu

Platform: Mac

Overview:
---------------------------

The purpose of this assignment is to generate a 3D heightmap from a grayscale JPEG image. When the program is run, the name of an image file must be passed as a command line argument.

Building the Application:
---------------------------

1: Make
 + Simply navigate to the "hw1-starterCode" folder and type "make." To remove the build directory, type "make clean."

2: Visual Studio
 + Open up the project in Visual Studio, then rebuild the application. Note, development was done on a Mac, so there was no way to test the Visual Studio workflow.

Running the Application:
---------------------------

In order to run the application, a single command line argument (the path to a JPEG file) must be specified when the program is run. By default, the program runs in display mode, which rotates the heightfield about the y-axis. To manually rotate, scale, or translate the mesh, press "tab" to switch to modify mode. The full list of commands are listed below.

 - 1: Triangle Draw Mode: Draws the mesh as grayscale triangles.
 - 2: Wireframe Draw Mode: Draws the mesh as a yellow wireframe.
 - 3: Point Draw Mode: Draws the mesh as a series of purple points.
 - 4: (EXTRA) Overlay Mode: Draws the mesh as grayscale triangles, then overlays a yellow wireframe on top of it.
 - x: Take screenshot
 - q: Start animation
 - Tab: Switch between display (rotation) and modify modes.
 - Control: Translate the mesh in modify mode.
 - Shift: Scale the mesh in modify mode.
 - Escape: Terminate the program.

Bugs and Failure Conditions:
---------------------------

None found so far.

Attributions:
---------------------------

The starter code for this project was provided by Dr. Jernej Barbic, PhD.
