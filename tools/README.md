# Tools

Development and release utilities: code generation, formatting, dependency
bootstrap, symbol processing, and local diagnostics.
## Project-local build tools

`tools/` holds fixed build-tool binaries. They are separate from `third_party/`,
which holds libraries linked by AgentIM.

The source repository tracks utility scripts and this documentation, but
ignores locally staged CMake, Ninja, and downloaded tool archives. A developer
may use any CMake 3.25+ installation to run the checked-in presets. Once CMake
starts, AgentIM automatically restores its pinned Ninja generator and library
SDKs; developers do not prepare Ninja, Qt, or CEF manually.

| Tool | macOS ARM64 path | Windows x64 path |
|---|---|---|
| Optional local CMake | `tools/cmake/macos-universal/bin/cmake` | `tools/cmake/windows-x64/bin/cmake.exe` |
| Ninja | `tools/ninja/macos-arm64/ninja` | `tools/ninja/windows-x64/ninja.exe` |

VS Code extensions are recommended in `.vscode/extensions.json`. CMake Tools
uses the selected CMake Configure Preset; clangd reads
`out/debug/compile_commands.json`; CodeLLDB is used on macOS and the Microsoft
C/C++ extension provides the Windows Visual Studio debugger integration.

## Platform prerequisites

- **macOS**: install CMake 3.25+ and Xcode Command Line Tools, which supplies
  Apple Clang and LLDB.
- **Windows**: install Visual Studio Build Tools with the **Desktop development
  with C++** workload, a Windows SDK, and CMake 3.25+. Then select
  `windows-debug` in CMake Tools.

The `.vscode/tasks.json` file contains explicit configure/build tasks for both
platforms. CMake Tools can provide the same workflow through its status bar and
uses the CMake executable available on the developer's `PATH`.

## Dependency bootstrap

`fetch_ninja.cmake`, `fetch_qt.cmake`, and `fetch_cef.cmake` download the pinned
official binary distributions, verify their recorded checksums, and stage them
under `tools/` or `third_party/`. The root CMake flow invokes them automatically
when a normal configure detects a missing tool or SDK.

```bash
cmake -P tools/fetch_ninja.cmake
cmake -P tools/fetch_qt.cmake
cmake -P tools/fetch_cef.cmake
```

Use `-DAGENTIM_CEF_PLATFORM=windows-x64` on Windows. Networks that block the
official distribution host must provide a trusted mirror through
`-DAGENTIM_CEF_BASE_URL=<mirror-url>`.

Direct script execution remains available for dependency maintenance. It is not
part of the normal developer workflow.
