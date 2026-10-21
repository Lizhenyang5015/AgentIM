# Third-party dependencies

AgentIM maintains pinned, platform-specific binary SDKs. Application targets
resolve dependencies only from this directory and never require developers or
end users to install Qt, CEF, OpenSSL, or similar SDKs globally. Vendor sources
remain unmodified. The only source compiled from the CEF distribution is its
official C++ ABI wrapper, which CEF requires consumers to build with their own
toolchain.

```text
third_party/
├─ qt/            Qt Base prebuilt SDK
│  ├─ macos-arm64/
│  │  └─ 6.10.3/macos/
│  │     ├─ include/     Build-time headers
│  │     ├─ lib/         Frameworks/libraries and CMake package files
│  │     ├─ plugins/     Platform plugins
│  │     └─ bin/         Qt build and runtime tools
│  └─ windows-x64/
├─ cef/           CEF minimal binary SDK
│  ├─ macos-arm64/<version>/
│  │  ├─ *.tar.bz2       Verified vendor archive
│  │  ├─ *.sha1          Vendor checksum
│  │  └─ sdk/            Extracted build and runtime SDK
│  └─ windows-x64/<version>/
├─ <other-lib>/   Other pinned prebuilt dependencies
├─ manifests/     Version, provenance, license, integrity, and runtime metadata
└─ README.md
```

Each dependency comes from an internal artifact repository, an official vendor
distribution, or a controlled mirror. `manifests/` records its exact version
and integrity policy. Large SDKs belong in Git LFS, an internal artifact store,
or a controlled bootstrap flow rather than ordinary Git history.

The source repository therefore tracks this README, dependency manifests, and
bootstrap logic, but ignores locally staged `qt/` and `cef/` payloads. A clean
checkout must restore the pinned SDKs before configuration. Never force-add
vendor archives or extracted SDK binaries to normal Git history.

Every dependency must record its version, origin, license, patches, supported
platforms, build settings, runtime file list, and upgrade procedure.

## Qt Base

Qt version, binary origin, license, and runtime requirements are pinned in
`manifests/qt.json`. Only Core, Gui, and Widgets are consumed.

```bash
# AGENTIM_QT_ROOT points to the platform-specific Qt SDK root.
third_party/qt/macos-arm64/6.10.3/macos/
third_party/qt/windows-x64/6.10.3/msvc2022_64/
```

The macOS ARM64 Qt 6.10.3 SDK is staged in the path above and pruned for the
current Widgets UI. It keeps Core, Gui, Widgets, transitive DBus dependencies,
CMake package files, required build tools, and `libqcocoa.dylib`. QML, Quick,
Designer, documentation, translations, and unrelated plugins are removed. See
`tools/prune_qt_macos_sdk.sh`; rerun it and verify a full build after upgrades.

End users receive only the packaged AgentIM application. Packaging must bundle
the Qt frameworks/DLLs and required platform plugins.

## Chromium Embedded Framework

CEF is pinned in `manifests/cef.json`. AgentIM consumes the official minimal
binary distribution, excluding samples, extra tools, and symbol archives. The
macOS ARM64 and Windows x64 distributions are currently staged in the paths
declared by the manifest.

Restore the SDK once per platform on a clean workstation or after removing it:

```bash
# Auto-detect the current supported host.
tools/cmake/macos-universal/bin/cmake -P tools/fetch_cef.cmake

# Explicit platform selection; use the project-local CMake on Windows.
cmake -DAGENTIM_CEF_PLATFORM=windows-x64 -P tools/fetch_cef.cmake
```

The fetch script keeps the archive, checksum, and extracted SDK together under
`third_party/cef/<platform>/<version>/`. It verifies the official versioned
SHA-1 sidecar before extracting. A trusted internal mirror can be selected with
`-DAGENTIM_CEF_BASE_URL=<mirror-url>`. Normal configure/build commands never
access the network. Shared project presets set `AGENTIM_ENABLE_CEF=ON`, so a
missing SDK is reported during configure instead of being downloaded
implicitly. CEF consumers link `AgentIM::CEF`; they never add SDK include,
library, or framework paths directly.
