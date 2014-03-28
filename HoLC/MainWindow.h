#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class ProjectFilesWidget;
class MessageWidget;
class EditorWidget;
class MessageHandler;
class FileHandler;
class OptionsWidget;
class OptionsHandler;
class NewProjectDialog;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    ProjectFilesWidget *mpProjectFilesWidget;
    MessageWidget *mpMessageWidget;
    EditorWidget *mpEditorWidget;
    OptionsWidget *mpOptionsWidget;
    NewProjectDialog *mpNewProjectDialog;

    MessageHandler *mpMessageHandler;
    FileHandler *mpFileHandler;
    OptionsHandler *mpOptionsHandler;
};

#endif // MAINWINDOW_H
