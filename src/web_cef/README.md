# web_cef

CEF container and web workbench: browser lifecycle, native bridge, web-resource loading, and security policies.

## Current status

The pinned macOS ARM64 and Windows x64 minimal SDKs are staged. Shared CMake
presets validate the selected SDK, compile the unmodified official
`libcef_dll_wrapper`, and expose `AgentIM::CEF`.

The module currently provides CEF process initialization, the complete macOS
helper bundle set required by CEF's process model, a Qt-hosted browser widget,
development runtime staging, and a custom packaged-resource scheme for the
independently maintained applications under `web_apps`. A manifest-driven
registry validates and serves every staged application through
`agentim://<appId>/`. The native shell selects an application by `appId`; the
manifest owns its entry point and the Web application owns internal routes. No
HTTP server or local port is created. No JavaScript/native bridge or business
capability endpoint exists yet.

The Workbench owns the `agentim_web_workbench` build target and produces a
self-contained, versioned `dist` package. Every Web application build target
publishes its package directory and `appId`, then registers itself in the global
`AGENTIM_WEB_APP_TARGETS` property. `web_cef` stages all registered package
artifacts without application-specific C++ or staging code. It does not inspect,
copy, or serve an application's authored `src` directory.

Independently maintained Web applications are packaged resources hosted by the
shared AgentIM CEF runtime. They do not create independent native application
processes, Qt event loops, or CEF Browser-process message pumps. Their HTML,
CSS, and JavaScript execute in CEF-managed Renderer processes, while AgentIM's
Qt event loop continues to drive the shared Browser-process integration.

## Ownership boundary

Sensitive credentials, database access, and file-system capabilities remain in the native layer; the web layer accesses capabilities through a versioned, asynchronous, authorized bridge.

This module owns:

- CEF initialization, shutdown, browser instances, and renderer-process IPC;
- Qt/CEF embedding and focus, input, resize, and lifecycle coordination;
- origin and capability allowlists, request validation, and bridge versioning;
- application web-resource loading and navigation policy;
- conversion between bridge messages and the `logic` facade.

It does not own business workflows, database or transport access, credentials,
or platform packaging. Application and UI targets must not refer to CEF paths
directly, and this module must link only the maintained `AgentIM::CEF` target.

The custom scheme handler resolves immutable staged application assets only.
It accepts `GET` requests for registered application origins, maps `/` to the
manifest entry point, supports entry-point fallback for SPA main-frame routes,
prevents path traversal, and never binds a network interface. The Web entry
point supplies the restrictive content security and referrer policies. The
scheme handler must not grow business or administrative endpoints.

Dependency version and runtime inventory are defined in
`third_party/manifests/cef.json`; acquisition and directory policy are defined
in `third_party/README.md`.
