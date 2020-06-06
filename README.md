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
This project depends on GTest for unit testing, and it has it as git submodule. So, you should use the following command to clone the project:

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

For example, you can install all dependecies in Ubuntu with apt:
```
sudo apt install cmake libsdl2-dev libsdl2-mixer-dev libsdl2-image-dev libsdl2-ttf-dev g++ doxygen graphviz
```

Once you have all mandatory dependencies installed, just run the following commands in your cloned directory:

```
cmake .
make
sudo make install
```

#### Windows

There is a VS2019 solution at `msvc/2019/naive_gbe/naive_gbe.sln`. You can use it for build the project. The dependecies on this solution will be resolved by NuGet.

### References
http://marc.rawer.de/Gameboy/Docs/GBCPUman.pdf

https://www.pastraiser.com/cpu/gameboy/gameboy_opcodes.html

http://gameboy.mongenel.com/dmg/asmmemmap.html

https://gbdev.gg8.se/wiki/articles/Gameboy_Bootstrap_ROM

https://realboyemulator.wordpress.com
