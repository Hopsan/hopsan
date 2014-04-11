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

private slots:
    void openFile(QTreeWidgetItem *pItem, int);
    void removeFile(QTreeWidgetItem *pItem);

private:
    Configuration *mpConfiguration;
    ProjectFilesWidget *mpFilesWidget;
    EditorWidget *mpEditorWidget;
    MessageHandler *mpMessageHandler;

    QMap<QTreeWidgetItem*, FileObject*> mTreeToFileMap;

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
