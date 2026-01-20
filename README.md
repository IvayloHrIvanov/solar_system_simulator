# Solar System Simulator
Required Software for this project is: 
  - Favorite text editor or IDE (Recommended: Visual Studio)
  - Git
<br/><br/>
# Instructions on how to run the application (with Visual Studio):
1. Install all of the required software
   - Install Visual Studio [Download here](https://visualstudio.microsoft.com/downloads/)
   - Git: [Download Here](https://git-scm.com/downloads)
<br/><br/>
2. Open Visual Studio Installer (should be downloaded together with Visual Studio IDE)
   - Go to Installed > Modify > Find and click "Desktop development with C++" > Install
3. Clone this repository [Tutorial Here](https://blog.hubspot.com/website/clone-github-repository)
<br/><br/>
4. Open Visual Studio > Click on "Open a project or solution" > Navigate to "solar_system_simulator/Solar System Simulator" > Open SolarSystemSimulator.sln
<br/><br/>
5. In the Visual Studio taskbar at the top go to: Tools > NuGet Package Manager > Manage NuGet Packages for Solution
<br/><br/>
6. Go to the "Browse" section, search for and install:
   - glew-2.2.0
   - glfw
   - glm
   - sdl2
7. In the Visual Studio taskbar at the top go to: Project > Properties > Linker > Input > Additional Dependencies > Edit
<br/><br/>
8. In the input field add at different lines:
   - opengl32.lib
   - glu32.lib
<br/><br/>
9. Click on "Ok" and "Apply"
10. You should now be able to run the simulator :)
