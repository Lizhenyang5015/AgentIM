#include <array>
#include <cstddef>

#include <QLocale>
#include <QString>

#include "agentim/ui/i18n/localized_strings.h"

namespace agentim::ui::i18n {

namespace {

constexpr std::size_t kTextCount = static_cast<std::size_t>(TextId::kCount);
using TextCatalog = std::array<const char*, kTextCount>;

constexpr TextCatalog kEnglishCatalog{
    "Main conversation",
    "Currently created tasks",
    "New task",
    "Me",
};

constexpr TextCatalog kSimplifiedChineseCatalog{
    "主对话",
    "当前已创建任务",
    "新建任务",
    "我",
};

const TextCatalog& activeCatalog() {
    if (QLocale::system().language() == QLocale::Chinese) {
        return kSimplifiedChineseCatalog;
    }

    return kEnglishCatalog;
}

}  // namespace

QString text(TextId id) {
    const auto index = static_cast<std::size_t>(id);
    return QString::fromUtf8(activeCatalog()[index]);
}

}  // namespace agentim::ui::i18n
