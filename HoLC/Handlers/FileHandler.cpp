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
#include <QUuid>

#include "Configuration.h"
#include "FileHandler.h"
#include "Widgets/ProjectFilesWidget.h"
#include "Widgets/EditorWidget.h"
#include "MessageHandler.h"
#include "Utilities/CompilingUtilities.h"
#include "Utilities/StringUtilities.h"

FileHandler::FileHandler(Configuration *pConfiuration, ProjectFilesWidget *pFilesWidget, EditorWidget *pEditorWidget, MessageHandler *pMessageHandler) :
    QObject(nullptr)
{
    mpConfiguration = pConfiuration;
    mpFilesWidget = pFilesWidget;
    mpEditorWidget = pEditorWidget;
    mpMessageHandler = pMessageHandler;

    connect(pFilesWidget->mpTreeWidget, SIGNAL(itemClicked(QTreeWidgetItem*,int)), this, SLOT(openFile(QTreeWidgetItem*, int)));
    connect(pFilesWidget, SIGNAL(deleteRequested(QTreeWidgetItem*)), this, SLOT(removeFile(QTreeWidgetItem*)));

    mpCurrentFile.clear();
    mLibraryDebugExtension = "_d";
}

void FileHandler::generateNewXmlAndSourceFiles(const QString &libName, QString &path)
{
    mLibraryMainXMLFile.clear();
    mLibraryMainCPPFile.clear();
    mLibraryFiles.clear();
    mTreeToFileMap.clear();
   //mpFilesWidget->mpTreeWidget->clear();
    mpFilesWidget->clear();
    mLibraryId.clear();

    mLibraryName = libName;
    mLibraryCompiledFile = libName;

    generateXmlAndSourceFiles(path);
}

void FileHandler::generateXmlAndSourceFiles(QString path)
{
    if(path.isEmpty())
    {
        // Use the path of the first available file, the current library file.
        if (!mLibraryFiles.isEmpty())
        {
            path = mLibraryFiles.front()->mFileInfo.absolutePath();
        }
        else
        {
            mpMessageHandler->addErrorMessage("Unable to generate source files, no target location given");
            return;
        }
    }

    if (mLibraryMainXMLFile.isEmpty()) {
        mLibraryMainXMLFile = path+"/"+mLibraryName+".xml";
    }
    if (mLibraryMainCPPFile.isEmpty()) {
        mLibraryMainCPPFile = path+"/"+mLibraryName+".cpp";
    }

    QFile xmlFile(mLibraryMainXMLFile);
    QFile sourceFile(mLibraryMainCPPFile);
    QFile xmlTemplateFile(":Templates/xmlTemplate.xml");
    QFile sourceTemplateFile(":Templates/sourceTemplate.cpp");

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
    for(const auto& pFile : mLibraryFiles)
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

            xmlAppearanceString.append(spaces(4)+QString("<componentxml>%1</componentxml>\n").arg(baseDir.relativeFilePath(pFile->mFileInfo.absoluteFilePath())));
        }
    }

    sourceCode.replace("<<<libname>>>", mLibraryName);
    replacePatternLine(sourceCode, "<<<includecomponents>>>", includeCompString);
    replacePatternLine(sourceCode, "<<<registercomponents>>>",registerCompString);

    if (mLibraryId.isEmpty()) {
        mLibraryId = QUuid::createUuid().toString().remove('{').remove('}');
    }
    xmlCode.replace("<<<libid>>>", mLibraryId);
    xmlCode.replace("<<<libname>>>", mLibraryName);
    xmlCode.replace("<<<debugext>>>", mLibraryDebugExtension);
    //! @todo add support for entering cflags and lflags
    xmlCode.replace("<<<cflags>>>", "");
    xmlCode.replace("<<<lflags>>>", "");
    xmlCode.replace("<<<includepaths>>>", "");
    xmlCode.replace("<<<linkpaths>>>", "");
    xmlCode.replace("<<<linklibraries>>>", "");
    xmlCode.replace("<<<sourcefile>>>", QFileInfo(sourceFile).fileName());
    replacePatternLine(xmlCode,"<<<components>>>",xmlCompString);
    replacePatternLine(xmlCode,"<<<auxiliary>>>","");
    replacePatternLine(xmlCode,"<<<componentxml>>>", xmlAppearanceString);

    sourceFile.write(sourceCode.toUtf8());
    xmlFile.write(xmlCode.toUtf8());

    sourceFile.close();
    xmlFile.close();

    loadLibraryFromXml(QFileInfo(xmlFile).absoluteFilePath());
}

void FileHandler::addComponent(QString existingPath)
{
    if(mLibraryFiles.isEmpty())
    {
        mpMessageHandler->addErrorMessage("A project must be open before adding components.");
        return;
    }

    // Get directory of last added code file
    QString dir;
    for(const auto& pFile : mLibraryFiles)
    {
        if(pFile->mType == FileObject::Source) {
            dir = pFile->mFileInfo.absolutePath();
        }
    }

    //! @todo Make sure hpp file is in project directory

    QStringList paths;
    if(existingPath.isEmpty())
    {
        paths = QFileDialog::getOpenFileNames(mpEditorWidget->parentWidget(), "Add Component From Existing File", dir, "Hopsan component sources (*.hpp)");
    }
    else if(QFile::exists(existingPath))
    {
        paths.append(existingPath);
    }

    bool didAdd = false;
    for (const auto& newPath : paths)
    {
        QFile file(newPath);
        if(file.exists())
        {
            mLibraryFiles.append(QSharedPointer<FileObject>(new FileObject(newPath, FileObject::Component)));

            QTreeWidgetItem *pItem = mpFilesWidget->addFile(mLibraryFiles.last());
            mTreeToFileMap.insert(pItem, mLibraryFiles.last());

            didAdd = true;
        }
    }

    if (didAdd) {
        generateXmlAndSourceFiles();
    }
}

void FileHandler::addComponent(const QString &code, const QString &typeName)
{
    if(mLibraryFiles.isEmpty())
    {
        mpMessageHandler->addErrorMessage("A project must be open before adding components.");
        return;
    }

    QString path;
    for(const auto& pFile : mLibraryFiles)
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
    if(mLibraryFiles.isEmpty())
    {
        mpMessageHandler->addErrorMessage("A project must be open before adding appearance files.");
        return;
    }

    QString path;
    for(const auto& pFile : mLibraryFiles)
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

void FileHandler::addAppearanceFile(QString existingPath)
{
    if(mLibraryFiles.isEmpty())
    {
        mpMessageHandler->addErrorMessage("A project must be open before adding appearance files.");
        return;
    }

    // Get directory of last added xml file
    QString dir;
    for(const auto& pFile : mLibraryFiles)
    {
        if(pFile->mType == FileObject::XML) {
            dir = pFile->mFileInfo.absolutePath();
        }
    }

    //! @todo Make sure caf file is in project directory

    QStringList paths;
    if(existingPath.isEmpty())
    {
        paths = QFileDialog::getOpenFileNames(mpEditorWidget->parentWidget(), "Add Component Appearance From Existing File", dir, "XML files (*.xml)");
    }
    else if(QFile::exists(existingPath))
    {
        paths.append(existingPath);
    }

    bool didAdd = false;
    for (const auto& newPath : paths)
    {
        QFile file(newPath);
        if(file.exists())
        {
            mLibraryFiles.append(QSharedPointer<FileObject>(new FileObject(newPath, FileObject::CAF)));

            QTreeWidgetItem *pItem = mpFilesWidget->addFile(mLibraryFiles.last());
            mTreeToFileMap.insert(pItem, mLibraryFiles.last());

            didAdd = true;
        }
    }

    if (didAdd) {
        generateXmlAndSourceFiles();
    }
}


void FileHandler::loadLibraryFromXml()
{
    QString path;
    if (!mLibraryFiles.isEmpty()) {
        // The first one should be the current project loaded
        path = mLibraryFiles.front()->mFileInfo.absolutePath();
    }

    path = QFileDialog::getOpenFileName(nullptr, "Open Component Library", path, "XML files (*.xml)");
    if(!path.isEmpty())
    {
        loadLibraryFromXml(path);
    }
}

void FileHandler::saveToXml()
{
    for(int i=0; i<mLibraryFiles.size(); ++i)
    {
        if(mLibraryFiles[i]->mType == FileObject::XML)
        {
            saveToXml(mLibraryFiles[i]->mFileInfo.absoluteFilePath());
        }
    }
}


void FileHandler::loadLibraryFromXml(const QString &path)
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
        mpConfiguration->addRecentLibrary(path);

        QDomElement libRoot = domDocument.documentElement();

        if(libRoot.tagName() != "hopsancomponentlibrary")
        {
            mpMessageHandler->addErrorMessage(tr("Not a Hopsan component library! Root tag: %1 != hopsancomponentlibrary")
                                              .arg(libRoot.tagName()));
            return;
        }

        mLibraryFiles.clear();
        mpFilesWidget->clear();

        mLibraryMainXMLFile = path;
        mLibraryId = libRoot.firstChildElement("id").text();

        mLibraryName = libRoot.firstChildElement("name").text();
        // If the element did not exist (or name empty) try loading according to old format
        if (mLibraryName.isEmpty()) {
            mLibraryName = libRoot.attribute("name");
        }

        // The first file is the library xml file
        mLibraryFiles.append(QSharedPointer<FileObject>(new FileObject(path, FileObject::XML)));

        QDomElement libElement = libRoot.firstChildElement("lib");
        if(!libElement.isNull())
        {
            mLibraryCompiledFile = libElement.text();
        }

        QDomElement sourceElement = libRoot.firstChildElement("source");
        if(!sourceElement.isNull())
        {
            mLibraryFiles.append(QSharedPointer<FileObject>(new FileObject(info.absolutePath()+"/"+sourceElement.text(), FileObject::Source)));
            mLibraryMainCPPFile = mLibraryFiles.last()->mFileInfo.absoluteFilePath();
        }

        QDomElement extraSourceElement = libRoot.firstChildElement("extrasource");
        if(!extraSourceElement.isNull())
        {
            mLibraryFiles.append(QSharedPointer<FileObject>(new FileObject(info.absolutePath()+"/"+extraSourceElement.text(), FileObject::ExtraSource)));
        }

        QDomElement bfElement = libRoot.firstChildElement("buildflags");
        QDomElement cflagsElement = bfElement.firstChildElement("cflags");
        while(!cflagsElement.isNull()) {
            mLibraryBuildFlags.append({"c", cflagsElement.attribute("os"), cflagsElement.text()});
            cflagsElement = cflagsElement.nextSiblingElement("cflags");
        }
        QDomElement lflagsElement = bfElement.firstChildElement("lflags");
        while(!lflagsElement.isNull()) {
            mLibraryBuildFlags.append({"l", lflagsElement.attribute("os"), lflagsElement.text()});
            lflagsElement = lflagsElement.nextSiblingElement("lflags");
        }

        QDomElement includePathElement = libRoot.firstChildElement("includepath");
        while(!includePathElement.isNull()) {
            mLibraryIncludePaths.append(includePathElement.text());
            includePathElement = includePathElement.nextSiblingElement("includepath");
        }

        QDomElement linkPathElement = libRoot.firstChildElement("linkpath");
        while(!linkPathElement.isNull()) {
            mLibraryLinkPaths.append(linkPathElement.text());
            linkPathElement = linkPathElement.nextSiblingElement("linkpath");
        }

        QDomElement linkLibraryElement = libRoot.firstChildElement("linklibrary");
        while(!linkLibraryElement.isNull()) {
            mLibraryLinkLibraries.append(linkLibraryElement.text());
            linkLibraryElement = linkLibraryElement.nextSiblingElement("linklibrary");
        }

        QDomElement compElement = libRoot.firstChildElement("component");
        while(!compElement.isNull())
        {
            mLibraryFiles.append(QSharedPointer<FileObject>(new FileObject(info.absolutePath()+"/"+compElement.text(), FileObject::Component)));
            compElement = compElement.nextSiblingElement("component");
        }

        QDomElement auxElement = libRoot.firstChildElement("auxiliary");
        while(!auxElement.isNull())
        {
            mLibraryFiles.append(QSharedPointer<FileObject>(new FileObject(info.absolutePath()+"/"+auxElement.text(), FileObject::Auxiliary)));
            auxElement = auxElement.nextSiblingElement("auxiliary");
        }

        // This name have varied between files, so checking all of them
        // Note! componentxml is the name that should be used
        for (const auto& name : {"componentxml", "hopsanobjectappearance", "caf"} ) {
            QDomElement cafElement = libRoot.firstChildElement(name);
            while(!cafElement.isNull())
            {
                mLibraryFiles.append(QSharedPointer<FileObject>(new FileObject(info.absolutePath()+"/"+cafElement.text(), FileObject::CAF)));
                cafElement = cafElement.nextSiblingElement(name);
            }
        }

        // Add all opened files to the widget and map
        for (auto pFile : mLibraryFiles) {
            QTreeWidgetItem *pItem = mpFilesWidget->addFile(pFile);
            mTreeToFileMap.insert(pItem, pFile);
        }
    }
    file.close();

    mpConfiguration->setProjectPath(path);
}

void FileHandler::setFileNotSaved()
{
    mpFilesWidget->addAsterisk();
    auto it = mTreeToFileMap.find(mpFilesWidget->mpTreeWidget->currentItem());
    if (it != mTreeToFileMap.end()) {
        it.value()->mIsSaved = false;
    }
}

bool FileHandler::hasFile(QString filePath)
{
    QFileInfo info(filePath);
    for (const auto& pFile : mLibraryFiles)
    {
        if(info.fileName() == pFile->mFileInfo.fileName())
        {
            return true;
        }
    }
    return false;
}

void FileHandler::reloadFile()
{
    QString path = mpCurrentFile->mFileInfo.absoluteFilePath();
    FileObject::FileTypeEnum type = mpCurrentFile->mType;
    mpCurrentFile.clear();
    mLibraryFiles.removeAll(mpCurrentFile);
    mLibraryFiles.append(QSharedPointer<FileObject>(new FileObject(path, type)));
    this->openFile(mLibraryFiles.last());
}

void FileHandler::saveToXml(const QString &filePath)
{
    mpFilesWidget->removeAsterisks();

    for (const auto& pFile : mLibraryFiles)
    {
        QFile file(pFile->mFileInfo.absoluteFilePath());
        file.open(QFile::WriteOnly | QFile::Text);
        file.write(pFile->mFileContents.toUtf8());
        file.close();
    }

    QString path = QFileInfo(filePath).absolutePath();

    QDomDocument domDocument;
    QDomElement libRoot = domDocument.createElement("hopsancomponentlibrary");
    libRoot.setAttribute("version", "0.2");
    domDocument.appendChild(libRoot);

    if (mLibraryId.isEmpty()) {
        mLibraryId = QUuid::createUuid().toString().remove('{').remove('}');
    }
    QDomElement libIdElement = domDocument.createElement("id");
    libIdElement.appendChild(domDocument.createTextNode(mLibraryId));
    libRoot.appendChild(libIdElement);

    QDomElement libNameElement = domDocument.createElement("name");
    libNameElement.appendChild(domDocument.createTextNode(mLibraryName));
    libRoot.appendChild(libNameElement);

    QDomElement libElement = domDocument.createElement("lib");
    libElement.setAttribute("debug_ext", mLibraryDebugExtension);
    libElement.appendChild(domDocument.createTextNode(mLibraryCompiledFile));
    libRoot.appendChild(libElement);

    QDomElement bfElement = domDocument.createElement("buildflags");
    for (const auto& bf : mLibraryBuildFlags) {
        QDomElement flagsElement = domDocument.createElement(QString("%1flags").arg(bf.type));
        flagsElement.setAttribute("os", bf.os);
        flagsElement.appendChild(domDocument.createTextNode(bf.content));
        bfElement.appendChild(flagsElement);
    }
    libRoot.appendChild(bfElement);

    for(const QString includePath : mLibraryIncludePaths) {
        QDomElement includePathElement = domDocument.createElement("includepath");
        includePathElement.appendChild(domDocument.createTextNode(includePath));
        libRoot.appendChild(includePathElement);
    }

    for(const QString linkPath : mLibraryLinkPaths) {
        QDomElement linkPathElement = domDocument.createElement("linkpath");
        linkPathElement.appendChild(domDocument.createTextNode(linkPath));
        libRoot.appendChild(linkPathElement);
    }

    for(const QString linkLibrary : mLibraryLinkLibraries) {
        QDomElement linkLibraryElement = domDocument.createElement("linklibrary");
        linkLibraryElement.appendChild(domDocument.createTextNode(linkLibrary));
        libRoot.appendChild(linkLibraryElement);
    }

    for(int f=0; f<mLibraryFiles.size(); ++f)
    {
        QDomElement fileElement;
        if(mLibraryFiles[f]->mType == FileObject::Source)
        {
            fileElement = domDocument.createElement("source");
        }
        if(mLibraryFiles[f]->mType == FileObject::ExtraSource)
        {
            fileElement = domDocument.createElement("extrasource");
        }
        else if(mLibraryFiles[f]->mType == FileObject::Component)
        {
            fileElement = domDocument.createElement("component");
        }
        else if(mLibraryFiles[f]->mType == FileObject::Auxiliary)
        {
            fileElement = domDocument.createElement("auxiliary");
        }
        else if(mLibraryFiles[f]->mType == FileObject::CAF)
        {
            fileElement = domDocument.createElement("componentxml");
        }
        fileElement.appendChild(domDocument.createTextNode(mLibraryFiles[f]->mFileInfo.absoluteFilePath().remove(path+"/")));
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
    domDocument.save(out, 2);
}

void FileHandler::updateText()
{
    if(mpCurrentFile)
    {
        mpCurrentFile->mFileContents = mpEditorWidget->getText();
    }
}

void FileHandler::compileLibrary()
{
    mpMessageHandler->clear();
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
        for(const auto& pFile : mLibraryFiles)
        {
            if(!pFile->mIsSaved)
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

    for(const auto& pFile : mLibraryFiles)
    {
        if(pFile->mType == FileObject::XML)
        {
            path = pFile->mFileInfo.absolutePath();
        }
        else if(pFile->mType == FileObject::Source || pFile->mType == FileObject::ExtraSource)
        {
            sources.append(pFile->mFileInfo.absoluteFilePath());
        }
    }

    includeDirs.append(mpConfiguration->getIncludePath());
    libs.append(mpConfiguration->getHopsanCoreLibPath());

    QString target = mLibraryCompiledFile;
#ifdef __linux__
    target.prepend("lib");
#endif

    QString compilerPath = mpConfiguration->getCompilerPath();
#ifdef _WIN32
    compilerPath = compilerPath+"/g++.exe";
#else
    if (!compilerPath.isEmpty()) {
        compilerPath.append("/");
    }
    compilerPath.append("gcc");
    //! @todo support other compilers
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

    openFile(mTreeToFileMap.find(pItem).value());
}

void FileHandler::openFile(QSharedPointer<FileObject> pFile)
{
    mpCurrentFile = pFile;

    EditorWidget::HighlighterTypeEnum highlightType = EditorWidget::HighlighterTypeEnum::PlainText;
    QString fileName = mpCurrentFile->mFileInfo.fileName();
    bool editingEnabled=true;
    if(mpCurrentFile->mType == FileObject::XML || mpCurrentFile->mType == FileObject::Source)
    {
        editingEnabled=false;
    }
    if(fileName.endsWith(".xml"))
    {
        highlightType = EditorWidget::XML;
    }
    else if(fileName.endsWith(".hpp") || fileName.endsWith(".c") || fileName.endsWith(".cpp") ||
            fileName.endsWith(".cc") || fileName.endsWith(".h"))
    {
        highlightType = EditorWidget::Cpp;
    }
    mpEditorWidget->setText(mpCurrentFile->mFileContents, highlightType, editingEnabled);

    emit fileOpened(false);
}

void FileHandler::removeFile(QTreeWidgetItem *pItem)
{
    auto pFile = mTreeToFileMap.find(pItem).value();
    if( (pFile->mType == FileObject::XML) || (pFile->mType == FileObject::Source) )
    {
        mpMessageHandler->addErrorMessage("Project files cannot be removed from project.");
        return;
    }

    mLibraryFiles.removeAt(mLibraryFiles.indexOf(pFile));
    mpFilesWidget->removeItem(pItem);
    mpEditorWidget->clear();
    mTreeToFileMap.remove(pItem);

    generateXmlAndSourceFiles();
}


FileObject::FileObject(const QString &path, FileTypeEnum type)
{
    mFileInfo = QFileInfo(path);
    mIsSaved=true;

    mType = type;

    QFile file(path);
    mExists = file.open(QFile::ReadOnly | QFile::Text);
    mFileContents = file.readAll();
    file.close();
}

bool FileObject::operator==(const FileObject &other) const
{
    return (this->mFileInfo == other.mFileInfo);
}
