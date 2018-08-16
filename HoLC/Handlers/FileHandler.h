/*-----------------------------------------------------------------------------

 Copyright 2017 Hopsan Group

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

 The full license is available in the file GPLv3.
 For details about the 'Hopsan Group' or information about Authors and
 Contributors see the HOPSANGROUP and AUTHORS files that are located in
 the Hopsan source code root directory.

-----------------------------------------------------------------------------*/

#ifndef FILEHANDLER_H
#define FILEHANDLER_H

#include <QMap>
#include <QFileInfo>
#include <QTreeWidgetItem>

class Configuration;
class ProjectFilesWidget;
class EditorWidget;
class MessageHandler;
class FileObject;

class FileHandler : public QObject
{
    Q_OBJECT

public:
    FileHandler(Configuration *pConfiuration, ProjectFilesWidget *pFilesWidget, EditorWidget *pEditorWidget, MessageHandler *pMessageHandler);

signals:
    void fileOpened(bool);

public slots:
    void generateNewXmlAndSourceFiles(const QString &libName, QString &path);
    void generateXmlAndSourceFiles(QString path="");
    void addComponent(QString path="");
    void addComponent(const QString &code, const QString &typeName);
    void addAppearanceFile(const QString &code, const QString &fileName);
    void addAppearanceFile(QString path="");
    void loadFromXml();
    void saveToXml();
    void updateText();
    void compileLibrary();
    void saveToXml(const QString &path);
    void loadFromXml(const QString &path);
    void setFileNotSaved();
    bool hasFile(QString filePath);

private slots:
    void openFile(QTreeWidgetItem *pItem, int);
    void removeFile(QTreeWidgetItem *pItem);

private:
    Configuration *mpConfiguration;
    ProjectFilesWidget *mpFilesWidget;
    EditorWidget *mpEditorWidget;
    MessageHandler *mpMessageHandler;

    QMap<QTreeWidgetItem*, FileObject*> mTreeToFileMap;

    QString mLibId;
    QString mLibName;
    QString mLibTarget;
    QString mLibDebugExt;
    QVector<FileObject*> mFilePtrs;

    FileObject *mpCurrentFile;
};


class FileObject
{
public:
    enum FileTypeEnum {XML, Source, Component, Auxiliary, CAF};

    FileObject();
    FileObject(const QString &path, FileTypeEnum type);
    bool operator==(const FileObject &other) const;

    FileTypeEnum mType;
    QFileInfo mFileInfo;
    QString mText;
    bool mIsSaved;
    bool mExists;
};

#endif // FILEHANDLER_H
