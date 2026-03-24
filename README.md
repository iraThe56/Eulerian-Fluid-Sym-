# READ ME
This code is a Eulerian fluid sum written In C++ a physics based Numerical methods class, It uses OpenGl for rendering to the screen and IMGUI for a basic user interface

Theoretically it should be possible to run it by cloning the repository into an IDE like Clion and letting it build, but it may require downloading requirements like opengl and IMGUI 

## configuration 
the code is spread across 3 main files, 
1. `Main.cpp` which renders to the screen and applies the simulation updates
2. `Fluidsim.cpp` that contains the buffers for the fluid sim and helper methods to run the simulation 
3. and `ImguiManager.cpp` which contains the customization for the right sided UI
![Wind tunnle image.png](resources/Wind%20tunnle%20image.png)