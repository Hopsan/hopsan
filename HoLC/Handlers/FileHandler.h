#ifndef FILEHANDLER_H
#define FILEHANDLER_H

#include <QMap>
#include <QFileInfo>
#include <QTreeWidgetItem>

class ProjectFilesWidget;
class EditorWidget;
class MessageHandler;
class FileObject;
class OptionsHandler;

class FileHandler : public QObject
{
    Q_OBJECT

public:
    FileHandler(ProjectFilesWidget *pFilesWidget, EditorWidget *pEditorWidget, MessageHandler *pMessageHandler, OptionsHandler *pOptionsHandler);

signals:
    void fileOpened(bool);

public slots:
    void generateNewXmlAndSourceFiles(const QString &libName, QString &path);
    void generateXmlAndSourceFiles(QString path="");
    void addComponent(QString path="");
    void addComponent(const QString &code, const QString &typeName);
    void loadFromXml();
    void saveToXml();
    void updateText();
    void compileLibrary();

private slots:
    void openFile(QTreeWidgetItem *pItem, int);
    void removeFile(QTreeWidgetItem *pItem);

private:
    void saveToXml(const QString &path);
    void loadFromXml(const QString &path);

    ProjectFilesWidget *mpFilesWidget;
    EditorWidget *mpEditorWidget;
    MessageHandler *mpMessageHandler;
    OptionsHandler *mpOptionsHandler;

    QMap<QTreeWidgetItem*, FileObject*> mTreeToFileMap;

    QString mLibName;
    QString mLibTarget;
    QVector<FileObject*> mFilePtrs;

    FileObject *mpCurrentFile;
};


class FileObject
{
public:
    enum FileTypeEnum {XML, Source, Component, Auxiliary};

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
