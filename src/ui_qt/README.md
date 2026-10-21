# ui_qt

Native Qt UI: main window, conversations, contacts, settings, system notifications, and platform-window integration.

The current native UI milestone uses only `Qt6::Widgets` to create a
cross-platform `MainWindow`. Qt Core and Qt Gui required by `Widgets` are
included automatically as transitive dependencies. QML, Quick, and WebEngine
are not used. CEF dependency discovery is enabled at the project level, but CEF
is not linked into `ui_qt`; embedding belongs to the future `web_cef` module.

The UI must run only on the Qt main thread and must not directly read or write databases or call low-level networking implementations.
