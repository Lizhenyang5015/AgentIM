# Build, output, install, and debug

## Directory contract

```text
out/debug/ or out/release/
├─ CMakeCache.txt       CMake configuration cache
├─ CMakeFiles/          CMake internal metadata
├─ build.ninja          Ninja build graph
├─ web_apps/            Independently built Web application packages
├─ src/                 Module intermediate files
└─ compile_commands.json

build/debug/ or build/release/
├─ bin/                 Local runnable target: AgentIM.app or AgentIM.exe
└─ lib/                 Static and shared library outputs

dist/<preset>/          `cmake --install` staging output for packaging
packaging/              Platform signing and installer rules
```

`out/`, `build/`, and `dist/` are generated and never committed, but remain
visible and searchable like every other project directory. `out/` owns all
CMake/Ninja internals and compiler intermediate files. `build/` intentionally
contains only products that developers run, link, or inspect directly.

Qt automatic code generation remains enabled for the desktop application so
future `Q_OBJECT`, `.ui`, and Qt resource inputs work without per-file setup.
CEF helper subprocess targets explicitly disable Qt automatic generation
because they contain no Qt meta-object, UI form, or Qt resource sources.

## macOS development flow

```bash
cmake --preset macos-debug
cmake --build --preset macos-debug
cmake --install out/debug
```

This is also the clean-clone workflow. The first configure restores and
checksum-verifies missing Ninja, Qt Base, and CEF packages. Later configures
reuse the local tools, SDKs, and download cache. No global Ninja/Qt/CEF
installation or separate bootstrap command is required. A C++ compiler and
CMake 3.25+ are host prerequisites.

## Windows development flow

Install CMake 3.25+ and Visual Studio Build Tools with the **Desktop development
with C++** workload, a Windows SDK, and English (en-US) language resources for
the C++ toolchain. Windows presets set `VSLANG=1033` so MSVC's `/showIncludes`
output stays in English for Ninja's header-dependency parsing. Open an **x64
Native Tools Command Prompt** or **Developer PowerShell for VS**, then run from
the repository root:

```powershell
cmake --preset windows-debug
cmake --build --preset windows-debug
.\build\debug\bin\agentim_desktop.exe
```

If using the optional project-local CMake, replace `cmake` in these commands
with `.\tools\cmake\windows-x64\bin\cmake.exe`. The tool layout is documented
in `tools/README.md`.

The first configure restores the same pinned dependencies as the macOS flow.
After linking, the Windows build runs `windeployqt.exe` from `AGENTIM_QT_ROOT`,
with that SDK's explicit `qtpaths.exe`, to stage the matching Debug or Release
Qt DLLs and plugins beside the application. No global Qt path or separate
manual deployment command is required. CEF DLLs, resources, and locales are
staged separately, together with the packaged Web workbench.

The Windows CEF minimal SDK supplies only Release binaries. Both Debug and
Release application builds therefore link `Release/libcef.lib`; the official
C++ wrapper is compiled locally with `/MDd` for Debug and `/MD` for Release to
match Qt's shared MSVC runtime. CEF's wrapper-specific flag setup is kept in
its subdirectory, and the normal CMake C++ flags are restored for Qt and the
application. `AgentIM::CEF` also supplies `NOMINMAX`, `WIN32_LEAN_AND_MEAN`,
and an 8 MiB main-thread stack.

Development builds set CEF's `USE_SANDBOX=OFF`, matching the existing runtime
`no_sandbox` setting. Release packaging must establish and verify its sandbox
policy before distribution. These integration settings are maintained in
`cmake/AgentIMDependencies.cmake`; vendor SDK sources remain unmodified.

Windows x64 Debug configuration, wrapper/application compilation, automatic
Qt/CEF runtime staging, and launch are verified with MSVC 19.44.35229 (Visual
Studio Build Tools 17.14.42) and Windows SDK 10.0.26100.0. Manual inspection
confirmed the Qt main window and embedded Workbench content are visible.
After installing English C++ resources, a fresh configuration and clean build
produced valid Ninja header dependencies, and a subsequent build performed no
C++ recompilation; Workbench resource staging still runs.

The high-DPI viewport sizing change has been rebuilt and manually verified on
the current scaled display: Workbench fills its Qt host at startup and resizes
correctly after maximizing the window, with the input area fully visible.
Moving between displays with different scaling has not been checked. The
viewport sizing contract is maintained in `src/web_cef/README.md`.

Release builds, `cmake --install`, signing, installer production, and an enabled
CEF sandbox have not been verified.

## Dependency bootstrap

Missing Ninja, Qt, and CEF packages are downloaded during normal project
configuration. The scripts below remain available for explicit maintenance and
diagnostics:

```bash
cmake -P tools/fetch_ninja.cmake
cmake -P tools/fetch_qt.cmake
cmake -P tools/fetch_cef.cmake
```

On Windows:

```powershell
cmake `
  -DAGENTIM_CEF_PLATFORM=windows-x64 `
  -P tools\fetch_cef.cmake
```

Set `AGENTIM_AUTO_FETCH_DEPENDENCIES=OFF` only for an intentionally offline or
pre-provisioned build. In that mode, `AGENTIM_QT_ROOT` and `AGENTIM_CEF_ROOT`
must identify complete SDKs.

The script verifies the official versioned SHA-1 sidecar and stores the vendor
archive, sidecar, and extracted SDK together under
`third_party/cef/<platform>/<version>/`. `AGENTIM_CEF_ROOT` selects the
extracted `sdk` directory.

Shared configure presets set `AGENTIM_ENABLE_CEF=ON`. Configure therefore
validates the staged SDK and defines `AgentIM::CEF`; build compiles the
unmodified official `libcef_dll_wrapper` with the current toolchain. On macOS,
the wrapper output is `build/debug/lib/libcef_dll_wrapper.a`.

`agentim_desktop` links CEF and starts its browser process model. On macOS, the
build creates and embeds CEF's complete helper bundle set: the base, Alerts,
GPU, Plugin, and Renderer helpers. It also stages the CEF framework and
resources. The `agentim_web_workbench` target first builds the independently
versioned Workbench package under
`out/<configuration>/web_apps/workbench/dist`; native staging copies only this
artifact into the desktop application bundle. It never consumes Web source
files directly. `web_apps/CMakeLists.txt` discovers Web application
subdirectories, and each package target registers itself in
`AGENTIM_WEB_APP_TARGETS`. The native staging logic copies every registered
`dist` artifact under the application resource root by manifest `appId`.

At runtime, `web_cef` discovers and validates all staged manifests. The
embedded browser opens the application root `agentim://workbench/`; the
Workbench manifest, rather than C++, declares `index.html` as its entry point.
No HTTP listener, local port, Node.js service, or Qt Network dependency is
involved. The custom scheme is registered in the Browser and helper processes
before CEF creates a browser instance.

Qt drives CEF with `external_message_pump`. CEF schedules work through
`OnScheduleMessagePumpWork`; a guarded fallback timer prevents starvation and
also prevents re-entering `CefDoMessageLoopWork` while macOS is processing a
nested native event loop.

The Web application can also be built independently of the native shell:

```bash
cmake \
  -S web_apps/workbench \
  -B out/web-workbench
cmake \
  --build out/web-workbench
```

The result is `out/web-workbench/dist`. Its application manifest and
`package.json` versions are validated during configuration. No Node.js package
installation is required for this static application build.

The authored Web source can be previewed without the native shell:

```bash
cd web_apps/workbench
npm run serve
```

This preview is for Web-only iteration. The bundled path and CEF integration
must still be validated by building and launching `agentim_desktop`.

The macOS ARM64 configure and Debug build are verified.

The local macOS Debug bundle contains the CEF framework, CEF resources and
locales, the complete helper application set, and the local workbench. Release
installation must add complete Qt deployment verification, signing, hardened
runtime, notarization, and installer production before the bundle is
distributable.

## VS Code debugging

1. Install the recommended project extensions.
2. Run **AgentIM: Build macOS Debug** from VS Code Tasks, or run the commands above.
3. Choose **AgentIM: Debug on macOS** and press F5.

The debug configuration launches the application from `build/debug/bin` and
runs the matching build task first. CMake and Ninja operate in `out/debug`; the
workflow does not depend on a user-local CMake preset.
