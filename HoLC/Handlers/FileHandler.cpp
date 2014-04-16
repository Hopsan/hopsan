#include <QApplication>
#include <QTreeWidgetItem>
#include <QMap>
#include <QFileInfo>
#include <QtXml>
#include <QFileDialog>
#include <QLabel>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QPushButton>

#include "Configuration.h"
#include "FileHandler.h"
#include "Widgets/ProjectFilesWidget.h"
#include "Widgets/EditorWidget.h"
#include "MessageHandler.h"
#include "Utilities/CompilingUtilities.h"
#include "Utilities/StringUtilities.h"

FileHandler::FileHandler(Configuration *pConfiuration, ProjectFilesWidget *pFilesWidget, EditorWidget *pEditorWidget, MessageHandler *pMessageHandler) :
    QObject(0)
{
    mpConfiguration = pConfiuration;
    mpFilesWidget = pFilesWidget;
    mpEditorWidget = pEditorWidget;
    mpMessageHandler = pMessageHandler;

    connect(pFilesWidget->mpTreeWidget, SIGNAL(itemClicked(QTreeWidgetItem*,int)), this, SLOT(openFile(QTreeWidgetItem*, int)));
    connect(pFilesWidget, SIGNAL(deleteRequested(QTreeWidgetItem*)), this, SLOT(removeFile(QTreeWidgetItem*)));

    mpCurrentFile = 0;
    mLibDebugExt = "_d";
}

void FileHandler::generateNewXmlAndSourceFiles(const QString &libName, QString &path)
{
    mFilePtrs.clear();
    mTreeToFileMap.clear();
   //mpFilesWidget->mpTreeWidget->clear();
    mpFilesWidget->clear();

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
    QFile xmlTemplateFile(":Templates/xmlTemplate.xml");
    QFile sourceTemplateFile(":Templates/sourceTemplate.cc");

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
    QString xmlAppearanceString;
    foreach(const FileObject *pFile, mFilePtrs)
    {
        if(pFile->mType == FileObject::Component)
        {
            QDir baseDir(path);

            includeCompString.append(QString("#include \"%1\"\n").arg(baseDir.relativeFilePath(pFile->mFileInfo.absoluteFilePath())));
            registerCompString.append(spaces(4)+QString("pComponentFactory->registerCreatorFunction(\"%1\", %2::Creator);\n").arg(pFile->mFileInfo.baseName(), pFile->mFileInfo.baseName()));
            xmlCompString.append(spaces(4)+QString("<component>%1</component>\n").arg(baseDir.relativeFilePath(pFile->mFileInfo.absoluteFilePath())));
        }
        else if(pFile->mType == FileObject::CAF)
        {
            QDir baseDir(path);

            xmlAppearanceString.append(spaces(4)+QString("<caf>%1</caf>\n").arg(baseDir.relativeFilePath(pFile->mFileInfo.absoluteFilePath())));
        }
    }

    sourceCode.replace("<<<libname>>>", mLibName);
    replacePatternLine(sourceCode, "<<<includecomponents>>>", includeCompString);
    replacePatternLine(sourceCode, "<<<registercomponents>>>",registerCompString);


    xmlCode.replace("<<<libname>>>", mLibName);
    xmlCode.replace("<<<debugext>>>", mLibDebugExt);
    xmlCode.replace("<<<sourcefile>>>", QFileInfo(sourceFile).fileName());
    replacePatternLine(xmlCode,"<<<components>>>",xmlCompString);
    replacePatternLine(xmlCode,"<<<auxiliary>>>","");
    replacePatternLine(xmlCode,"<<<caf>>>", xmlAppearanceString);

    sourceFile.write(sourceCode.toUtf8());
    xmlFile.write(xmlCode.toUtf8());

    sourceFile.close();
    xmlFile.close();

    loadFromXml(QFileInfo(xmlFile).absoluteFilePath());
}

void FileHandler::addComponent(QString path)
{
    if(mFilePtrs.isEmpty())
    {
        mpMessageHandler->addErrorMessage("A project must be open before adding components.");
        return;
    }

    //! @todo Make sure hpp file is in project directory

    if(path.isEmpty())
    {
        path = QFileDialog::getOpenFileName(mpEditorWidget->parentWidget(), "Add Component From Existing File", "", "*.hpp");
    }

    QFile file(path);
    if(!path.isEmpty() && file.exists())
    {
        FileObject *pFileObject = new FileObject(path, FileObject::Component);
        mFilePtrs.append(pFileObject);

        QTreeWidgetItem *pItem = mpFilesWidget->addFile(mFilePtrs.last());
        mTreeToFileMap.insert(pItem, mFilePtrs.last());

        generateXmlAndSourceFiles();
    }
}

void FileHandler::addComponent(const QString &code, const QString &typeName)
{
    if(mFilePtrs.isEmpty())
    {
        mpMessageHandler->addErrorMessage("A project must be open before adding components.");
        return;
    }

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


    addComponent(QFileInfo(componentFile).absoluteFilePath());
}


void FileHandler::addAppearanceFile(const QString &code, const QString &fileName)
{
    if(mFilePtrs.isEmpty())
    {
        mpMessageHandler->addErrorMessage("A project must be open before adding appearance files.");
        return;
    }

    QString path;
    foreach(const FileObject *pFile, mFilePtrs)
    {
        if(pFile->mType == FileObject::XML)
        {
            path = pFile->mFileInfo.absolutePath();
        }
    }

    if(path.isEmpty()) return;

    QFile cafFile(path+"/"+fileName);

    if(!cafFile.open(QFile::WriteOnly | QFile::Text))
    {
        mpMessageHandler->addErrorMessage("Cannot open file for writing: " + cafFile.fileName());
        return;
    }

    cafFile.write(code.toUtf8());

    cafFile.close();


    addAppearanceFile(QFileInfo(cafFile).absoluteFilePath());
}

void FileHandler::addAppearanceFile(QString path)
{
    if(mFilePtrs.isEmpty())
    {
        mpMessageHandler->addErrorMessage("A project must be open before adding appeaerance files.");
        return;
    }

    //! @todo Make sure caf file is in project directory

    if(path.isEmpty())
    {
        path = QFileDialog::getOpenFileName(mpEditorWidget->parentWidget(), "Add Component Appearance From Existing File", "", "*.xml");
    }

    QFile file(path);
    if(!path.isEmpty() && file.exists())
    {
        FileObject *pFileObject = new FileObject(path, FileObject::CAF);
        mFilePtrs.append(pFileObject);

        QTreeWidgetItem *pItem = mpFilesWidget->addFile(mFilePtrs.last());
        mTreeToFileMap.insert(pItem, mFilePtrs.last());

        generateXmlAndSourceFiles();
    }
}


void FileHandler::loadFromXml()
{
    QString path;
    foreach(const FileObject *file, mFilePtrs)
    {
        if(file->mType == FileObject::XML)
        {
            path = file->mFileInfo.absolutePath();
        }
    }

    path = QFileDialog::getOpenFileName(0, "Open Component Library", path, "*.xml");
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
        mpMessageHandler->addErrorMessage("Cannot open file for reading: "+path);
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

        QDomElement cafElement = libRoot.firstChildElement("caf");
        while(!cafElement.isNull())
        {
            mFilePtrs.append(new FileObject(info.absolutePath()+"/"+cafElement.text(), FileObject::CAF));
            pItem = mpFilesWidget->addFile(mFilePtrs.last());
            mTreeToFileMap.insert(pItem, mFilePtrs.last());
            cafElement = cafElement.nextSiblingElement("caf");
        }
    }
    file.close();

    mpConfiguration->setProjectPath(path);
}

void FileHandler::setFileNotSaved()
{
    mpFilesWidget->addAsterisk();
    if(mpFilesWidget->mpTreeWidget->currentItem() && mTreeToFileMap.contains(mpFilesWidget->mpTreeWidget->currentItem()))
        mTreeToFileMap.find(mpFilesWidget->mpTreeWidget->currentItem()).value()->mIsSaved = false;
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
    libElement.setAttribute("debug_ext", mLibDebugExt);
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
        else if(mFilePtrs[f]->mType == FileObject::CAF)
        {
            fileElement = domDocument.createElement("caf");
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
    if(mpConfiguration->getIncludePath().isEmpty())
    {
        mpMessageHandler->addErrorMessage("Hopsan path is not setup correctly.");
        return;
    }

    if(mpConfiguration->getAlwaysSaveBeforeCompiling())
    {
        saveToXml();
    }
    else
    {
        bool allSaved=true;
        foreach(const FileObject *file, mFilePtrs)
        {
            if(!file->mIsSaved)
            {
                allSaved=false;
            }
        }
        if(!allSaved)
        {
            QDialog *pSaveDialog = new QDialog(mpEditorWidget->parentWidget());
            pSaveDialog->setWindowTitle("Warning!");

            QVBoxLayout *pSaveDialogLayout = new QVBoxLayout(pSaveDialog);

            QLabel *pSaveDialogLabel = new QLabel("All files are not saved. Save all files before compiling?");
            QCheckBox *pAlwaysSaveCheckBox = new QCheckBox("Always save files before compiling", pSaveDialog);
            QDialogButtonBox *pSaveDialogButtonBox = new QDialogButtonBox(QDialogButtonBox::Yes | QDialogButtonBox::No, Qt::Horizontal, pSaveDialog);

            connect(pAlwaysSaveCheckBox, SIGNAL(toggled(bool)), pSaveDialogButtonBox->button(QDialogButtonBox::No), SLOT(setDisabled(bool)));

            pSaveDialogLayout->addWidget(pSaveDialogLabel);
            pSaveDialogLayout->addWidget(pAlwaysSaveCheckBox);
            pSaveDialogLayout->addWidget(pSaveDialogButtonBox);

            connect(pSaveDialogButtonBox->button(QDialogButtonBox::Yes), SIGNAL(clicked()), pSaveDialog, SLOT(accept()));
            connect(pSaveDialogButtonBox->button(QDialogButtonBox::Yes), SIGNAL(clicked()), pSaveDialog, SLOT(close()));
            connect(pSaveDialogButtonBox->button(QDialogButtonBox::No), SIGNAL(clicked()), pSaveDialog, SLOT(reject()));
            connect(pSaveDialogButtonBox->button(QDialogButtonBox::No), SIGNAL(clicked()), pSaveDialog, SLOT(close()));

            int ret = pSaveDialog->exec();

            qDebug() << "ret = " << ret;

            if(ret == QDialog::Accepted)
            {
                saveToXml();
                if(pAlwaysSaveCheckBox->isChecked())
                {
                    mpConfiguration->setAlwaysSaveBeforeCompiling(true);
                }
            }
        }
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

    includeDirs.append(mpConfiguration->getIncludePath());
    libs.append(mpConfiguration->getHopsanCoreLibPath());

    QString target = mLibTarget;
#ifdef linux
    target.prepend("lib");
#endif

#ifdef linux
    QString compilerPath = mpConfiguration->getCompilerPath()+"/gcc";
#elif WIN32
    QString compilerPath = mpConfiguration->getCompilerPath()+"/g++.exe";
#endif

    bool success;
    QStringList output = compileComponentLibrary(compilerPath, path, target, sources, libs, includeDirs, success);
    //! @todo Do something with the success variable

    foreach(const QString &line, output)
    {
        QString newLine = line;     //Chop extra newlines (if they exist)
        if(newLine.endsWith("\r"))
            newLine.chop(1);
        if(newLine.contains("error:"))
        {
            mpMessageHandler->addErrorMessage(newLine);
        }
        else if(newLine.contains("warning:"))
        {
            mpMessageHandler->addWarningMessage(newLine);
        }
        else
        {
            mpMessageHandler->addInfoMessage(newLine);
        }
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
    if(mTreeToFileMap.find(pItem).value()->mType == FileObject::XML ||
       mTreeToFileMap.find(pItem).value()->mType == FileObject::Source)
    {
        mpMessageHandler->addErrorMessage("Project files cannot be removed from project.");
        return;
    }

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
