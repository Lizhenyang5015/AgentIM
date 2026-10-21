# Windows platform

Windows-specific implementations: system notifications, registry access, crash collection, installer integration, and Win32/COM integration.

Cross-platform interfaces are defined in `src/base`; this directory contains implementations only.

CEF SDK discovery and generic runtime staging are configured, but the Windows
subprocess entry, DLL/resource staging result, sandbox policy, and application
launch still require verification on a Windows MSVC host. This platform area
owns code signing and installer verification. Browser lifecycle and bridge
behavior remain in `src/web_cef`; business services remain in `src/logic`.
