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
    void addComponent(QString existingPath="");
    void addComponent(const QString &code, const QString &typeName);
    void addAppearanceFile(const QString &code, const QString &fileName);
    void addAppearanceFile(QString existingPath="");
    void loadLibraryFromXml();
    void saveToXml();
    void updateText();
    void compileLibrary();
    void saveToXml(const QString &path);
    void loadLibraryFromXml(const QString &path);
    void setFileNotSaved();
    bool hasFile(QString filePath);
    void reloadFile();

private slots:
    void openFile(QTreeWidgetItem *pItem, int);
    void openFile(QSharedPointer<FileObject> pFile);
    void removeFile(QTreeWidgetItem *pItem);

private:
    Configuration *mpConfiguration;
    ProjectFilesWidget *mpFilesWidget;
    EditorWidget *mpEditorWidget;
    MessageHandler *mpMessageHandler;

    QMap<QTreeWidgetItem*, QSharedPointer<FileObject>> mTreeToFileMap;

    struct BuildFlags {
        QString type;
        QString os;
        QString content;
    };

    QString mLibraryId;
    QString mLibraryName;
    QString mLibraryCompiledFile;
    QString mLibraryDebugExtension;
    QString mLibraryMainXMLFile;
    QString mLibraryMainCPPFile;
    QVector<BuildFlags> mLibraryBuildFlags;
    QList<QSharedPointer<FileObject>> mLibraryFiles;

    QSharedPointer<FileObject> mpCurrentFile;
};


class FileObject
{
public:
    enum FileTypeEnum {XML, Source, Component, Auxiliary, CAF};

    FileObject() = default;
    FileObject(const QString &path, FileTypeEnum type);
    bool operator==(const FileObject &other) const;

    FileTypeEnum mType = FileObject::Auxiliary;
    bool mIsSaved = false;
    bool mExists = false;
    QFileInfo mFileInfo;
    QString mFileContents;
};

#endif // FILEHANDLER_H
