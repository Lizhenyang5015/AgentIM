#pragma once

class QString;

namespace agentim::ui::i18n {

enum class TextId {
    kMainConversation,
    kCreatedTasks,
    kNewTask,
    kCurrentUser,
    kCount,
};

QString text(TextId id);

}  // namespace agentim::ui::i18n
