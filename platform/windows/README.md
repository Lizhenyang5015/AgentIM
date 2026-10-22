# Windows platform

Windows-specific implementations: system notifications, registry access, crash collection, installer integration, and Win32/COM integration.

Cross-platform interfaces are defined in `src/base`; this directory contains implementations only.

The development build stages the CEF DLLs/resources and packaged Web workbench
beside the executable. It also runs the project Qt SDK's `windeployqt.exe` with
the matching configuration to collect Qt DLLs and platform plugins. See
`docs/build-and-debug.md` for the x64 MSVC configure, build, and launch commands.

The Windows x64 Debug build, subprocess entry, runtime staging, and application
launch have been verified on an MSVC host, with the Qt main window and embedded
Workbench visible. Release builds, installation staging, signing, installers,
and enabled CEF sandbox behavior remain unverified. This platform area owns
code signing and installer verification. Browser lifecycle and bridge behavior
remain in `src/web_cef`; business services remain in `src/logic`.
