#pragma once

#include <QMainWindow>

class QButtonGroup;
class QGridLayout;
class QLabel;
class QPushButton;
class QSplitter;
class QVBoxLayout;
class QWidget;

namespace agentim::ui {

class MainWindow final : public QMainWindow {
public:
    explicit MainWindow(QWidget* workspace_content, QWidget* parent = nullptr);

private:
    void setupUi();
    void setupStyles();

    QWidget* central_widget_ = nullptr;
    QVBoxLayout* root_layout_ = nullptr;

    QSplitter* workspace_splitter_ = nullptr;

    QWidget* task_sidebar_ = nullptr;
    QVBoxLayout* task_sidebar_layout_ = nullptr;
    QButtonGroup* task_navigation_group_ = nullptr;
    QPushButton* main_conversation_button_ = nullptr;
    QLabel* created_tasks_label_ = nullptr;
    QPushButton* new_task_button_ = nullptr;

    QWidget* profile_widget_ = nullptr;
    QGridLayout* profile_layout_ = nullptr;
    QPushButton* avatar_button_ = nullptr;
    QLabel* avatar_add_badge_ = nullptr;

    QWidget* workspace_content_ = nullptr;
};

}  // namespace agentim::ui
