# AgentIM Workbench Web application

This directory is the independent local Web application loaded by the AgentIM
CEF container. It has its own source, application identity, version, build
target, and deployable artifact. It does not call a remote application server
and does not depend on Qt implementation details.

Independence is a build and release boundary, not a native-process boundary.
AgentIM owns the shared CEF Browser process and native message pump. The Web
application runs in a CEF-managed Renderer process after AgentIM creates its
browser instance.

## Directory contract

```text
workbench/
├─ CMakeLists.txt        Standalone and integrated Web package build
├─ app.manifest.json     Stable application identity and runtime contract
├─ package.json          Web package metadata and developer commands
└─ src/                  Authored HTML, CSS, JavaScript, and assets
```

The build produces a self-contained `dist/` directory containing the entry
point, static assets, and `app.manifest.json`. Native code consumes only this
artifact; it must not copy or serve files directly from `src/`.

## Runtime contract

- `src/` is the authored static application source root.
- `app.manifest.json` defines the URL-safe `appId`, version, entry point,
  runtime, and the future native-capability declaration. It is the only owner
  of the entry-point filename.
- `agentim_web_workbench` builds the deployable `dist/` package and rejects a
  version mismatch between the manifest and `package.json`.
- The native build stages only `dist/` into the desktop application bundle.
- `web_cef` discovers the staged manifest and exposes the package through the
  isolated `agentim://workbench/` origin. The native caller selects only the
  `workbench` application ID; CEF does not start an HTTP server or load the
  application with a `file://` URL.
- The registry maps the application root to the manifest entry point and lets
  the Workbench own subsequent client-side routing.
- The CEF scheme handler serves read-only assets, rejects path traversal, and
  is registered and removed with the shared CEF runtime.
- Native services, model requests, and the versioned JavaScript bridge are not
  part of this milestone.

## Standalone build

From the AgentIM repository root on macOS:

```bash
tools/cmake/macos-universal/bin/cmake \
  -S web_apps/workbench \
  -B out/web-workbench
tools/cmake/macos-universal/bin/cmake \
  --build out/web-workbench
```

On Windows:

```powershell
tools\cmake\windows-x64\bin\cmake.exe `
  -S web_apps\workbench `
  -B out\web-workbench
tools\cmake\windows-x64\bin\cmake.exe `
  --build out\web-workbench
```

Both commands generate `out/web-workbench/dist`. The desktop build invokes the
same target inside its normal build graph and generates
`out/<configuration>/web_apps/workbench/dist` before staging the package.

For a browser-only preview during Web development, run:

```bash
npm run serve
```

Then open `http://127.0.0.1:4173`.

After a standalone build, `npm run preview` serves the built `dist` package
instead of the source tree. npm is a developer convenience only; neither the
desktop build nor the shipped application requires Node.js or npm.
