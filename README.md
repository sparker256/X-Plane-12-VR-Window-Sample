# VR-Window-Sample

[![CMake](https://github.com/sparker256/X-Plane-12-VR-Window-Sample/actions/workflows/cmake.yml/badge.svg)](https://github.com/sparker256/X-Plane-12-VR-Window-Sample/actions/workflows/cmake.yml)

Using VR Window Sample code from

https://developer.x-plane.com/code-sample/vr-window-sample/

Create a CMake project with GitHub Actions

## Building VR-Window-Sample from source

### On windows

1.  Install required software using [Chocolatey](https://chocolatey.org/) using admin command prompt:

    ```
    choco install git cmake
    choco install mingw --version 8.1.0
    ```

    You can also install the same programs manually if you prefer.

2.  Checkout and configure the project:

    ```
    git clone https://github.com/sparker256/X-Plane-12-VR-Window-Sample.git
    cd X-Plane-12-VR-Window-Sample
    cmake -G "MinGW Makefiles" -S .\src -B .\build -DCMAKE_BUILD_TYPE=RelWithDebInfo
    ```

3.  Build the project and copy the plugin DLL into the appropriate directory:

    ```
    cmake --build .\build
    cp .\build\win.xpl .\VR-Window-Sample\64\
    ```

### On Ubuntu:

1. Install required software:

   ```
   sudo apt-get install -y --no-install-recommends build-essential cmake git freeglut3-dev libudev-dev libopenal-dev

   ```

2. Checkout and configure the project:

   ```
   git clone https://github.com/sparker256/X-Plane-12-VR-Window-Sample.git
   cd X-Plane-12-VR-Window-Sample
   cmake -S ./src -B ./build -DCMAKE_BUILD_TYPE=RelWithDebInfo
   ```

3. Build the project and copy the plugin DLL into the appropriate directory:

   ```
   cmake --build ./build
   cp ./build/lin.xpl ./VR_Window_Sample/64/
   ```

### On OS X:

1. Install XCode, Git, CMake (Homebrew can be convenient for this).

2. Checkout and configure the project:

   ```
   git clone https://github.com/sparker256/X-Plane-12-VR-Window-Sample.git
   cd X-Plane-12-VR-Window-Sample
   cmake -S ./src -B ./build -DCMAKE_BUILD_TYPE=RelWithDebInfo
   ```

3. Build the project and copy the plugin DLL into the appropriate directory:

   ```
   cmake --build ./build
   cp ./build/mac.xpl ./VR_Window_Sample/64/
   ```
