#include <array>

#include <QButtonGroup>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QSizePolicy>
#include <QSplitter>
#include <QString>
#include <Qt>
#include <QVBoxLayout>
#include <QWidget>

#include "agentim/ui/i18n/localized_strings.h"
#include "agentim/ui/main_window.h"
#include "agentim/ui/theme/design_tokens.h"

namespace agentim::ui {

namespace {

constexpr int kDefaultSidebarWidth = 248;
constexpr int kMinimumSidebarWidth = 200;
constexpr int kMaximumSidebarWidth = 360;

struct StyleToken final {
    const char* placeholder;
    const char* value;
};

constexpr std::array kStyleTokens{
    StyleToken{"@{surface-canvas}", theme::color::kSurfaceCanvas},
    StyleToken{"@{surface-primary}", theme::color::kSurfacePrimary},
    StyleToken{"@{surface-sidebar}", theme::color::kSurfaceSidebar},
    StyleToken{"@{surface-interactive}", theme::color::kSurfaceInteractive},
    StyleToken{"@{surface-interactive-hover}", theme::color::kSurfaceInteractiveHover},
    StyleToken{"@{surface-interactive-pressed}", theme::color::kSurfaceInteractivePressed},
    StyleToken{"@{surface-selected}", theme::color::kSurfaceSelected},
    StyleToken{"@{surface-selected-hover}", theme::color::kSurfaceSelectedHover},
    StyleToken{"@{surface-selected-pressed}", theme::color::kSurfaceSelectedPressed},
    StyleToken{"@{surface-avatar}", theme::color::kSurfaceAvatar},
    StyleToken{"@{surface-avatar-hover}", theme::color::kSurfaceAvatarHover},
    StyleToken{"@{surface-avatar-pressed}", theme::color::kSurfaceAvatarPressed},
    StyleToken{"@{border-subtle}", theme::color::kBorderSubtle},
    StyleToken{"@{border-interactive}", theme::color::kBorderInteractive},
    StyleToken{"@{text-primary}", theme::color::kTextPrimary},
    StyleToken{"@{text-secondary}", theme::color::kTextSecondary},
    StyleToken{"@{text-selected}", theme::color::kTextSelected},
    StyleToken{"@{text-on-dark}", theme::color::kTextOnDark},
    StyleToken{"@{text-accent}", theme::color::kTextAccent},
    StyleToken{"@{font-family-ui}", theme::typography::kUiFontFamily},
    StyleToken{"@{font-size-micro}", theme::typography::kSizeMicro},
    StyleToken{"@{font-size-caption}", theme::typography::kSizeCaption},
    StyleToken{"@{font-size-body}", theme::typography::kSizeBody},
    StyleToken{"@{font-weight-regular}", theme::typography::kWeightRegular},
    StyleToken{"@{font-weight-medium}", theme::typography::kWeightMedium},
    StyleToken{"@{font-weight-semibold}", theme::typography::kWeightSemibold},
    StyleToken{"@{font-weight-bold}", theme::typography::kWeightBold},
};

QString createMainWindowStyleSheet() {
    QString style_sheet = QStringLiteral(R"(
        QMainWindow, QWidget#centralWidget {
            background: @{surface-canvas};
            font-family: @{font-family-ui};
            font-size: @{font-size-body};
            font-weight: @{font-weight-regular};
        }

        QSplitter#workspaceSplitter::handle {
            background: @{border-subtle};
        }

        QSplitter#workspaceSplitter::handle:hover,
        QSplitter#workspaceSplitter::handle:pressed {
            background: @{border-interactive};
        }

        QWidget#taskSidebar {
            background: @{surface-sidebar};
        }

        QLabel#createdTasksLabel {
            color: @{text-secondary};
            font-size: @{font-size-caption};
            font-weight: @{font-weight-medium};
            padding: 0 16px;
        }

        QPushButton#mainConversationButton,
        QPushButton#newTaskButton {
            min-height: 44px;
            max-height: 44px;
            border: 0;
            border-radius: 9px;
            outline: none;
            padding: 0 16px;
            text-align: left;
            color: @{text-primary};
            background: transparent;
            font-size: @{font-size-body};
            font-weight: @{font-weight-medium};
        }

        QPushButton#mainConversationButton:checked,
        QPushButton#newTaskButton:checked {
            background: @{surface-selected};
            color: @{text-selected};
            font-weight: @{font-weight-semibold};
        }

        QPushButton#newTaskButton:hover,
        QPushButton#newTaskButton:focus,
        QPushButton#mainConversationButton:hover:!checked,
        QPushButton#mainConversationButton:focus:!checked {
            background: @{surface-interactive-hover};
        }

        QPushButton#mainConversationButton:hover:checked,
        QPushButton#mainConversationButton:focus:checked,
        QPushButton#newTaskButton:hover:checked,
        QPushButton#newTaskButton:focus:checked {
            background: @{surface-selected-hover};
        }

        QPushButton#newTaskButton:pressed,
        QPushButton#mainConversationButton:pressed:!checked {
            background: @{surface-interactive-pressed};
        }

        QPushButton#mainConversationButton:pressed:checked,
        QPushButton#newTaskButton:pressed:checked {
            background: @{surface-selected-pressed};
        }

        QWidget#profileWidget {
            background: transparent;
        }

        QPushButton#avatarButton {
            border: 0;
            border-radius: 23px;
            outline: none;
            background: @{surface-avatar};
            color: @{text-on-dark};
            font-size: @{font-size-caption};
            font-weight: @{font-weight-semibold};
        }

        QPushButton#avatarButton:hover,
        QPushButton#avatarButton:focus {
            background: @{surface-avatar-hover};
        }

        QPushButton#avatarButton:pressed {
            background: @{surface-avatar-pressed};
        }

        QLabel#avatarAddBadge {
            border: 2px solid @{surface-sidebar};
            border-radius: 8px;
            background: @{text-accent};
            color: @{text-on-dark};
            font-size: @{font-size-micro};
            font-weight: @{font-weight-bold};
        }

        QWidget#workspaceContent {
            background: @{surface-primary};
        }
    )");

    for (const StyleToken& token : kStyleTokens) {
        style_sheet.replace(
            QString::fromLatin1(token.placeholder), QString::fromLatin1(token.value));
    }

    return style_sheet;
}

}  // namespace

MainWindow::MainWindow(QWidget* workspace_content, QWidget* parent)
    : QMainWindow(parent),
      workspace_content_(workspace_content) {
    Q_ASSERT(workspace_content_ != nullptr);

    setWindowTitle(QStringLiteral("AgentIM"));
    resize(1440, 960);
    setMinimumSize(960, 640);

    setupUi();
    setupStyles();
    main_conversation_button_->setChecked(true);
}

void MainWindow::setupUi() {
    central_widget_ = new QWidget;
    central_widget_->setObjectName(QStringLiteral("centralWidget"));
    setCentralWidget(central_widget_);

    root_layout_ = new QVBoxLayout(central_widget_);
    root_layout_->setContentsMargins(0, 0, 0, 0);
    root_layout_->setSpacing(0);

    workspace_splitter_ = new QSplitter(Qt::Horizontal, central_widget_);
    workspace_splitter_->setObjectName(QStringLiteral("workspaceSplitter"));
    workspace_splitter_->setChildrenCollapsible(false);
    workspace_splitter_->setHandleWidth(5);
    root_layout_->addWidget(workspace_splitter_, 1);

    task_sidebar_ = new QWidget(workspace_splitter_);
    task_sidebar_->setObjectName(QStringLiteral("taskSidebar"));
    task_sidebar_->setMinimumWidth(kMinimumSidebarWidth);
    task_sidebar_->setMaximumWidth(kMaximumSidebarWidth);
    task_sidebar_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    workspace_splitter_->addWidget(task_sidebar_);

    task_sidebar_layout_ = new QVBoxLayout(task_sidebar_);
    task_sidebar_layout_->setContentsMargins(14, 16, 14, 14);
    task_sidebar_layout_->setSpacing(6);

    task_navigation_group_ = new QButtonGroup(task_sidebar_);
    task_navigation_group_->setExclusive(true);

    main_conversation_button_ = new QPushButton(task_sidebar_);
    main_conversation_button_->setObjectName(QStringLiteral("mainConversationButton"));
    main_conversation_button_->setText(i18n::text(i18n::TextId::kMainConversation));
    main_conversation_button_->setCheckable(true);
    main_conversation_button_->setChecked(true);
    main_conversation_button_->setFocusPolicy(Qt::TabFocus);
    main_conversation_button_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    task_navigation_group_->addButton(main_conversation_button_);
    task_sidebar_layout_->addWidget(main_conversation_button_);

    created_tasks_label_ =
        new QLabel(i18n::text(i18n::TextId::kCreatedTasks), task_sidebar_);
    created_tasks_label_->setObjectName(QStringLiteral("createdTasksLabel"));
    created_tasks_label_->setFixedHeight(28);
    task_sidebar_layout_->addWidget(created_tasks_label_);

    const QString new_task_text =
        QStringLiteral("+  %1").arg(i18n::text(i18n::TextId::kNewTask));
    new_task_button_ = new QPushButton(new_task_text, task_sidebar_);
    new_task_button_->setObjectName(QStringLiteral("newTaskButton"));
    new_task_button_->setCheckable(true);
    new_task_button_->setFocusPolicy(Qt::TabFocus);
    new_task_button_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    task_navigation_group_->addButton(new_task_button_);
    task_sidebar_layout_->addWidget(new_task_button_);
    task_sidebar_layout_->addStretch(1);

    profile_widget_ = new QWidget(task_sidebar_);
    profile_widget_->setObjectName(QStringLiteral("profileWidget"));
    profile_widget_->setFixedHeight(52);
    profile_widget_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    task_sidebar_layout_->addWidget(profile_widget_);

    profile_layout_ = new QGridLayout(profile_widget_);
    profile_layout_->setContentsMargins(0, 0, 0, 0);
    profile_layout_->setSpacing(0);
    profile_layout_->setColumnMinimumWidth(0, 48);
    profile_layout_->setColumnStretch(1, 1);

    avatar_button_ =
        new QPushButton(i18n::text(i18n::TextId::kCurrentUser), profile_widget_);
    avatar_button_->setObjectName(QStringLiteral("avatarButton"));
    avatar_button_->setFocusPolicy(Qt::TabFocus);
    avatar_button_->setFixedSize(46, 46);
    profile_layout_->addWidget(avatar_button_, 0, 0, Qt::AlignTop | Qt::AlignLeft);

    avatar_add_badge_ = new QLabel(QStringLiteral("+"), profile_widget_);
    avatar_add_badge_->setObjectName(QStringLiteral("avatarAddBadge"));
    avatar_add_badge_->setAlignment(Qt::AlignCenter);
    avatar_add_badge_->setFixedSize(16, 16);
    profile_layout_->addWidget(avatar_add_badge_, 0, 0, Qt::AlignBottom | Qt::AlignRight);

    workspace_content_->setParent(workspace_splitter_);
    workspace_content_->setObjectName(QStringLiteral("workspaceContent"));
    workspace_content_->setMinimumWidth(0);
    workspace_splitter_->addWidget(workspace_content_);

    workspace_splitter_->setCollapsible(0, false);
    workspace_splitter_->setCollapsible(1, false);
    workspace_splitter_->setStretchFactor(0, 0);
    workspace_splitter_->setStretchFactor(1, 1);
    workspace_splitter_->setSizes({kDefaultSidebarWidth, width() - kDefaultSidebarWidth});
}

void MainWindow::setupStyles() {
    setStyleSheet(createMainWindowStyleSheet());
}

}  // namespace agentim::ui
