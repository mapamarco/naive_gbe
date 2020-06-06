# Game Boy Emulator
[![Build Status Travis](https://travis-ci.org/mapamarco/naive_gbe.svg?branch=master)](https://travis-ci.org/mapamarco/naive_gbe)
[![Build Status Appveyor](https://ci.appveyor.com/api/projects/status/github/mapamarco/naive_gbe?branch=master&svg=true)](https://ci.appveyor.com/project/mapamarco/naive-gbe)
[![Coverage Status](https://coveralls.io/repos/github/mapamarco/naive_gbe/badge.svg?branch=master)](https://coveralls.io/github/mapamarco/naive_gbe?branch=master)

WIP

### TODO
- [x] Instruction set
- [x] Travis CI integration
- [x] Appveyor CI integration
- [x] Unit tests
- [ ] Audio system
- [ ] Interruptions and timing
- [ ] Memory Mapping Unit
- [ ] Pixel Process Unit
- [ ] GUI
- [ ] Doxygen
- [ ] Documentation

### Cloning the project
This project depends on GTest for unit testing, and it has it as git submodule. So, you should use the following commands to clone the projectand its submodules:

```
git clone https://github.com/mapamarco/naive_gbe
cd naive_gbe
git submodule update --init --recursive
```

### Build

#### Linux

Dependencies:
* Mandatory
  * CMake
  * SDL2
  * SDL2-image
  * SDL2-ttf
  * SDL2-mixer
  * C++17 compiler (clang, g++)

* Optional
  * Doxygen

You can install all dependecies from Ubuntu:

```
sudo apt install cmake libsdl2-dev libsdl2-mixer-dev libsdl2-image-dev libsdl2-ttf-dev g++ doxygen graphviz
```

In case you are using a different distro or package manager, look at the documentation to see how to solve the dependencies.

Once all mandatory dependencies have been resolved, run the following commands from the cloned directory:

```
cmake . -DCMAKE_BUILD_TYPE=Release
make
make tests
sudo make install
```

#### Windows

There is a VS2019 solution at `msvc/2019/naive_gbe/naive_gbe.sln`. You can use it to build all the projects. 

All dependencies should be resolved automatically by the package manager NuGet.

### Assets
Icons made by [Freepik](https://www.flaticon.com/authors/freepik) from [Flaticon](https://www.flaticon.com/)

Menu sound [cabled_mess](https://freesound.org/people/cabled_mess/) from [Freesound](https://freesound.org/people/cabled_mess/sounds/350859/)

Font [Philipp Nurullin](https://www.fontsquirrel.com/fonts/list/foundry/philipp-nurullin) from [Fontsquirrel](https://www.fontsquirrel.com/fonts/jetbrains-mono/)

### References
http://marc.rawer.de/Gameboy/Docs/GBCPUman.pdf

https://www.pastraiser.com/cpu/gameboy/gameboy_opcodes.html

http://gameboy.mongenel.com/dmg/asmmemmap.html

https://gbdev.gg8.se/wiki/articles/Gameboy_Bootstrap_ROM

https://realboyemulator.wordpress.com
