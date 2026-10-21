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
tools/cmake/macos-universal/bin/cmake --preset macos-debug
tools/cmake/macos-universal/bin/cmake --build --preset macos-debug
tools/cmake/macos-universal/bin/cmake --install out/debug
```

## CEF dependency bootstrap

CEF is not downloaded during normal project configuration. The maintained
workspace currently contains the pinned minimal SDK for macOS ARM64 and Windows
x64. On a clean workstation or after deleting a staged SDK, restore the host
SDK explicitly before configuring:

```bash
tools/cmake/macos-universal/bin/cmake -P tools/fetch_cef.cmake
```

On Windows:

```powershell
tools\cmake\windows-x64\bin\cmake.exe `
  -DAGENTIM_CEF_PLATFORM=windows-x64 `
  -P tools\fetch_cef.cmake
```

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
tools/cmake/macos-universal/bin/cmake \
  -S web_apps/workbench \
  -B out/web-workbench
tools/cmake/macos-universal/bin/cmake \
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

The macOS ARM64 configure and Debug build are verified. The Windows SDK archive
and binary architecture are verified, but its wrapper build must run on a
Windows host with MSVC before Windows integration is considered complete.

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
