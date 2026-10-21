# AgentIM architecture and directory conventions

## Dependency direction

```text
base
  -> model / protocol
  -> storage / network
  -> logic (unified application service interface)
  -> ui_qt / web_cef
  -> app
```

`platform/windows` and `platform/macos` implement cross-platform interfaces only and must not leak platform details to higher layers.

## Module principles

- Each independently compiled target has its own `CMakeLists.txt`.
- Application modules link only CMake targets and never refer directly to third-party installation paths.
- `base` does not depend on application, UI, networking, or storage modules.
- `logic` is the only application entry point for higher layers and encapsulates storage, network, protocol, and model.
- UI and web layers do not directly access local databases, networking implementations, protocol details, or credentials.

## Native UI and CEF boundary

The Qt layer owns the native application shell, windows, navigation, and native
interaction. `web_cef` owns CEF lifecycle, browser views, renderer integration,
Web/native messaging, web-resource loading, and browser security policy. The
`app` target composes these modules but contains no business implementation.

The workbench is an independently maintained local Web application. Its source,
manifest, version, build target, and deployable package live under
`web_apps/workbench`. The Web build generates a self-contained `dist` artifact;
the native build stages only that artifact into the desktop bundle. The generic
Web application registry discovers staged packages, validates each manifest,
and exposes each package as an isolated `agentim://<appId>/` origin. C++ selects
an application by stable `appId`; the manifest owns its entry point and the Web
application owns all internal navigation. CEF loads the custom URL instead of
starting a local HTTP service, contacting a remote application server, or using
a `file://` URL.

Independent Web application ownership is a source, package, version, and
release boundary; it is not a separate native CEF host process. AgentIM owns
the shared CEF runtime and Browser-process integration. Each packaged Web
application is loaded into a CEF browser instance and executes in a
CEF-managed Renderer process. A Web application must not initialize its own
`QApplication`, Qt event loop, `CefRuntime`, or Browser-process message pump.

```text
AgentIM main process
  Qt application event loop
    -> shared CefRuntime and Browser-process message pump
       -> workbench CefBrowser
       -> future mail CefBrowser
       -> future document CefBrowser
            |
            | CEF-managed IPC
            v
       CEF Renderer process(es)
            -> independently packaged Web applications
```

The Qt event loop remains required because it owns the native shell and drives
the shared CEF Browser-process work through the external message pump. Each
Renderer process owns its Chromium/Blink/V8 task and JavaScript event loops.
Creating one browser instance per Web application does not by itself guarantee
one Renderer process per application; process isolation, request contexts,
origins, storage partitions, and capability scopes must be defined explicitly
when stronger application isolation is introduced.

```text
web_apps/workbench/src + app.manifest.json
          |
          | agentim_web_workbench
          v
versioned dist artifact
          |
          | native package staging
          v
application resources (immutable Web package)
          |
          | manifest-driven Web application registry
          v
agentim://workbench/
          |
          | manifest entryPoint -> index.html
          |
          v
CEF browser view inside the Qt shell
```

The custom scheme handler is only a packaged static-resource transport. It does
not start a server, bind a network interface, expose business APIs, or replace
`logic`. Browser JavaScript cannot bootstrap its own resource server because it
must first be loaded and cannot listen on a local TCP port. Future Web code that
needs native capabilities must use the authorized `web_cef` bridge and the same
`logic` facade as native Qt UI.

Web source directories are authoring inputs and are never a native runtime
contract. The application manifest is packaged with the assets. The Web
application registry validates identity, schema version, runtime, entry point,
origin directory, and path safety before creating a browser instance. Requests
for an application root resolve to its manifest entry point. Unknown main-frame
paths fall back to that entry point so the Web application may own client-side
SPA routing; missing subresources do not receive that fallback.

Adding another packaged Web application must not require a new C++ scheme
handler. The application contributes its own build target and manifest, appends
the target to `AGENTIM_WEB_APP_TARGETS`, and is then staged and served by the
same native framework.

The intended communication path is:

```text
Qt UI ---------> logic facade ---------> storage / network / protocol
                    ^
                    |
CEF renderer -> web_cef bridge
```

- JavaScript sends typed, versioned, asynchronous requests through the bridge.
- `web_cef` validates request origin, capability, payload, and session state,
  then calls only the `logic` facade.
- Logic results and domain events return through `web_cef`; renderer code never
  receives database handles, native paths, credentials, or transport objects.
- CEF browser/renderer process messaging is contained within `web_cef` and does
  not become a project-wide IPC API.
- Platform-specific helper-process and bundle details stay under `platform/`
  and are exposed through cross-platform interfaces.

CEF lifecycle, the complete macOS helper process set, Qt embedding, packaged
resource scheme, and development runtime staging are implemented. The native
bridge and its capability authorization remain planned. Release signing,
notarization, and Windows runtime validation remain platform delivery work.
