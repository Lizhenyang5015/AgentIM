# AgentIM

AgentIM is a C++ cross-platform intelligent messaging desktop client for Windows and macOS.

## Project conventions

- C++20
- CMake Presets are the shared build configuration.
- Ninja is the cross-platform build backend.
- The project uses target-based CMake; modules are connected through target dependencies.
- The project maintains third-party binary SDKs, version manifests, and runtime packaging rules. Neither developers nor end users need a preinstalled Qt or CEF runtime.
- Qt and CEF are pinned project dependencies. CEF SDK discovery, its official C++ wrapper, the Qt browser container, local Web hosting, and development runtime staging are integrated.

## Directory layout

```text
AgentIM/
├─ src/             Cross-platform application source code
│  ├─ base/         Logging, configuration, threading, events, and platform abstractions
│  ├─ model/        Entities such as User, Conversation, and Message
│  ├─ protocol/     Client protocols and serialization
│  ├─ storage/      SQLite, caches, and file indexes
│  ├─ network/      HTTP, persistent connections, uploads, and downloads
│  ├─ logic/        Unified higher-level interface for login, synchronization, messaging, and lower-layer orchestration
│  ├─ ui_qt/        Native Qt UI
│  ├─ web_cef/      CEF and the native bridge
│  └─ app/          Final desktop application entry point
├─ platform/        Windows- and macOS-specific implementations
├─ web_apps/        Independently maintained local Web applications
├─ third_party/     Integration of CEF, vendor SDKs, and other special dependencies
├─ resources/       Native icons, translations, and configuration templates
├─ packaging/       Deployment, signing, and installer rules
├─ cmake/           Shared CMake modules
├─ docs/            Architecture and development documentation
├─ tools/           Build and release helper tools
├─ out/             CMake/Ninja workspaces and intermediate files
├─ build/           Local runnable binaries and linked libraries
├─ dist/            Installed files staged for packaging
└─ tests/           Unit, integration, and end-to-end tests
```

## Build workflow

The project maintains fixed-version Qt binary SDKs under
`third_party/qt/<platform-architecture>` and fixed-version CEF minimal SDKs under
`third_party/cef/<platform-architecture>/<version>/sdk`. The root CMake
configuration selects the matching SDKs for the current platform. On a clean
workspace, restore the CEF SDK before the first configure:

```bash
tools/cmake/macos-universal/bin/cmake -P tools/fetch_cef.cmake
```

Then run:

```bash
tools/cmake/macos-universal/bin/cmake --preset macos-debug
tools/cmake/macos-universal/bin/cmake --build --preset macos-debug
tools/cmake/macos-universal/bin/cmake --install out/debug
```

Use `windows-debug` on Windows. The application starts in `main()` at
`src/app/src/main.cpp`, initializes CEF and its packaged Web resource scheme, and
then creates `MainWindow` from `src/ui_qt` with the CEF browser as its content.

Shared presets enable CEF dependency validation and build the official
`libcef_dll_wrapper`. The local Web source is maintained under
`web_apps/workbench`. Its own `agentim_web_workbench` target generates a
versioned `dist` package, and the native build stages that artifact into the
desktop bundle. Native CMake discovers registered Web application targets and
never deploys files directly from Web source. At runtime, the generic CEF
registry validates each packaged manifest; native code selects an `appId`, the
manifest selects the entry point, and the Web application owns internal routes.

See `docs/build-and-debug.md` for build, output, installation, and VS Code debugging instructions.
