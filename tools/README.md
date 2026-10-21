# Tools

Development and release utilities: code generation, formatting, dependency
bootstrap, symbol processing, and local diagnostics.
## Project-local build tools

`tools/` holds fixed build-tool binaries. They are separate from `third_party/`,
which holds libraries linked by AgentIM.

The source repository tracks utility scripts and this documentation, but
ignores locally staged CMake, Ninja, and downloaded tool archives. These binary
tool distributions must be restored from the controlled development artifact
source on a clean checkout and must not be force-added to ordinary Git history.

| Tool | macOS ARM64 path | Windows x64 path |
|---|---|---|
| CMake | `tools/cmake/macos-universal/bin/cmake` | `tools/cmake/windows-x64/bin/cmake.exe` |
| Ninja | `tools/ninja/macos-arm64/ninja` | `tools/ninja/windows-x64/ninja.exe` |

VS Code extensions are recommended in `.vscode/extensions.json`. CMake Tools
uses the selected CMake Configure Preset; clangd reads
`out/debug/compile_commands.json`; CodeLLDB is used on macOS and the Microsoft
C/C++ extension provides the Windows Visual Studio debugger integration.

## Platform prerequisites

- **macOS**: Xcode Command Line Tools supplies Apple Clang and LLDB. Run the
  CMake executable from `tools/cmake/macos-universal/bin/` after it is staged.
- **Windows**: install Visual Studio Build Tools with the **Desktop development
  with C++** workload and a Windows SDK. Stage the matching CMake, Ninja and Qt
  Windows SDK at the paths above, then select `windows-debug` in CMake
  Tools.

The `.vscode/tasks.json` file contains explicit configure/build tasks for both
platforms. CMake Tools can provide the same workflow through its status bar
after its `cmake.cmakePath` user setting points at the platform-local CMake
executable.

## CEF SDK bootstrap

`fetch_cef.cmake` downloads the pinned official CEF minimal binary distribution,
verifies its versioned SHA-1 sidecar, and keeps the archive plus extracted SDK
under one `third_party/cef/<platform>/<version>/` directory.
It is an explicit dependency bootstrap step and is never run by normal CMake
configure or build commands.

```bash
tools/cmake/macos-universal/bin/cmake -P tools/fetch_cef.cmake
```

Use `-DAGENTIM_CEF_PLATFORM=windows-x64` on Windows. Networks that block the
official distribution host must provide a trusted mirror through
`-DAGENTIM_CEF_BASE_URL=<mirror-url>`.

Both supported platform SDKs are currently staged. Run the bootstrap only to
restore a missing SDK or to stage a newly pinned manifest version. Shared CMake
presets enable CEF discovery; the bootstrap itself is never part of configure,
build, or packaging.
