# Advanced Settlers 3 Stats
Read the stats and chosen races of a Settlers 3 game while running, with detailed statistics over time. Compared to other games, Settlers 3 has very limited statistics that only show a snapshot at the time you leave the game. It turned out that it is very easy to read the statistics. 

## Table of Contents

- [s3Reader](#s3reader)
  * [Build Instructions](build-instructions)
- [Plots](#plots)
- [Twitch Overlay](#twitch-overlay)

## s3Reader

The s3Reader reads the Settlers 3 statistics while the game is running. You must first start the game and then run the s3Reader as administrator. 

Note: 

- The s3Reader only works with the s3_alobby.exe 
- If you click inside the terminal window the program will be terminated

### Build Instructions

1. Requirements:

   - Visual Studio 2019
   - CMake 3.19+
   - LLVM (if you prefer Clang over the MSVC compiler)

2. Create a separate build directory

   ```shell
   $ mkdir build
   $ cd build
   ```

3.  Configure the project and generate a native build system
   
    ```shell
    $ cmake -G"Visual Studio 16 2019" -T ClangCL ..
    ```
    If you stick with the MSVC Compiler use
    ```shell
    $ cmake ..
    ```

4. Call the build system to compile/link the project

    ```shell
    $ cmake --build .
    ```

## Plots



## Twitch Overlay

