/*-----------------------------------------------------------------------------
 This source file is a part of Hopsan

 Copyright (c) 2009 to present year, Hopsan Group

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 For license details and information about the Hopsan Group see the files
 GPLv3 and HOPSANGROUP in the Hopsan source code root directory

 For author and contributor information see the AUTHORS file
-----------------------------------------------------------------------------*/

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

private slots:
    void showHistory();

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
