# macOS platform

macOS-specific implementations: application sandboxing, notifications, signing and notarization, bundles, Keychain, and system integration.

Cross-platform interfaces are defined in `src/base`; this directory contains implementations only.

The development build creates the macOS CEF base, Alerts, GPU, Plugin, and
Renderer helper applications and stages them with the CEF framework, resources,
locales, and local Web workbench into the main bundle. This platform area owns
the remaining entitlements, code-signing order, hardened-runtime rules, and
notarization verification. Browser lifecycle and bridge behavior remain in
`src/web_cef`; business services remain in `src/logic`.
