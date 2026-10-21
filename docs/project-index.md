# AgentIM project index

This file is the maintained navigation index for AgentIM. It records project contracts and entry points; implementation details remain in the referenced source, CMake, and topic-specific documentation files.

## Documentation routing

| Topic | Document | Read when |
|---|---|---|
| Architecture and module dependencies | `docs/architecture.md` | Adding or changing modules, interfaces, or dependencies |
| C++ style, include order, and Qt ownership | `docs/coding-conventions.md` | Creating or editing C++ or Qt UI code |
| Build, output, installation, and debugging | `docs/build-and-debug.md` | Changing or using CMake, build, install, or debugger workflows |
| Figma design integration | `docs/design-integration.md` | Using or changing the design-to-code integration |
| Third-party SDK layout and acquisition | `third_party/README.md` | Adding, restoring, or upgrading a dependency SDK |
| CEF module boundary and bridge | `src/web_cef/README.md` | Implementing browser lifecycle, embedding, or Web/native communication |
| Local Web workbench | `web_apps/workbench/README.md` | Developing or packaging the independently maintained local Web application |
| Platform runtime integration | `platform/macos/README.md`, `platform/windows/README.md` | Adding helper processes, native integration, or release staging |
| Build, dependency, test, and packaging contracts | This document | Changing project-wide contracts or validating delivery requirements |

## Architecture

Dependency direction is strictly bottom to top:

```text
base
  -> model / protocol
  -> storage / network
  -> logic
  -> ui_qt / web_cef / platform
  -> app
```

`logic` is the only application-service entry for UI and Web code. UI, Web, and `app` must not directly access storage, network, or protocol implementations.

Web applications are independently maintained and packaged, but run in
CEF-managed Renderer processes hosted by AgentIM's shared main-process CEF
runtime. They do not own separate Qt or CEF Browser-process message loops. See
`docs/architecture.md` for the runtime and isolation contract.

| Area | Status | Responsibility | Entry point |
|---|---|---|---|
| `base` | Active | Cross-cutting utilities and platform-neutral interfaces | `src/base/CMakeLists.txt` |
| `model` | Planned | Stable, serializable domain entities | `src/model/README.md` |
| `protocol` | Planned | Wire formats, codecs, and version compatibility | `src/protocol/README.md` |
| `storage` | Planned | SQLite, migrations, cache, and local file indexes | `src/storage/README.md` |
| `network` | Planned | HTTP, long connection, transfer, and retry transport | `src/network/README.md` |
| `logic` | Planned | Unified application services over lower layers | `src/logic/README.md` |
| `ui_qt` | Active | Qt Widgets desktop UI, on the Qt main thread | `src/ui_qt/CMakeLists.txt` |
| `web_cef` | Active | CEF lifecycle, Qt browser container, packaged-resource loading, and future authorized native bridge | `src/web_cef/README.md` |
| `platform` | Planned | macOS/Windows-only implementations of base interfaces | `platform/` |
| `app` | Active | Process entry point and executable composition | `src/app/CMakeLists.txt` |

## Code conventions

See `docs/coding-conventions.md` for the C++20 baseline, include order, and Qt ownership rules. New modules must preserve the architecture dependency direction and be added through the relevant parent `CMakeLists.txt`.

## Build and dependency contract

| Purpose | Command or location |
|---|---|
| macOS Debug configure | `cmake --preset macos-debug` |
| macOS Debug build | `cmake --build --preset macos-debug` |
| Debug CMake workspace | `out/debug` |
| Release CMake workspace | `out/release` |
| Debug products | `build/debug/bin`, `build/debug/lib` |
| Release products | `build/release/bin`, `build/release/lib` |
| Application target | `agentim_desktop` |
| Dependency root selection | `cmake/AgentIMDependencies.cmake` |
| Clean-clone tool/dependency restoration | Automatic during configure (`AGENTIM_AUTO_FETCH_BUILD_TOOLS`, `AGENTIM_AUTO_FETCH_DEPENDENCIES`) |
| Qt SDK CMake root | `AGENTIM_QT_ROOT` |
| Tool/SDK maintenance scripts | `tools/fetch_ninja.cmake`, `tools/fetch_qt.cmake`, `tools/fetch_cef.cmake` |
| CEF SDK CMake switch | `AGENTIM_ENABLE_CEF` (enabled by shared presets) |
| CEF platform/version manifest | `third_party/manifests/cef.json` |
| CEF consumer target | `AgentIM::CEF` |
| Web app discovery | `web_apps/CMakeLists.txt` |
| Local Web app definition | `web_apps/workbench/CMakeLists.txt`, `web_apps/workbench/app.manifest.json` |
| Local Web app build target | `agentim_web_workbench` |
| Integrated Web app artifact | `out/<configuration>/web_apps/workbench/dist` |
| Standalone Web app artifact | `out/web-workbench/dist` |
| Local Web app runtime URL | `agentim://workbench/` |
| Build presets | `CMakePresets.json` |

Third-party paths and discovery belong in `cmake/AgentIMDependencies.cmake`.
Modules declare only the targets they use through `target_link_libraries()`;
do not link every dependency globally. Missing Qt Base and CEF SDKs are restored
and checksum-verified automatically during a normal configure; no separate
developer bootstrap step is part of the clean-clone workflow. CEF is pinned to
the official minimal binary SDK and exposed to future CEF consumers as
`AgentIM::CEF`. The SDKs are staged in the maintained dependency tree. The
macOS wrapper build, complete helper bundle set, runtime staging, custom Web
resource scheme, manifest-driven application registry, Qt browser container,
and independent Workbench package target have been verified. Native staging
discovers registered Web application targets, consumes each `dist` artifact,
and never reads a Web source directory at runtime. C++ selects an application
by stable `appId`; the package manifest owns its entry point. Windows
compilation must be verified on a Windows host with the MSVC toolchain.

## Test contract

No automated test target exists yet. When a testable non-UI module is introduced:

1. Add its tests under `tests/`.
2. Register the test target with CTest.
3. Add its build and run command to this index and `docs/build-and-debug.md`.
4. Run the affected tests with every implementation change.

Until then, every code change must at minimum build the affected Debug preset. UI changes must also be launched and checked manually.

## Packaging contract

`packaging/` owns deployment, signing, installer, and symbol-archive rules. Platform-specific packaging details belong under `platform/macos` and `platform/windows`.

The local macOS build stages the CEF framework, resources, base, Alerts, GPU,
Plugin, and Renderer helper bundles, and the local Web application inside the
application bundle. Release-grade Qt/CEF deployment verification, code signing,
notarization, installer generation, and the equivalent Windows runtime
validation remain pending. Any change that introduces them must document the
exact staging, signing, and verification commands in `docs/build-and-debug.md`
and this index.

## Index maintenance triggers

Update this file when any of the following changes:

- module responsibility, dependency direction, or CMake entry point;
- third-party dependency, version, or discovery path;
- build preset, output directory, debug command, or toolchain contract;
- test target, test command, or validation requirement;
- packaging, runtime deployment, signing, or installation workflow.
