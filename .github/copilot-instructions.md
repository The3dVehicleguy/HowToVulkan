## HowToVulkan — Copilot instructions

Purpose: Provide concise, actionable guidance so an AI coding agent can be immediately productive in this repo.

- Repo snapshot: a minimal Vulkan tutorial app written in modern C++ (C++20). The runnable program is in `source/main.cpp`. Build is driven by CMake using `source/CMakeLists.txt`.

- Quick start (Windows, developer machine):
  1. Ensure the Vulkan SDK is installed and the `VULKAN_SDK` environment variable is set.
  2. Generate Visual Studio project files and build (from repo root):

     ```powershell
     cmake -S source -B build -G "Visual Studio 17 2022" -A x64
     cmake --build build --config Release
     ```

  3. The binary is emitted to `build/bin/<Config>/HowToVulkan(.exe)`. When running, the process expects to find the `assets/` folder in the working directory — the default VS debugger working dir is set to `source/`.

- Important runtime tips:
  - Set `VULKAN_SDK` before running. The CMake project reads this env var to include Vulkan headers and link libraries.
  - Run with the repository `source/` as working directory, or copy `source/assets` to the runtime working directory. Example run from repo root (PowerShell):

    ```powershell
    pushd source
    ..\build\bin\Release\HowToVulkan.exe  # optional argument: device index
    popd
    ```

- Build / configuration conventions to preserve:
  - CMake in `source/CMakeLists.txt` uses FetchContent for SFML and builds an internal static `ktx` lib from `external/ktx`.
  - Compiler flags: project targets C++20 and sets platform defines (e.g., `VK_USE_PLATFORM_WIN32_KHR` on Windows).
  - The main translation unit (`source/main.cpp`) contains implementation macros for third-party libs (e.g., `VOLK_IMPLEMENTATION`, `VMA_IMPLEMENTATION`). Avoid duplicating these implementations in new files unless intentionally moving them.

- Key integrations and files to inspect when modifying behavior:
  - `source/main.cpp` — central app logic, render loop, resource setup. Most changes touch this file.
  - `source/CMakeLists.txt` — controls dependency inclusion (Vulkan SDK, SFML via FetchContent, ktx library) and runtime output directory.
  - `source/assets/` — models (`.obj`), textures (`.ktx`), and `assets/shader.slang` (Slang shaders). Shaders are compiled/loaded via the Slang API at runtime (`slang::loadModuleFromSource`).
  - `external/ktx`, `external/tinyobj` — vendored third-party sources; modify only when you understand ABI/link impacts.

- Project-specific patterns and gotchas an agent should follow:
  - Single-binary, one-file example: most features live in `main.cpp`. Prefer small, incremental edits and keep API surface localized.
  - Assets must be reachable at runtime. The CMake project sets debugger working directory to `source/` to make this seamless in VS — mimic that when running from the terminal.
  - Shader workflow: `assets/shader.slang` is shipped as a source file and compiled/consumed at runtime by Slang; do not assume precompiled SPIR-V artifacts.
  - Device selection is optional: the program accepts a device index argument (see `main.cpp`). Useful for automated runs.

- Common developer workflows (what maintainers do):
  - Debug in Visual Studio using `build/HowToVulkan.sln` (open solution). VS uses `source` as working dir by default (see `DEBUGGER_WORKING_DIRECTORY`).
  - Edit C++ in `source/`, regenerate project files with CMake, rebuild, then run with `source` as working dir.

- When changing dependencies or build settings:
  - Update `source/CMakeLists.txt`. Keep FetchContent usage for SFML and preserve the `ktx` static target location.
  - Be mindful of the `VULKAN_SDK` dependency: if you add platform-specific code, gate it with the same `if(NOT IS_DIRECTORY $ENV{VULKAN_SDK})` checks.

- Tests / CI: none present. Avoid assuming test harnesses exist.

- Minimal checklist for pull requests an agent should help create:
  1. Small, focused change to `source/` only.
  2. Build locally with CMake and ensure the binary runs with `source` as working dir.
  3. If assets or shaders change, confirm runtime loads succeed (no missing-file errors).

If anything here is unclear or you want more detail about a specific workflow (packaging assets for CI, cross-platform build steps, or where to split `main.cpp` into modules), tell me which section to expand and I will iterate.
