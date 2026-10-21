# Resources

Application resources: icons, translation files, Qt resource files, default configuration templates, and static CEF web assets.

The independently maintained local Web application belongs under `web_apps/`.
This directory may contain native application resources and Web bootstrap,
error, or offline-fallback assets, but it must not contain the primary Web
application source. Vendor CEF resources and locales remain under the staged
SDK and are collected by packaging rules; do not copy them into application
source resources.
