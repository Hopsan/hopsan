#include <QApplication>
#include <QTreeWidgetItem>
#include <QMap>
#include <QFileInfo>
#include <QtXml>
#include <QFileDialog>

#include "FileHandler.h"
#include "Widgets/ProjectFilesWidget.h"
#include "Widgets/EditorWidget.h"
#include "MessageHandler.h"
#include "Utilities/CompilingUtilities.h"
#include "Handlers/OptionsHandler.h"

FileHandler::FileHandler(ProjectFilesWidget *pFilesWidget, EditorWidget *pEditorWidget, MessageHandler *pMessageHandler, OptionsHandler *pOptionsHandler) :
    QObject(0)
{
    mpFilesWidget = pFilesWidget;
    mpEditorWidget = pEditorWidget;
    mpMessageHandler = pMessageHandler;
    mpOptionsHandler = pOptionsHandler;

    connect(pFilesWidget->mpTreeWidget, SIGNAL(itemClicked(QTreeWidgetItem*,int)), this, SLOT(openFile(QTreeWidgetItem*, int)));
    connect(pFilesWidget, SIGNAL(deleteRequested(QTreeWidgetItem*)), this, SLOT(removeFile(QTreeWidgetItem*)));

    mpCurrentFile = 0;
}

void FileHandler::generateNewXmlAndSourceFiles(const QString &libName, QString &path)
{
    mFilePtrs.clear();
    mTreeToFileMap.clear();
    mpFilesWidget->mpTreeWidget->clear();

    mLibName = libName;
    mLibTarget = libName;

    generateXmlAndSourceFiles(path);
}

void FileHandler::generateXmlAndSourceFiles(QString path)
{
    if(path.isEmpty())
    {
        foreach(const FileObject *pFile, mFilePtrs)
        {
            if(pFile->mType == FileObject::XML)
            {
                path = pFile->mFileInfo.absolutePath();
            }
        }
    }

    QFile xmlFile(path+"/"+mLibName+".xml");
    QFile sourceFile(path+"/"+mLibName+".cc");
    QFile xmlTemplateFile(":/templates/Templates/xmlTemplate.xml");
    QFile sourceTemplateFile(":/templates/Templates/sourceTemplate.cc");

    if(!xmlFile.open(QFile::WriteOnly | QFile::Text))
    {
        mpMessageHandler->addErrorMessage("Unable to open file for writing: "+xmlFile.fileName());
        return;
    }

    if(!sourceFile.open(QFile::WriteOnly | QFile::Text))
    {
        mpMessageHandler->addErrorMessage("Unable to open file for writing: "+sourceFile.fileName());
        return;
    }

    if(!xmlTemplateFile.open(QFile::ReadOnly | QFile::Text))
    {
        mpMessageHandler->addErrorMessage("Unable to open file for reading: "+xmlTemplateFile.fileName());
        return;
    }

    if(!sourceTemplateFile.open(QFile::ReadOnly | QFile::Text))
    {
        mpMessageHandler->addErrorMessage("Unable to open file for reading: "+sourceTemplateFile.fileName());
        return;
    }

    QString xmlCode = xmlTemplateFile.readAll();
    QString sourceCode = sourceTemplateFile.readAll();

    QString includeCompString;
    QString registerCompString;
    QString xmlCompString;
    foreach(const FileObject *pFile, mFilePtrs)
    {
        if(pFile->mType == FileObject::Component)
        {
            includeCompString.append("\n#include \""+pFile->mFileInfo.fileName()+"\"");
            registerCompString.append("\n    pComponentFactory->registerCreatorFunction(\""+pFile->mFileInfo.baseName()+"\", "+pFile->mFileInfo.baseName()+"::Creator);");
            xmlCompString.append("\n    <component>"+pFile->mFileInfo.fileName()+"</component>");
        }
    }


    sourceCode.replace("<<<includecomponents>>>",includeCompString);
    sourceCode.replace("<<<registercomponents>>>",registerCompString);
    sourceCode.replace("<<<libname>>>", mLibName);

    xmlCode.replace("<<<libname>>>", mLibName);
    xmlCode.replace("<<<sourcefile>>>", QFileInfo(sourceFile).fileName());
    xmlCode.replace("<<<components>>>",xmlCompString);
    xmlCode.replace("<<<auxiliary>>>","");

    sourceFile.write(sourceCode.toUtf8());
    xmlFile.write(xmlCode.toUtf8());

    sourceFile.close();
    xmlFile.close();

    loadFromXml(QFileInfo(xmlFile).absoluteFilePath());
}

void FileHandler::addComponent(const QString &code, const QString &typeName)
{
    QString path;
    foreach(const FileObject *pFile, mFilePtrs)
    {
        if(pFile->mType == FileObject::XML)
        {
            path = pFile->mFileInfo.absolutePath();
        }
    }

    if(path.isEmpty()) return;

    QFile componentFile(path+"/"+typeName+".hpp");

    if(!componentFile.open(QFile::WriteOnly | QFile::Text))
    {
        mpMessageHandler->addErrorMessage("Cannot open file for writing: " + componentFile.fileName());
        return;
    }

    componentFile.write(code.toUtf8());

    componentFile.close();

    FileObject *pFileObject = new FileObject(QFileInfo(componentFile).absoluteFilePath(), FileObject::Component);
    mFilePtrs.append(pFileObject);

    QTreeWidgetItem *pItem = mpFilesWidget->addFile(mFilePtrs.last());
    mTreeToFileMap.insert(pItem, mFilePtrs.last());

    generateXmlAndSourceFiles();
}


void FileHandler::loadFromXml()
{
    QString path = QFileDialog::getOpenFileName(0, "Open Component Library", QString(), "*.xml");
    if(!path.isEmpty())
    {
        loadFromXml(path);
    }
}

void FileHandler::saveToXml()
{
    for(int i=0; i<mFilePtrs.size(); ++i)
    {
        if(mFilePtrs[i]->mType == FileObject::XML)
        {
            saveToXml(mFilePtrs[i]->mFileInfo.absoluteFilePath());
        }
    }
}


void FileHandler::loadFromXml(const QString &path)
{
    mpMessageHandler->addInfoMessage("Loading from: " + path);

    QFileInfo info(path);

    QFile file(path);
    if(!file.open(QFile::ReadOnly | QFile::Text))
    {
        mpMessageHandler->addErrorMessage("Cannot open file for reading.");
        return;
    }

    QDomDocument domDocument;
    QString errorStr;
    int errorLine, errorColumn;
    if (!domDocument.setContent(&file, false, &errorStr, &errorLine, &errorColumn))
    {
        mpMessageHandler->addErrorMessage(tr("Parse error at line %1, column %2:\n%3")
                                 .arg(errorLine)
                                 .arg(errorColumn)
                                 .arg(errorStr));
    }
    else
    {
        QDomElement libRoot = domDocument.documentElement();

        mFilePtrs.clear();
        mpFilesWidget->clear();

        mLibName = libRoot.attribute("name");

        FileObject *pFile = new FileObject(path, FileObject::XML);
        mFilePtrs.append(pFile);
        QTreeWidgetItem *pItem = mpFilesWidget->addFile(mFilePtrs.last());
        mTreeToFileMap.insert(pItem, mFilePtrs.last());

        QDomElement libElement = libRoot.firstChildElement("lib");
        if(!libElement.isNull())
        {
            mLibTarget = libElement.text();
        }

        QDomElement sourceElement = libRoot.firstChildElement("source");
        if(!sourceElement.isNull())
        {
            mFilePtrs.append(new FileObject(info.absolutePath()+"/"+sourceElement.text(), FileObject::Source));
            pItem = mpFilesWidget->addFile(mFilePtrs.last());
            mTreeToFileMap.insert(pItem, mFilePtrs.last());
        }

        QDomElement compElement = libRoot.firstChildElement("component");
        while(!compElement.isNull())
        {
            mFilePtrs.append(new FileObject(info.absolutePath()+"/"+compElement.text(), FileObject::Component));
            pItem = mpFilesWidget->addFile(mFilePtrs.last());
            mTreeToFileMap.insert(pItem, mFilePtrs.last());
            compElement = compElement.nextSiblingElement("component");
        }

        QDomElement auxElement = libRoot.firstChildElement("auxiliary");
        while(!auxElement.isNull())
        {
            mFilePtrs.append(new FileObject(info.absolutePath()+"/"+auxElement.text(), FileObject::Auxiliary));
            pItem = mpFilesWidget->addFile(mFilePtrs.last());
            mTreeToFileMap.insert(pItem, mFilePtrs.last());
            auxElement = auxElement.nextSiblingElement("auxiliary");
        }
    }
    file.close();
}

void FileHandler::saveToXml(const QString &filePath)
{
    mpFilesWidget->removeAsterisks();

    for(int i=0; i<mFilePtrs.size(); ++i)
    {
        QFile file(mFilePtrs[i]->mFileInfo.absoluteFilePath());
        file.open(QFile::WriteOnly | QFile::Text);
        file.write(mFilePtrs[i]->mText.toUtf8());
        file.close();
    }

    QString path = QFileInfo(filePath).absolutePath();

    QDomDocument domDocument;
    QDomElement libRoot = domDocument.createElement("hopsancomponentlibrary");
    libRoot.setAttribute("xmlversion", 0.1);
    libRoot.setAttribute("libversion", 1);
    libRoot.setAttribute("name", mLibName);
    domDocument.appendChild(libRoot);

    QDomElement libElement = domDocument.createElement("lib");
    libElement.appendChild(domDocument.createTextNode(mLibTarget));
    libRoot.appendChild(libElement);

    for(int f=0; f<mFilePtrs.size(); ++f)
    {
        QDomElement fileElement;
        if(mFilePtrs[f]->mType == FileObject::Source)
        {
            fileElement = domDocument.createElement("source");
        }
        else if(mFilePtrs[f]->mType == FileObject::Component)
        {
            fileElement = domDocument.createElement("component");
        }
        else if(mFilePtrs[f]->mType == FileObject::Auxiliary)
        {
            fileElement = domDocument.createElement("auxiliary");
        }
        fileElement.appendChild(domDocument.createTextNode(mFilePtrs[f]->mFileInfo.absoluteFilePath().remove(path+"/")));
        libRoot.appendChild(fileElement);
    }

    QDomNode xmlProcessingInstruction = domDocument.createProcessingInstruction("xml","version=\"1.0\" encoding=\"UTF-8\"");
    domDocument.insertBefore(xmlProcessingInstruction, domDocument.firstChild());

    //Save to file
    QFile xmlFile(filePath);
    if (!xmlFile.open(QIODevice::WriteOnly | QIODevice::Text))  //open file
    {
        mpMessageHandler->addErrorMessage("Failed to open config file for writing: "+filePath);
        return;
    }
    QTextStream out(&xmlFile);
    domDocument.save(out, 4);
}

void FileHandler::updateText()
{
    if(mpCurrentFile)
    {
        mpCurrentFile->mText = mpEditorWidget->getText();
    }
}

void FileHandler::compileLibrary()
{
    if(mpOptionsHandler->getIncludePath().isEmpty())
    {
        mpMessageHandler->addErrorMessage("Hopsan path is not setup correctly.");
        return;
    }
    //! @todo Maybe check this in some better way
    //! @todo Also check compiler path

    QString path;
    QStringList sources;
    QStringList includeDirs;
    QStringList libs;

    foreach(const FileObject *file, mFilePtrs)
    {
        if(file->mType == FileObject::XML)
        {
            path = file->mFileInfo.absolutePath();
        }
        else if(file->mType == FileObject::Source)
        {
            sources.append(file->mFileInfo.absoluteFilePath());
        }
    }

    includeDirs.append(mpOptionsHandler->getIncludePath());
    libs.append(mpOptionsHandler->getLibPath());

    QString target = mLibTarget;
#ifdef linux
    target.prepend("lib");
#endif

    QString compilerPath = mpOptionsHandler->getCompilerPath();

    bool success;
    QStringList output = compileComponentLibrary(compilerPath, path, target, sources, libs, includeDirs, success);
    //! @todo Do something with the success variable

    foreach(const QString &line, output)
    {
        mpMessageHandler->addInfoMessage(line);
    }
}


void FileHandler::openFile(QTreeWidgetItem *pItem, int)
{
    if(!mTreeToFileMap.contains(pItem)) return;

    mpCurrentFile = mTreeToFileMap.find(pItem).value();

    EditorWidget::HighlighterTypeEnum type;
    QString fileName = mpCurrentFile->mFileInfo.fileName();
    bool editingEnabled=true;
    if(mpCurrentFile->mType == FileObject::XML || mpCurrentFile->mType == FileObject::Source)
    {
        editingEnabled=false;
    }
    if(fileName.endsWith(".xml"))
    {
        type = EditorWidget::XML;
    }
    else if(fileName.endsWith(".hpp") || fileName.endsWith(".c") || fileName.endsWith(".cpp") ||
            fileName.endsWith(".cc") || fileName.endsWith(".h"))
    {
        type = EditorWidget::Cpp;
    }
    mpEditorWidget->setText(mTreeToFileMap.find(pItem).value()->mText, type, editingEnabled);

    emit fileOpened(false);
}

void FileHandler::removeFile(QTreeWidgetItem *pItem)
{
    mFilePtrs.remove(mFilePtrs.indexOf(mTreeToFileMap.find(pItem).value()));
    mpFilesWidget->removeItem(pItem);
    mpEditorWidget->clear();
    mTreeToFileMap.remove(pItem);

    generateXmlAndSourceFiles();
}


FileObject::FileObject()
{
    mFileInfo = QFileInfo();
    mIsSaved = false;
    mType = FileObject::Auxiliary;
    mText = QString();
    mExists = false;
}

FileObject::FileObject(const QString &path, FileTypeEnum type)
{
    mFileInfo = QFileInfo(path);
    mIsSaved=true;

    mType = type;

    QFile file(path);
    mExists = file.open(QFile::ReadOnly | QFile::Text);
    mText = file.readAll();
    file.close();
}

bool FileObject::operator==(const FileObject &other) const
{
    return (this->mFileInfo == other.mFileInfo);
}
