/*-----------------------------------------------------------------------------

 Copyright 2017 Hopsan Group

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.


 The full license is available in the file LICENSE.
 For details about the 'Hopsan Group' or information about Authors and
 Contributors see the HOPSANGROUP and AUTHORS files that are located in
 the Hopsan source code root directory.

-----------------------------------------------------------------------------*/

// $Id$

#include <QString>
#include <QtTest>
#include <QDir>
#include <QtXml>
#include <QFileInfo>
#include <iostream>
#include "HopsanEssentials.h"
#include "CoreUtilities/HopsanCoreMessageHandler.h"
#include "HopsanCoreVersion.h"

#ifndef DEFAULT_LIBRARY_ROOT
#define DEFAULT_LIBRARY_ROOT "../componentLibraries/defaultLibrary"
#endif

#ifndef HOPSAN_INTERNALDEFAULTCOMPONENTS
#define DEFAULTLIBFILE SHAREDLIB_PREFIX "defaultcomponentlibrary" HOPSAN_DEBUG_POSTFIX "." SHAREDLIB_SUFFIX
const std::string defaultLibraryFilePath = DEFAULT_LIBRARY_ROOT "/" DEFAULTLIBFILE;
#else
const std::string defaultLibraryFilePath = "";
#endif

using namespace hopsan;


class DefaultLibraryXMLTest : public QObject
{
    Q_OBJECT

public:
    DefaultLibraryXMLTest();

private Q_SLOTS:
    void initTestCase();
    void testIconPaths();
    void testPortNames();
    void testSourceCodeLink();
    void testDescriptionExists();

private:
    void recurseCollectXMLFiles(const QDir &rDir);
    void printCoreMessages()
    {
        while (mHopsanCore.getCoreMessageHandler()->getNumWaitingMessages() > 0) {
            HString message, type, tag;
            mHopsanCore.getCoreMessageHandler()->getMessage(message, type, tag);
            std::cout << type.c_str() << ": " << message.c_str() << std::endl;
        }
    }

    QFileInfoList mAllXMLFiles;
    HopsanEssentials mHopsanCore;
};

typedef QMap<QString, QString> IconNameMapT;

QDomElement loadXMLFileToDOM(const QFileInfo &rXMLFileInfo, QDomDocument &rDoc)
{
    QFile file(rXMLFileInfo.absoluteFilePath());
    if (!file.open(QIODevice::ReadOnly))
    {
        return QDomElement();
    }

    rDoc.setContent(&file);
    file.close();

    QDomElement root = rDoc.documentElement();
    if (root.tagName() == "hopsanobjectappearance")
    {
        return root;
    }
    else
    {
        return QDomElement();
    }
}

IconNameMapT extractIconPathsFromMO(const QDomElement dom)
{
    IconNameMapT map;
    QDomElement icon = dom.firstChildElement("icons").firstChildElement("icon");
    while (!icon.isNull())
    {
        map.insert(icon.attribute("type"), icon.attribute("path"));
        icon = icon.nextSiblingElement("icon");
    }
    return map;
}

QStringList extractPortNamesFromMO(const QDomElement dom)
{
    QStringList names;
    QDomElement port = dom.firstChildElement("ports").firstChildElement("port");
    while (!port.isNull())
    {
        names.append(port.attribute("name"));
        port = port.nextSiblingElement("port");
    }
    return names;
}

QString extractTypeNameFromMO(const QDomElement dom)
{
    return dom.attribute("typename");
}

QString extractSourceCodeLinkFromMO(const QDomElement dom)
{
    return dom.attribute("sourcecode");
}

DefaultLibraryXMLTest::DefaultLibraryXMLTest()
{
}

void DefaultLibraryXMLTest::initTestCase()
{
    mAllXMLFiles.clear();

    // Loop through all subdirs, to find all XML files, and store them in a list
    QDir libRoot(DEFAULT_LIBRARY_ROOT);
    QVERIFY2(libRoot.exists(), qPrintable(QString("Libroot: %1 could not be found!").arg(libRoot.absolutePath())));
    recurseCollectXMLFiles(libRoot);

    if (!defaultLibraryFilePath.empty())
    {
        bool loadOK = mHopsanCore.loadExternalComponentLib(defaultLibraryFilePath.c_str());
        if (!loadOK) {
            printCoreMessages();
        }
        QVERIFY2(loadOK, qPrintable(QString("Could not load the default component library file: %1").arg(defaultLibraryFilePath.c_str())));
    }
}

void DefaultLibraryXMLTest::testIconPaths()
{
    bool isOK = true;
    for (int i=0; i<mAllXMLFiles.size(); ++i)
    {
        QDomDocument doc;
        QDomElement mo = loadXMLFileToDOM(mAllXMLFiles[i].absoluteFilePath(),doc).firstChildElement("modelobject");
        while(!mo.isNull())
        {
            IconNameMapT map = extractIconPathsFromMO(mo);
            IconNameMapT::iterator it;
            for (it=map.begin(); it!=map.end(); ++it)
            {
                QString relPath = it.value();
                QFileInfo iconFile(mAllXMLFiles[i].absolutePath()+"/"+relPath);
                if (!iconFile.exists() && !relPath.startsWith(":graphics/"))
                {
                    QWARN(QString("The icon file %1 could not be found, linked from xml file: %2").arg(iconFile.absoluteFilePath()).arg(mAllXMLFiles[i].absoluteFilePath()).toStdString().c_str());
                    isOK = false;
                }
            }
            mo = mo.nextSiblingElement("modelobject");
        }
    }
    QVERIFY2(isOK, "There were at least one icon not found!");
}

void DefaultLibraryXMLTest::testPortNames()
{
    bool isOK = true;
    for (int i=0; i<mAllXMLFiles.size(); ++i)
    {
        QDomDocument doc;
        QDomElement mo = loadXMLFileToDOM(mAllXMLFiles[i].absoluteFilePath(),doc).firstChildElement("modelobject");
        while(!mo.isNull())
        {
            QString typeName = extractTypeNameFromMO(mo);
            QStringList portNames = extractPortNamesFromMO(mo);

            Component* pComponent = mHopsanCore.createComponent(typeName.toStdString().c_str());
            if (pComponent)
            {
                for (int p=0; p<portNames.size(); ++p )
                {
                    Port *pPort = pComponent->getPort(portNames[p].toStdString().c_str());
                    if (pPort == 0)
                    {
                        isOK = false;
                        QWARN(QString("Component: %1    Port: %2    exist in XML but not in CODE!").arg(typeName).arg(portNames[p]).toStdString().c_str());
                    }
                }
                mHopsanCore.removeComponent(pComponent);
            }
            else
            {
                QWARN(QString("Component: %1 exist in XML but not in core!").arg(typeName).toStdString().c_str());
            }
            mo = mo.nextSiblingElement("modelobject");
        }
    }
    QVERIFY2(isOK, "There were at least one port name mismatch in your XML and CODE");
}

void DefaultLibraryXMLTest::testSourceCodeLink()
{
    bool isOK = true;
    for (int i=0; i<mAllXMLFiles.size(); ++i)
    {
        QDomDocument doc;
        QDomElement mo = loadXMLFileToDOM(mAllXMLFiles[i].absoluteFilePath(),doc).firstChildElement("modelobject");
        while(!mo.isNull())
        {
            QString typeName = extractTypeNameFromMO(mo);
            QString sourceCode = extractSourceCodeLinkFromMO(mo);

            if (sourceCode.isEmpty())
            {
                QWARN(QString("SourceCode attribute not set for component %1!").arg(typeName).toStdString().c_str());
                //isOK = false;

                // ===================================
                // Add the code automatically
//                QString codefile = typeName+".hpp";
//                mo.setAttribute("sourcecode", codefile);
//                QFile f(mAllXMLFiles[i].absoluteFilePath());
//                if (f.open(QIODevice::WriteOnly | QIODevice::Text))
//                {
//                    QTextStream out(&f);
//                    QDomDocument doc = mo.ownerDocument();
//                    doc.save(out,4);
//                }
//                f.close();
                // ===================================
            }
            else
            {
                QFileInfo sourceFile(mAllXMLFiles[i].absolutePath()+"/"+sourceCode);
                if(!sourceFile.exists())
                {
                    QWARN(QString("SourceCode file: %1 for component %2 in file %3 is missing!").arg(sourceFile.absoluteFilePath()).arg(typeName).arg(mAllXMLFiles[i].canonicalFilePath()).toStdString().c_str());
                    isOK = false;
                }
            }
            mo = mo.nextSiblingElement("modelobject");
        }
    }
    QVERIFY2(isOK, "There were at least one sourcecode link not working!");
}


void DefaultLibraryXMLTest::testDescriptionExists()
{
    bool isOK = true;
    for (int i=0; i<mAllXMLFiles.size(); ++i)
    {
        QDomDocument doc;
        QDomElement mo = loadXMLFileToDOM(mAllXMLFiles[i].absoluteFilePath(),doc).firstChildElement("modelobject");
        while(!mo.isNull())
        {
            QString typeName = extractTypeNameFromMO(mo);

            if(mo.firstChildElement("help").isNull()) {
                QWARN(QString("No description available for component %1").arg(typeName).toStdString().c_str());
            }
            else {
                QDomElement mdElement = mo.firstChildElement("help").firstChildElement("md");
                if(!mdElement.isNull()) {
                    QFileInfo mdFile(mAllXMLFiles[i].absolutePath()+"/"+mdElement.text());
                    if(!mdFile.exists())
                    {
                        QWARN(QString("Description file: %1 for component %2 in file %3 is missing!").arg(mdFile.absoluteFilePath()).arg(typeName).arg(mAllXMLFiles[i].canonicalFilePath()).toStdString().c_str());
                        isOK = false;
                    }
                }
            }
            mo = mo.nextSiblingElement("modelobject");
        }
    }
    QVERIFY2(isOK, "There were at least one description file link not working.");
}


void DefaultLibraryXMLTest::recurseCollectXMLFiles(const QDir &rDir)
{
    QFileInfoList contents = rDir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot, QDir::DirsLast);
    for (int i=0;i<contents.size();++i)
    {
        if (contents[i].isFile() && contents[i].suffix().toLower() == "xml")
        {
            mAllXMLFiles.append(contents[i]);
        }
        else if (contents[i].isDir())
        {
            recurseCollectXMLFiles(contents[i].absoluteFilePath());
        }
    }
}

QTEST_APPLESS_MAIN(DefaultLibraryXMLTest)

#include "tst_defaultlibraryxmltest.moc"
