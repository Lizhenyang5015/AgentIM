# Packaging

Windows and macOS installation and release rules: Qt runtime deployment, CEF resource collection, signing, notarization, installer creation, and symbol archive generation.

CEF packaging must collect the runtime files declared by
`third_party/manifests/cef.json`, including the framework or DLLs, resources,
locales, and platform helper processes. Packaging consumes staged SDK contents;
it never downloads dependencies.

Development CMake rules stage the macOS CEF runtime, complete helper application
set, and local Web workbench into the build bundle. Release deployment is not
complete: full Qt runtime collection, Windows verification, signing,
notarization, installer creation, and symbol archiving must be implemented here.
Their final commands and verification steps must be recorded in
`docs/build-and-debug.md` and `docs/project-index.md` when introduced.
