## Steps for building locally
![](https://github.com/jackb7890/butimagine/blob/main/hentie.gif)
### Download these things
- ~~CMake: https://cmake.org/download/#latest~~
    - DONT HAVE TO DOWNLOAD, IT COMES WITH VISUAL STUDIO APPARENTLY
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

#### For the following steps, open CMD prompt 

### Clone this repo
On the main page for this repo, click the green "code" button, and copy
the https url. Open a CMD prompt and go to folder where you want to put this.
When you're there, do:  
`git clone <the_url_u_copied>`

### And then do this
1. run the init.cmd script (you must do this everytime you start a new CMD prompt)
2. run the setupBuild.cmd script (you need to do this whenever you need to setup your build folder)  
(so if you clear out or delete the build folder, rerun this. However, if you build, make some changes, and build again, you don't have to run setupBuild again)


*init.cmd sets up the environment in your CMD prompt for using Visual Studio command line stuff. setupBuild.cmd only needs to be called when you don't already have a build folder set up. The build folder is a CMake thing and it's where the finished program + other shit will get put after you compile.*

### To compile everything in the project
you go to the build folder and run `cmake --build .`

### To run the program
the cmake build command above will output the program at *...\butimagine\build\Debug\butimagine.exe* if you wanna run it


### Other things
- CMake is only here so that we don't have to carry around external libraries in our repo. It automatically downloads them for us. However, once you've built the project with cmake and installs the libraries, you can take the build command it uses and repurpose that to compile things using just the compiler and not have to worry about all the other shit CMake's doing. To get the build command from cmake, run it with the verbose option -- `cmake --build . -v`. It'll have a ton of shit, most you don't need to worry about. But you can easily add and remove source files from the build command if you wanna build just small parts of the project. The reason why you need to copy from the cmake build command is because it has the library linking stuff in there that's annoying to find on your own.
- The tutorial i've been using for sdl2 https://thenumb.at/cpp-course/index.html

### Git command cheat sheet
- Saving new changes to your branch:
    1. **git add <filepath>** | this is where you select what files you are going to save (you don't have to save everything, just save what you want with **git add**)
    2. **git commit -m "some short description of the changes"** | this is the real "save" command. It saves it to your branch (locally on your computer only, this won't change the branch on github)
    3. **git push origin <branchname>** or **git push origin HEAD** to "push" the current branch you're on. This uploads your local branch to the branch on github. If these two branches are wildly different, git might complain that there are "conflicts", if that happens just text me im not putting into words. if ur not cringe it wont happen.
- Switching between branches::
    1. **git checkout <branchname>** | this will switch branches. If you have "unstaged changes or uncommitted changes" (this means changes that you havent **git add'd** yet), git may say you can't switch branches until you do something about those. Your options are either to add and commit the changes or to temporarily "stash" them. I'll go over that in one sec.
    2. **git checkout -b <newbranchname>** or **git checkout -b <newbranchname> <branchtocopyfrom>** | these commands will create a new branch and also switch to it. The first one will create a new branch that is a copy of all the commits in the current branch youre on. The second one will do the same thing except you can pick what branch your new branch is copying from without switching to it. The usual case is **git checkout my-new-branch main** to create branch called "my-new-branch" that is a replica of the main branch.
- The **git stash** command:
    1. This is the command referenced before. It saves any unstaged changes to something called the "stash" which you can think of as a "stack" from programming lingo (look it up bucko). This will allow you to switch branches when you have unstaged changes that you aren't ready to save yet. I try not to let stuff sit in the git stash cause i forgot about it, and if you bring the changes back on a different branch it can get messy (conflicts).
    2. **git stash pop** will bring the last git stash'd changes back to your branch. If you did two **git stash's** without popping, then you would get the most recent changes on the first pop, and then popping again will bring back the first **git stash'd** changes. (this is what makes it like a "stack")
- Getting branches from the github that you don't have on your compute:
    1. **git pull origin <branchname>** | don't use HEAD here if you were thinking about it, it doesn't do what you probably think. I'm not sure but I think you need to be on a branch with no unstaged changes before doing this. but maybe not idk

You could survive with just those commands, but i'll put a couple more that i spam for sanity. they are really just for getting information about the current state of your git stuff

- **git status**
    1. This will show you your unstaged changes (in red), staged changes (in green, these are the one's that just got **git add'd**), as well as untracked files/folders (also in red). If you do **git status** and see "working branch clean" that means you don't have any changes that are different from the current branch you are on (this is the state you usually wanna be in before switching branches, pushing, pulling, etc)
- **git log --oneline**
    1. This will show you the history of the current branch (the commits aka saves)
    2. It will also show you where your branch diverges from other branches, like main
- **git diff**
    1. This command is like **git status** but it shows you what the changes in the files actually are. **git status** with no extra arguments will just show you the unstaged changes (red ones from **git status**).
    2. **git diff --staged** will show you the staged changes (green ones from **git status**)
    3. Super helpful one is **git diff <commitID1> <commitID2>**. This will show you all the changes between two commits. The commitID is the hexnumber you see next to each commit when you do a **git log --oneline**. Remmeber to do the older commit first, then more recent one second. You can do this without looking up the commitIDs, too, using the "HEAD" keyword. HEAD corresponds to your current branch. If you wanted to see the differences between the last two commits on your branch, you can do **git diff HEAD\~1 HEAD**. This means "the difference between the commit 1 before the current one you are on, and the current one you are on." You can add \~N to go back as far as you want, you can do it for both commits you are diffing (ex: **git diff HEAD\~2 HEAD\~1**), and you can also look at the difference between two commits that aren't right after one another, and it will show you all the changes in the commits between them (ex: **git diff HEAD\~5 HEAD**) will show you what changed in the last 5 commits.
