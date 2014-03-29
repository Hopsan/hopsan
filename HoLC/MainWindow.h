#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class ProjectFilesWidget;
class MessageWidget;
class EditorWidget;
class MessageHandler;
class FileHandler;
class OptionsWidget;
class NewProjectDialog;
class CreateComponentWizard;
class Configuration;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Configuration *mpConfiguration;

    ProjectFilesWidget *mpProjectFilesWidget;
    MessageWidget *mpMessageWidget;
    EditorWidget *mpEditorWidget;
    OptionsWidget *mpOptionsWidget;
    NewProjectDialog *mpNewProjectDialog;
    CreateComponentWizard *mpCreateComponentWizard;

    MessageHandler *mpMessageHandler;
    FileHandler *mpFileHandler;
};

#endif // MAINWINDOW_H
