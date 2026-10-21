#!/usr/bin/env bash
set -euo pipefail

# Keeps the smallest known Qt SDK subset for AgentIM's current Widgets-only UI.
# Re-run only after replacing the Qt SDK, then validate with a CMake build.
sdk_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/../third_party/qt/macos-arm64/6.10.3/macos" && pwd)"

test -f "$sdk_root/lib/cmake/Qt6/Qt6Config.cmake"
test -f "$sdk_root/lib/QtWidgets.framework/Versions/A/QtWidgets"

for framework in "$sdk_root"/lib/Qt*.framework; do
    case "$(basename "$framework")" in
        QtCore.framework|QtDBus.framework|QtGui.framework|QtWidgets.framework) ;;
        *) rm -rf -- "$framework" ;;
    esac
done

rm -rf -- \
    "$sdk_root/doc" \
    "$sdk_root/phrasebooks" \
    "$sdk_root/qml" \
    "$sdk_root/sbom" \
    "$sdk_root/translations" \
    "$sdk_root/lib/objects-RelWithDebInfo" \
    "$sdk_root/lib/pkgconfig"

find "$sdk_root/lib" -maxdepth 1 -type f \( -name '*.a' -o -name '*.prl' \) -delete

for plugin_dir in "$sdk_root"/plugins/*; do
    [ -e "$plugin_dir" ] || continue
    [ "$(basename "$plugin_dir")" = "platforms" ] || rm -rf -- "$plugin_dir"
done
find "$sdk_root/plugins/platforms" -maxdepth 1 -type f ! -name 'libqcocoa.dylib' -delete

for tool in "$sdk_root"/bin/*; do
    case "$(basename "$tool")" in
        androiddeployqt|androidtestrunner|macdeployqt|qdbuscpp2xml|qdbusxml2cpp|qmake|qmake6|qt-cmake|qt.conf|qtpaths|qtpaths6) ;;
        *) rm -rf -- "$tool" ;;
    esac
done

rm -f -- "$sdk_root"/config_qtdeclarative.* "$sdk_root"/config_qtsvg.* \
    "$sdk_root"/config_qttools.* "$sdk_root"/config_qttranslations.*

echo "Pruned Qt SDK: $sdk_root"
