## Steps for building locally
![](https://github.com/jackb7890/butimagine/blob/main/hentie.gif)
### Download these things
- CMake: https://cmake.org/download/#latest
    - Download and run the x64 Windows Installer
    - This is for keeping track of our library dependencies (SDL2)
- Git: https://git-scm.com/downloads/win
    - Download and run the x64 Standalone Window Installer
    - This gets things
- Visual Studio: https://visualstudio.microsoft.com/downloads/
    - Download and install Community version of Visual Studio
    - After installing the installer, it will ask what packages you
    want with VS. Select C++ Desktop Development
    - This is a great product with really sexy devs

### Clone this repo
On the main page for this repo, click the green "code" button, and copy
the https url. Open a CMD prompt and go to folder where you want to put this.
When you're there, do:  
`git clone <the_url_u_copied>`

### And then do this
1. run the init.cmd script
2. run the setupBuild.cmd script

*init.cmd sets up the environment in your CMD prompt for using Visual Studio command line stuff. setupBuild.cmd only needs to be called when you don't already have a build folder set up. The build folder is a CMake thing and it's where the finished program + other shit will get put after you compile.*

### To compile everything in the project
you go to the build folder and run `cmake --build ..`


### Other things
- CMake is only here so that we don't have to carry around external libraries in our repo. It automatically downloads them for us. However, once you've built the project with cmake and installs the libraries, you can take the build command it uses and repurpose that to compile things using just the compiler and not have to worry about all the other shit CMake's doing. To get the build command from cmake, run it with the verbose option -- `cmake --build .. -v`. It'll have a ton of shit, most you don't need to worry about. But you can easily add and remove source files from the build command if you wanna build just small parts of the project. The reason why you need to copy from the cmake build command is because it has the library linking stuff in there that's annoying to find on your own.
