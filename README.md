# Audio-Server

Server software running on the Audio computer.

It communicates with the clients, configures the audio devices, and launches the audio apps.

## Building

### Prerequisites

_mrlabctrl_ needs a standard C++ toolchain to build and should compile with MSVC, clang, or gcc (on Windows, JUCE is reported to not anymore support the minGW gcc compiler).

On Windows, one way is to install _Visual Studio 2022 (Community)_ with the _Desktop Development with C++_ workload.

### Cloning the repository

Use your IDE's built-in _git_ facility or the command line to clone the repository:

```
git clone https://github.com/sonible/mrlab-audio-server.git
```

### Building from the command line

Note that on Windows with Visual Studio, the _Developer Command Prompt_ or the _Developer Power Shell_ should be used in order set the correct environment for the build toolchain.

_mrlabctrl_ uses `cmake` for building and includes _CMake_ preset definitions, so either way of standard cmake commands can be used.

#### Using CMake presets

`cd` into the repository directory.

This will configure and build the main app in _release_ configuration:

```
cmake --preset release
cmake --build --preset app-release
```

There are `debug` and `app-debug` presets for configuration and build in _debug_ configuration, respectively. The `test-release` will build the unit test binary.

The build folders used by the presets are `build-release` and `build-debug` for the respective configurations.

#### Using manual CMake commands

`cd` into the repository directory.

This will configure and build the default targets (i.e., the main app) in _release_ configuration in the `build` subdirectory, using the _Ninja_ generator:

```
cmake -B build -G Ninja -D CMAKE_BUILD_TYPE=Release
cmake --build build
```

### Building and running in Visual Studio

- If you already cloned the repository (e.g., using the git command line client or in a previous session):
  - Open the project using _Open a local folder_ from the Visual Studio startup screen or the _File > Open > Folder…_ menu entry.
  - _CMake_ configuration will be performed automatically using the above-mentioned _CMake_ presets.
- For cloning using Visual Studio's builtin git client:
  - At the Visual Studio startup screen, select _Clone a repository_, enter the repository url (<https://github.com/sonible/mrlab-audio-server.git>) and optionally adjust the local folder where to put the cloned repository.
  - Optionally, select the desired branch to use in the _Git Changes_ tab.
  - If _CMake_ configuration does not start automatically (appears to happen after initial checkout), open the project's _Folder View_ in the _Solution Explorer_ to initiate the configuration (or close and re-open the repository folder).
- After successful _CMake_ configuration, build presets can be selected in the top-central dropdown menu.
- Build the current target using `Ctrl+B`, start a debug run with `F5`, or run without debugging with `Ctrl+F5`.

### Enabling dependency source caching

The project uses [CPM](https://github.com/cpm-cmake/CPM.cmake) for dependency management. The provided _CMake_ presets include dependency source caching in the `.cache/CPM` subdirectory in the user's home directory.

When using manual `cmake` commands (or manually configuring an IDE), it is recommended to specify a cache directory for CPM in order to avoid repeated downloading, e.g.:

```
cmake -B build -G Ninja -D CMAKE_BUILD_TYPE=Debug -D CPM_SOURCE_CACHE=$HOME/.cache/CPM -D CPM_USE_NAMED_CACHE_DIRECTORIES=1
```

Note that you might have to explicitly specify the full path for `CPM_SOURCE_CACHE` instead of using `$HOME` in case your IDE does not perform environment variable expansion.  The option `CPM_USE_NAMED_CACHE_DIRECTORIES` makes the dependency directories look better in IDEs like CLion.

Both options can also be set via environment variables of the same names (i.e., `CPM_SOURCE_CACHE`).

## Running

The app binary can be directly run from its build directory, e.g.

```
build-release/mrlabctrl_App_artefacts/Release/mrlabctrl
```

## Application configuration data and resources

The default base path for application resources on Windows is in `%APPDATA%/mrlabctrl`, which usually translates to the `AppData/Roaming/mrlabctrl` subdirectory in the user's home directory.

Below this base path, the following subdirectories are used:

- `webgui`: webserver document root for serving the web-based GUI to the control clients
- `apps`: configuration files for managed apps

### Webgui

The repository includes a very simple proof-of-concept webgui project in `resources/webgui_minimal`. Its contents need to be copied to the webserver document root (see above).

The default listening port of the webserver is `7080`, so pointing the browser to <http://localhost:7080> should show the webgui.
