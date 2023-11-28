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

//!
//! @file   LibraryHandler.cpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2013-10-23
//!
//! @brief Contains a class for handling component libraries
//!
//$Id$

//Defines
#define XML_LIBRARY "hopsancomponentlibrary"
#define XML_VERSION "version"
#define XML_RECOMPILABLE "recompilable"
#define XML_LIBRARY_NAME "name"
#define XML_LIBRARY_ID "id"
#define XML_LIBRARY_LIB "lib"
#define XML_LIBRARY_LIB_DBGEXT "debug_ext"
#define XML_LIBRARY_CAF "caf"
#define XML_LIBRARY_SOURCE "source"
#define XML_LIBRARY_EXTRA_SOURCE "extrasource"
#define XML_LIBRARY_INCLUDEPATH "includepath"
#define XML_LIBRARY_LINKPATH "linkpath"
#define XML_LIBRARY_LINKLIBRARY "linklibrary"
#define XML_COMPONENT_XML "componentxml"

#include <QFileDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QTableWidget>
#include <QToolButton>
#include <QLabel>
#include <QHeaderView>
#include <QSizePolicy>
#include <QComboBox>
#include <QScrollArea>
#include <QScrollBar>
#include <QDialogButtonBox>
#include <QCheckBox>
#include <QApplication>
#include <QInputDialog>
#include <QFileDialog>
#include <QMessageBox>

#include <iostream>

//Hopsan includes
#include "LibraryHandler.h"
#include "global.h"
#include "Configuration.h"
#include "CoreAccess.h"
#include "DesktopHandler.h"
#include "GUIObjects/GUIModelObjectAppearance.h"
#include "ModelHandler.h"
#include "version_gui.h"
#include "MessageHandler.h"
#include "GUIObjects/GUIContainerObject.h"
#include "GeneratorUtils.h"
#include "Utilities/GUIUtilities.h"

namespace  {

//! @brief Chooses between canonicalFilePath and absoluteFilePath depnding on if file exists
//! @details canonicalFilePath returns empty string if file does not exist, making it less usefull for error messages about missing files
//! @param[in] fi A file info object
//! @returns The file path
QString visibleFilePath(const QFileInfo & fi) {
    if (fi.exists()) {
        return fi.canonicalFilePath();
    }
    else {
        return fi.absoluteFilePath();
    }
}

//! @brief Helpfunction to create full typename from type and subtype
//! @returns The full typename type|subtype, or type is subtype was empty
QString makeFullTypeString(const QString &rType, const QString &rSubType)
{
    if (rSubType.isEmpty())
    {
        return rType;
    }
    else
    {
       return rType+"|"+rSubType;
    }
}

//! @ brief Float comparisson of version numbers
bool libraryFormatVersionLessThen(double libraryFormatVersion, double referenceValue) {
    const auto diff = referenceValue - libraryFormatVersion;
    return (diff > 0) || ((diff < 0) && (fabs(diff) < 0.01));
}
}


//! @brief Constructor for library handler
LibraryHandler::LibraryHandler(QObject *parent)
    : QObject(parent)
{
    mUpConvertAllCAF = UndecidedToAll;

    mpDialog = new NewComponentDialog(gpMainWindowWidget);
}

void LibraryHandler::loadLibrary()
{
    QString libDir = QFileDialog::getExistingDirectory(gpMainWindowWidget, tr("Choose Library Directory"),
                                                   gpConfig->getStringSetting(CFG_EXTERNALLIBDIR),
                                                   QFileDialog::ShowDirsOnly
                                                   | QFileDialog::DontResolveSymlinks);
    if(libDir.isEmpty())
    {
        return;
    }
    else
    {
        gpConfig->setStringSetting(CFG_EXTERNALLIBDIR,libDir);

        loadLibrary(libDir);

//        // Check so that lib is not already loaded
//        if(!gpConfig->hasUserLib(libDir))
//        {
//            loadLibrary(libDir);
//        }
//        else
//        {
//            gpMessageHandler->addErrorMessage("Library " + libDir + " is already loaded!");
//        }
    }
}


//! @brief Loads a component library from either XML or folder (deprecated, for backwards compatibility)
//! @param loadPath Path to .xml file (or folder)
//! @param type Specifies whether library is internal or external
//! @param visibility Specifies whether library is visible or invisible
void LibraryHandler::loadLibrary(QString loadPath, LibraryTypeEnumT type, HiddenVisibleEnumT visibility, RecompileEnumT recompile)
{
    QFileInfo libraryLoadPathInfo(loadPath);
    QDir libraryLoadPathRootDir;
    QStringList foundLibraryXmlFiles;

    if (!libraryLoadPathInfo.exists())
    {
        gpMessageHandler->addWarningMessage("Library: "+libraryLoadPathInfo.absoluteFilePath()+" does not exist. The library was not loaded!");
        gpConfig->removeUserLib(loadPath);
        return;
    }

    // If loadpath is a directory then search for library xml files
    if(libraryLoadPathInfo.isDir())
    {
        // Remember root dir path (we use file path, to get all of the path)
        libraryLoadPathRootDir.setPath(libraryLoadPathInfo.absoluteFilePath());

        // Iterate over all xml files in folder and subfolders
        libraryLoadPathRootDir.setFilter(QDir::Files | QDir::Dirs | QDir::NoDot | QDir::NoDotDot);
        libraryLoadPathRootDir.setNameFilters(QStringList() << "*.xml");
        QDirIterator it(libraryLoadPathRootDir, QDirIterator::Subdirectories);
        while(it.hasNext())
        {
            // Read from the xml file
            QFile file(it.next());
            QFileInfo fileInfo(file);

            if(file.open(QIODevice::ReadOnly | QIODevice::Text))
            {
                QDomDocument domDocument;
                QString errorStr;
                int errorLine, errorColumn;
                if(domDocument.setContent(&file, false, &errorStr, &errorLine, &errorColumn))
                {
                    // Here only library xml files are interesting, other xml files are ignored
                    QDomElement xmlRoot = domDocument.documentElement();
                    if(xmlRoot.tagName() == QString(XML_LIBRARY))
                    {
                        foundLibraryXmlFiles.append(fileInfo.canonicalFilePath());
                    }
                }
                else
                {
                    gpMessageHandler->addWarningMessage(QString("When looking for Library XML files. Could not parse file: %1, Error: %2, Line: %3, Column: %4")
                                                        .arg(fileInfo.canonicalFilePath()).arg(errorStr).arg(errorLine).arg(errorColumn));
                }
            }
            else
            {
                gpMessageHandler->addWarningMessage(QString("When looking for Library XML files. Could not open file: %1").arg(visibleFilePath(fileInfo)));
            }
            file.close();
        }
    }
    else if (libraryLoadPathInfo.isFile())
    {
        // Remember root dir path
        libraryLoadPathRootDir.setPath(libraryLoadPathInfo.absolutePath());

        // Read from the file
        QFile file(libraryLoadPathInfo.absoluteFilePath());
        QFileInfo fileInfo(file);
        // IF this is an xml file, then it must be a library file
        if (fileInfo.suffix() == "xml")
        {
            if(file.open(QIODevice::ReadOnly | QIODevice::Text))
            {
                QDomDocument domDocument;
                QString errorStr;
                int errorLine, errorColumn;
                if(domDocument.setContent(&file, false, &errorStr, &errorLine, &errorColumn))
                {
                    // The document must have library root tag to be valid
                    QDomElement xmlRoot = domDocument.documentElement();
                    if(xmlRoot.tagName() == QString(XML_LIBRARY))
                    {
                        foundLibraryXmlFiles.append(fileInfo.canonicalFilePath());
                    }
                    else
                    {
                        gpMessageHandler->addErrorMessage(QString("The specified XML file does not have Hopsan library root element. Expected: %1, Found: %2, In: %3")
                                                          .arg(XML_LIBRARY).arg(xmlRoot.tagName()).arg(fileInfo.canonicalFilePath()));
                    }
                }
                else
                {
                    gpMessageHandler->addErrorMessage(QString("Could not parse File: %1, Error: %2, Line: %3, Column: %4. Is it a Library XML file?")
                                                      .arg(fileInfo.canonicalFilePath()).arg(errorStr).arg(errorLine).arg(errorColumn));
                }
            }
            else
            {
                gpMessageHandler->addErrorMessage(QString("Could not open (read) Library XML file: %1").arg(visibleFilePath(fileInfo)));
            }
            file.close();
        }
        else
        {
            gpMessageHandler->addErrorMessage(QString("The Library XML file must have file suffix .xml, File: %1").arg(visibleFilePath(fileInfo)));
        }
    }

    bool loadedSomething=false;
    if (!foundLibraryXmlFiles.isEmpty())
    {
        for (QString &xmlFile : foundLibraryXmlFiles)
        {
            SharedComponentLibraryPtrT pLib(new GUIComponentLibrary);
            pLib->loadPath = loadPath;
            pLib->xmlFilePath = xmlFile;
            pLib->type = type;
            if (loadLibrary(pLib, type, visibility, recompile))
            {
                loadedSomething = true;
            }
        }
        return;
    }
    else
    {
        gpMessageHandler->addErrorMessage("Did not find any libary xml files, fall-back " SHAREDLIB_SUFFIX " loading is no longer possible");
    }

    if (!loadedSomething)
    {
        gpConfig->removeUserLib(loadPath);
        gpMessageHandler->addWarningMessage("Could not find any component libraries to load!");
    }
}

bool LibraryHandler::isLibraryLoaded(const QString &rLibraryXmlPath, const QString &rLibraryFilePath) const
{
    for (const SharedComponentLibraryPtrT &pLib : mLoadedLibraries)
    {
        if (!rLibraryXmlPath.isEmpty() && (pLib->xmlFilePath == rLibraryXmlPath))
        {
            return true;
        }
        if (!rLibraryFilePath.isEmpty() && (pLib->libFilePath == rLibraryFilePath))
        {
            return true;
        }
    }
    return false;
}

QStringList LibraryHandler::getLoadedLibraryNames() const
{
    QStringList libraries;
    for (const auto& pLibrary : mLoadedLibraries) {
        libraries.append(pLibrary->name);
    }
    return libraries;
}

const SharedComponentLibraryPtrT LibraryHandler::getLibrary(const QString &id) const
{
    for (const auto& pLibrary : mLoadedLibraries) {
        if (pLibrary->id == id) {
            return pLibrary;
        }
    }
    return {};
}


//! @brief Returns all libraries of specified type among specified ids
//! @param[in] ids List with ids to search
//! @param[in] type Requested library type
const QVector<SharedComponentLibraryPtrT> LibraryHandler::getLibraries(const QStringList &ids, const LibraryTypeEnumT type) const
{
    QVector<SharedComponentLibraryPtrT> result;
    for (const auto& id : ids) {
        auto pLib = getLibrary(id);
        if ((type == LibraryTypeEnumT::AnyLib) || (pLib->type == type)) {
            result.append(pLib);
        }
    }
    return result;
}


//! @brief Returns all libraries of specified type
//! @param[in] type Requested library type
const QVector<SharedComponentLibraryPtrT> LibraryHandler::getLibraries(const LibraryTypeEnumT type) const
{
    QVector<SharedComponentLibraryPtrT> result;
    for (auto pLib : mLoadedLibraries) {
        if ((type == LibraryTypeEnumT::AnyLib) || (pLib->type == type)) {
            result.append(pLib);
        }
    }
    return result;
}

NewComponentDialog::NewComponentDialog(QWidget *parent)
    : QDialog(parent)
{
    this->setWindowTitle("Add Component");
    this->resize(1024,500);

    QScrollArea *pScrollArea = new QScrollArea(this);
    QWidget *pScrollWidget = new QWidget(this);
    QVBoxLayout *pLayout = new QVBoxLayout(pScrollWidget);
    pScrollArea->setWidgetResizable(true);
    pScrollArea->setWidget(pScrollWidget);

    QDialogButtonBox *pButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
    connect(pButtonBox, SIGNAL(accepted()), this, SLOT(validate()));
    connect(pButtonBox, SIGNAL(rejected()), this, SLOT(reject()));

    QVBoxLayout *pMainLayout = new QVBoxLayout(this);
    pMainLayout->addWidget(pScrollArea);
    pMainLayout->addWidget(pButtonBox);

    // General settings

    QLabel *pGeneralLabel = new QLabel("General settings");
    pLayout->addWidget(pGeneralLabel);
    QFont boldFont = pGeneralLabel->font();
    boldFont.setBold(true);
    pGeneralLabel->setFont(boldFont);

    setStyleSheet("QTableWidget {background-color: transparent;}");
    mpGeneralTable = new QTableWidget(this);
    mpGeneralTable->setColumnCount(2);
    mpGeneralTable->setRowCount(5);
#if QT_VERSION >= 0x050000  //not available in Qt4
    mpGeneralTable->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
#endif
    mpGeneralTable->verticalHeader()->setVisible(false);
    mpGeneralTable->horizontalHeader()->setVisible(false);
    mpGeneralTable->setFrameStyle(QFrame::NoFrame);
#if QT_VERSION >= 0x050000  //not available in Qt4
    mpGeneralTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
#endif
    pLayout->addWidget(mpGeneralTable);
    addLabelItem(mpGeneralTable,0,0,"Type name:");
    addLabelItem(mpGeneralTable,1,0,"Display name:");
    addLabelItem(mpGeneralTable,2,0,"CQS type:");
    addLabelItem(mpGeneralTable,3,0,"Language:");
    addLabelItem(mpGeneralTable,4,0,"Integration Method:");
    addInputItem(mpGeneralTable,0,1);
    addInputItem(mpGeneralTable,1,1);
    mpCqsTypeComboBox = new QComboBox(this);
    mpCqsTypeComboBox->addItems(QStringList() << "S (signal)" << "Q (resistive)" << "C (capacitive)");
    mpGeneralTable->setCellWidget(2,1,mpCqsTypeComboBox);
    mpLanguageComboBox = new QComboBox(this);
    mpLanguageComboBox->addItem("");
    mpLanguageComboBox->addItem("");
    mpLanguageComboBox->setItemText(NewComponentLanguage::Cpp, "C++");
    mpLanguageComboBox->setItemText(NewComponentLanguage::Modelica, "Modelica");
    mpGeneralTable->setCellWidget(3,1,mpLanguageComboBox);
    mpIntegrationMethodComboBox = new QComboBox(this);
    mpIntegrationMethodComboBox->addItems(QStringList() << "Explicit Euler" << "Implicit Euler" << "Trapezoid Rule" << "BDF1" << "BDF2" << "BDF3" << "BDF4" << "BDF5");
    mpIntegrationMethodComboBox->setCurrentIndex(4);
    updateIntegrationMethodComboBoxVisibility();
    mpGeneralTable->setCellWidget(4,1,mpIntegrationMethodComboBox);
    connect(mpLanguageComboBox,SIGNAL(currentIndexChanged(int)),this,SLOT(updateIntegrationMethodComboBoxVisibility()));

    //Constants

    QHBoxLayout *pConstantsHeadingLayout = new QHBoxLayout();
    pLayout->addLayout(pConstantsHeadingLayout);

    QLabel *pConstantsLabel = new QLabel("Constant parameters");
    pConstantsLabel->setFont(boldFont);
    pConstantsHeadingLayout->addWidget(pConstantsLabel);

    QToolButton *pAddConstantToolButton = new QToolButton(this);
    pAddConstantToolButton->setIcon(QIcon(QString(ICONPATH)+"svg/Hopsan-Add.svg"));
    connect(pAddConstantToolButton, SIGNAL(clicked(bool)), this, SLOT(addConstantRow()));
    pConstantsHeadingLayout->addWidget(pAddConstantToolButton);
    pConstantsHeadingLayout->setStretch(1,1);

    mpConstantsTable = new QTableWidget(this);
#if QT_VERSION >= 0x050000  //not available in Qt4
    mpConstantsTable->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
#endif
    mpConstantsTable->verticalHeader()->setVisible(false);
    mpConstantsTable->horizontalHeader()->setVisible(false);
    mpConstantsTable->setFrameStyle(QFrame::NoFrame);
    pLayout->addWidget(mpConstantsTable);
    mpConstantsTable->setColumnCount(5);
    mpConstantsTable->setRowCount(1);
#if QT_VERSION >= 0x050000  //not available in Qt4
    mpConstantsTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    mpConstantsTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    mpConstantsTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    mpConstantsTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Stretch);
#endif
    mpConstantsTable->verticalScrollBar()->setDisabled(true);
    addLabelItem(mpConstantsTable,0,0,"");
    addLabelItem(mpConstantsTable,0,1,"Name:");
    addLabelItem(mpConstantsTable,0,2,"Description:");
    addLabelItem(mpConstantsTable,0,3,"Unit:");
    addLabelItem(mpConstantsTable,0,4,"Default value:");
    mpConstantsTable->horizontalHeader()->setMinimumSectionSize(1);
    mpConstantsTable->setColumnWidth(0,mpConstantsTable->rowHeight(0));
    mpConstantsTable->hide();

    // Input variables

    QHBoxLayout *pInputVariablesHeadingLayout = new QHBoxLayout();
    pLayout->addLayout(pInputVariablesHeadingLayout);

    QLabel *pInputVariablesLabel = new QLabel("Input variables");
    pInputVariablesLabel->setFont(boldFont);
    pInputVariablesHeadingLayout->addWidget(pInputVariablesLabel);

    QToolButton *pAddInputVariableToolButton = new QToolButton(this);
    pAddInputVariableToolButton->setIcon(QIcon(QString(ICONPATH)+"svg/Hopsan-Add.svg"));
    connect(pAddInputVariableToolButton, SIGNAL(clicked(bool)), this, SLOT(addInputVariableRow()));
    pInputVariablesHeadingLayout->addWidget(pAddInputVariableToolButton);
    pInputVariablesHeadingLayout->setStretch(1,1);

    mpInputVariablesTable = new QTableWidget(this);
#if QT_VERSION >= 0x050000  //not available in Qt4
    mpInputVariablesTable->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
#endif
    mpInputVariablesTable->verticalHeader()->setVisible(false);
    mpInputVariablesTable->horizontalHeader()->setVisible(false);
    mpInputVariablesTable->setFrameStyle(QFrame::NoFrame);
    pLayout->addWidget(mpInputVariablesTable);
    mpInputVariablesTable->setColumnCount(5);
    mpInputVariablesTable->setRowCount(1);
#if QT_VERSION >= 0x050000  //not available in Qt4
    mpInputVariablesTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    mpInputVariablesTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    mpInputVariablesTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    mpInputVariablesTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Stretch);
#endif
    mpInputVariablesTable->verticalScrollBar()->setDisabled(true);
    addLabelItem(mpInputVariablesTable,0,0,"");
    addLabelItem(mpInputVariablesTable,0,1,"Name:");
    addLabelItem(mpInputVariablesTable,0,2,"Description:");
    addLabelItem(mpInputVariablesTable,0,3,"Unit:");
    addLabelItem(mpInputVariablesTable,0,4,"Default value:");
    mpInputVariablesTable->horizontalHeader()->setMinimumSectionSize(1);
    mpInputVariablesTable->setColumnWidth(0,mpInputVariablesTable->rowHeight(0));
    mpInputVariablesTable->hide();

    // Output variables

    QHBoxLayout *pOutputVariablesHeadingLayout = new QHBoxLayout();
    pLayout->addLayout(pOutputVariablesHeadingLayout);

    QLabel *pOutputVariablesLabel = new QLabel("Output variables");
    pOutputVariablesLabel->setFont(boldFont);
    pOutputVariablesHeadingLayout->addWidget(pOutputVariablesLabel);

    QToolButton *pAddOutputVariableToolButton = new QToolButton(this);
    pAddOutputVariableToolButton->setIcon(QIcon(QString(ICONPATH)+"svg/Hopsan-Add.svg"));
    connect(pAddOutputVariableToolButton, SIGNAL(clicked(bool)), this, SLOT(addOutputVariableRow()));
    pOutputVariablesHeadingLayout->addWidget(pAddOutputVariableToolButton);
    pOutputVariablesHeadingLayout->setStretch(1,1);

    mpOutputVariablesTable = new QTableWidget(this);
#if QT_VERSION >= 0x050000  //not available in Qt4
    mpOutputVariablesTable->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
#endif
    mpOutputVariablesTable->verticalHeader()->setVisible(false);
    mpOutputVariablesTable->horizontalHeader()->setVisible(false);
    mpOutputVariablesTable->horizontalHeader()->setStretchLastSection(false);
    mpOutputVariablesTable->setFrameStyle(QFrame::NoFrame);
    pLayout->addWidget(mpOutputVariablesTable);
    mpOutputVariablesTable->setColumnCount(5);
    mpOutputVariablesTable->setRowCount(1);
#if QT_VERSION >= 0x050000  //not available in Qt4
    mpOutputVariablesTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    mpOutputVariablesTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    mpOutputVariablesTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    mpOutputVariablesTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Stretch);
#endif
    mpOutputVariablesTable->verticalScrollBar()->setDisabled(true);
    addLabelItem(mpOutputVariablesTable,0,0,"");
    addLabelItem(mpOutputVariablesTable,0,1,"Name:");
    addLabelItem(mpOutputVariablesTable,0,2,"Description:");
    addLabelItem(mpOutputVariablesTable,0,3,"Unit:");
    addLabelItem(mpOutputVariablesTable,0,4,"Default value:");
    mpOutputVariablesTable->horizontalHeader()->setMinimumSectionSize(1);
    mpOutputVariablesTable->setColumnWidth(0,mpOutputVariablesTable->rowHeight(0));
    mpOutputVariablesTable->hide();

    // Power ports

    QHBoxLayout *pPortsHeadingLayout = new QHBoxLayout();
    pLayout->addLayout(pPortsHeadingLayout);

    QLabel *pPortsLabel = new QLabel("Power ports");
    pPortsLabel->setFont(boldFont);
    pPortsHeadingLayout->addWidget(pPortsLabel);

    QToolButton *pAddPortToolButton = new QToolButton(this);
    pAddPortToolButton->setIcon(QIcon(QString(ICONPATH)+"svg/Hopsan-Add.svg"));
    connect(pAddPortToolButton, SIGNAL(clicked(bool)), this, SLOT(addPortRow()));
    pPortsHeadingLayout->addWidget(pAddPortToolButton);
    pPortsHeadingLayout->setStretch(1,1);

    mpPortsTable = new QTableWidget(this);
#if QT_VERSION >= 0x050000  //not available in Qt4
    mpPortsTable->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
#endif
    mpPortsTable->verticalHeader()->setVisible(false);
    mpPortsTable->horizontalHeader()->setVisible(false);
    mpPortsTable->horizontalHeader()->setStretchLastSection(false);
    mpPortsTable->setFrameStyle(QFrame::NoFrame);
    pLayout->addWidget(mpPortsTable);
    mpPortsTable->setColumnCount(5);
    mpPortsTable->setRowCount(1);
#if QT_VERSION >= 0x050000  //not available in Qt4
    mpPortsTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    mpPortsTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    mpPortsTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
#endif
    mpPortsTable->verticalScrollBar()->setDisabled(true);
    addLabelItem(mpPortsTable,0,0,"");
    addLabelItem(mpPortsTable,0,1,"Name:");
    addLabelItem(mpPortsTable,0,2,"Desription:");
    addLabelItem(mpPortsTable,0,3,"Node type:");
    addLabelItem(mpPortsTable,0,4,"Required:");
    mpPortsTable->horizontalHeader()->setMinimumSectionSize(1);
    mpPortsTable->setColumnWidth(0,mpPortsTable->rowHeight(0));
    mpPortsTable->hide();

    pLayout->addWidget(new QWidget(this));
    pLayout->setStretch(pLayout->count()-1,1);
}


ComponentSpecification NewComponentDialog::getSpecification()
{
    ComponentSpecification spec;
    spec.typeName = mpGeneralTable->item(0,1)->text();
    spec.displayName= mpGeneralTable->item(1,1)->text();
    int cqsNum = qobject_cast<QComboBox*>(mpGeneralTable->cellWidget(2,1))->currentIndex();
    switch(cqsNum)
    {
        case 0:
            spec.cqsType = "S";
            break;
        case 1:
            spec.cqsType = "Q";
            break;
        default:
            spec.cqsType = "C";
    }
    int language = qobject_cast<QComboBox*>(mpGeneralTable->cellWidget(3,1))->currentIndex();
    spec.modelica = (1 == language);
    if(spec.modelica) {
        switch (mpIntegrationMethodComboBox->currentIndex()) {
            case 0:
                spec.transform = "expliciteuler";
                break;
            case 1:
                spec.transform = "impliciteuler";
                break;
            case 2:
                spec.transform = "trapezoid";
                break;
            case 3:
                spec.transform = "bdf1";
                break;
            case 4:
                spec.transform = "bdf2";
                break;
            case 5:
                spec.transform = "bdf3";
                break;
            case 6:
                spec.transform = "bdf4";
                break;
            case 7:
                spec.transform = "bdf5";
                break;
        }
    }
    for(int r=1; r<mpConstantsTable->rowCount(); ++r) {
        spec.constantNames.append(mpConstantsTable->item(r,1)->text());
        spec.constantDescriptions.append(mpConstantsTable->item(r,2)->text());
        spec.constantUnits.append(mpConstantsTable->item(r,3)->text());
        spec.constantInits.append(mpConstantsTable->item(r,4)->text());
    }
    for(int r=1; r<mpInputVariablesTable->rowCount(); ++r) {
        spec.inputNames.append(mpInputVariablesTable->item(r,1)->text());
        spec.inputDescriptions.append(mpInputVariablesTable->item(r,2)->text());
        spec.inputUnits.append(mpInputVariablesTable->item(r,3)->text());
        spec.inputInits.append(mpInputVariablesTable->item(r,4)->text());
    }
    for(int r=1; r<mpOutputVariablesTable->rowCount(); ++r) {
        spec.outputNames.append(mpOutputVariablesTable->item(r,1)->text());
        spec.outputDescriptions.append(mpOutputVariablesTable->item(r,2)->text());
        spec.outputUnits.append(mpOutputVariablesTable->item(r,3)->text());
        spec.outputInits.append(mpOutputVariablesTable->item(r,4)->text());
    }
    for(int r=1; r<mpPortsTable->rowCount(); ++r) {
        spec.portNames.append(mpPortsTable->item(r,1)->text());
        spec.portDescriptions.append(mpPortsTable->item(r,2)->text());
        spec.portTypes.append(qobject_cast<QComboBox*>(mpPortsTable->cellWidget(r,3))->currentText());
        spec.portsRequired.append(qobject_cast<QCheckBox*>(mpPortsTable->cellWidget(r,4))->isChecked());
    }

    return spec;
}


void NewComponentDialog::validate()
{
    bool error=false;
    QBrush defaultBrush = mpGeneralTable->item(1,1)->background();
    QBrush errorBrush(QColor(255,0,0,150));

    //Clear all seletions (to make any errors visible)
    mpGeneralTable->clearSelection();
    mpConstantsTable->clearSelection();
    mpInputVariablesTable->clearSelection();
    mpOutputVariablesTable->clearSelection();
    mpPortsTable->clearSelection();

    //Reset background colors
    mpGeneralTable->item(0,1)->setBackground(defaultBrush);
    for(int r=1; r<mpConstantsTable->rowCount(); ++r) {
        mpConstantsTable->item(r,1)->setBackground(defaultBrush);
        mpConstantsTable->item(r,4)->setBackground(defaultBrush);
    }
    for(int r=1; r<mpInputVariablesTable->rowCount(); ++r) {
        mpInputVariablesTable->item(r,1)->setBackground(defaultBrush);
        mpInputVariablesTable->item(r,4)->setBackground(defaultBrush);
    }
    for(int r=1; r<mpOutputVariablesTable->rowCount(); ++r) {
        mpOutputVariablesTable->item(r,1)->setBackground(defaultBrush);
        mpOutputVariablesTable->item(r,4)->setBackground(defaultBrush);
    }
    for(int r=1; r<mpPortsTable->rowCount(); ++r) {
        mpPortsTable->item(r,1)->setBackground(defaultBrush);
    }

    //Validate type name
    ComponentSpecification spec = getSpecification();
    if(spec.typeName.isEmpty()) {
        gpMessageHandler->addErrorMessage("Please enter a type name.");
        mpGeneralTable->item(0,1)->setBackground(errorBrush);
        error=true;
    }
    else if(!spec.typeName.at(0).isLetter()) {
        gpMessageHandler->addErrorMessage("Type name must begin with a letter.");
        mpGeneralTable->item(0,1)->setBackground(errorBrush);
        error=true;
    }
    else if(!isNameValid(spec.typeName)) {
        gpMessageHandler->addErrorMessage("Type name can only contain letters, numbers and underscores.");
        mpGeneralTable->item(0,1)->setBackground(errorBrush);
        error=true;
    }
    auto entry = gpLibraryHandler->getEntry(spec.typeName);
    if(entry.isValid()) {
        gpMessageHandler->addErrorMessage("Component with type name \""+spec.typeName+"\" already exist in library \""+entry.pLibrary->name+"\".");
        mpGeneralTable->item(0,1)->setBackground(errorBrush);
        error=true;
    }

    //Make sure all variable names are unique
    QStringList allNames = spec.constantNames+spec.inputNames+spec.outputNames+spec.portNames;
    for(const QString& name : allNames) {
        if(allNames.count(name) > 1) {
            gpMessageHandler->addErrorMessage("Each variable and port name must be unique.");
            QList<QTableWidgetItem*> items = mpConstantsTable->findItems(name, Qt::MatchExactly);
            items += mpInputVariablesTable->findItems(name, Qt::MatchExactly);
            items += mpOutputVariablesTable->findItems(name, Qt::MatchExactly);
            items += mpPortsTable->findItems(name, Qt::MatchExactly);
            for(QTableWidgetItem *item : items) {
                if(item->column() == 1) {
                    item->setBackground(errorBrush);
                }
            }
            error=true;
        }
    }

    mpConstantsTable->selectColumn(4);
    mpInputVariablesTable->selectColumn(4);
    mpOutputVariablesTable->selectColumn(4);
    QList<QTableWidgetItem*> items = mpConstantsTable->selectedItems()+mpInputVariablesTable->selectedItems()+mpOutputVariablesTable->selectedItems();
    mpConstantsTable->clearSelection();
    mpInputVariablesTable->clearSelection();
    mpOutputVariablesTable->clearSelection();
    for(QTableWidgetItem* pItem : items) {
        if(pItem->row() == 0) {
            continue;   //Ignore title row
        }
        bool isNumerical;
        pItem->text().toDouble(&isNumerical);
        if(!isNumerical) {
            gpMessageHandler->addErrorMessage("Initial values must be numerical");
            pItem->setBackground(errorBrush);
            error = true;
        }
    }

    if(error) {
        return;
    }

    QDialog::accept();
}



void NewComponentDialog::addConstantRow()
{
    int row = mpConstantsTable->rowCount();
    mpConstantsTable->insertRow(row);
    addInputItem(mpConstantsTable,row,1);
    addInputItem(mpConstantsTable,row,2);
    addInputItem(mpConstantsTable,row,3);
    addInputItem(mpConstantsTable,row,4,"0");
    while(mRemoveConstantToolButtons.size() < row) {
        mRemoveConstantToolButtons.push_back(new QToolButton(this));
        mRemoveConstantToolButtons.last()->setIcon(QIcon(QString(ICONPATH)+"svg/Hopsan-Discard.svg"));
        connect(mRemoveConstantToolButtons.last(), SIGNAL(clicked(bool)), this, SLOT(removeConstantRow()));
    }
    mpConstantsTable->setCellWidget(row,0,mRemoveConstantToolButtons.at(row-1));
    adjustTableSize(mpConstantsTable);
    mpConstantsTable->setVisible(true);
}

void NewComponentDialog::addInputVariableRow()
{
    int row = mpInputVariablesTable->rowCount();
    mpInputVariablesTable->insertRow(row);
    addInputItem(mpInputVariablesTable,row,1);
    addInputItem(mpInputVariablesTable,row,2);
    addInputItem(mpInputVariablesTable,row,3);
    addInputItem(mpInputVariablesTable,row,4,"0");
    while(mRemoveInputVariableToolButtons.size() < row) {
        mRemoveInputVariableToolButtons.push_back(new QToolButton(this));
        mRemoveInputVariableToolButtons.last()->setIcon(QIcon(QString(ICONPATH)+"svg/Hopsan-Discard.svg"));
        connect(mRemoveInputVariableToolButtons.last(), SIGNAL(clicked(bool)), this, SLOT(removeInputVariableRow()));
    }
    mpInputVariablesTable->setCellWidget(row,0,mRemoveInputVariableToolButtons.at(row-1));
    adjustTableSize(mpInputVariablesTable);
    mpInputVariablesTable->setVisible(true);
}

void NewComponentDialog::addOutputVariableRow()
{
    int row = mpOutputVariablesTable->rowCount();
    mpOutputVariablesTable->insertRow(row);
    addInputItem(mpOutputVariablesTable,row,1);
    addInputItem(mpOutputVariablesTable,row,2);
    addInputItem(mpOutputVariablesTable,row,3);
    addInputItem(mpOutputVariablesTable,row,4,"0");
    while(mRemoveOutputVariableToolButtons.size() < row) {
        mRemoveOutputVariableToolButtons.push_back(new QToolButton(this));
        mRemoveOutputVariableToolButtons.last()->setIcon(QIcon(QString(ICONPATH)+"svg/Hopsan-Discard.svg"));
        connect(mRemoveOutputVariableToolButtons.last(), SIGNAL(clicked(bool)), this, SLOT(removeOutputVariableRow()));
    }
    mpOutputVariablesTable->setCellWidget(row,0,mRemoveOutputVariableToolButtons.at(row-1));
    adjustTableSize(mpOutputVariablesTable);
    mpOutputVariablesTable->setVisible(true);
}

void NewComponentDialog::addPortRow()
{
    int row = mpPortsTable->rowCount();
    mpPortsTable->insertRow(row);
    QComboBox *pTypeComboBox = new QComboBox(this);
    QStringList nodeTypes;
    NodeInfo::getNodeTypes(nodeTypes);
    pTypeComboBox->addItems(nodeTypes);
    addInputItem(mpPortsTable,row,1);
    addInputItem(mpPortsTable,row,2);
    mpPortsTable->setCellWidget(row,3,pTypeComboBox);
    QCheckBox *pRequiredCheckBox = new QCheckBox(this);
    pRequiredCheckBox->setChecked(true);
    mpPortsTable->setCellWidget(row,4,pRequiredCheckBox);
    while(mRemovePortToolButtons.size() < row) {
        mRemovePortToolButtons.push_back(new QToolButton(this));
        mRemovePortToolButtons.last()->setIcon(QIcon(QString(ICONPATH)+"svg/Hopsan-Discard.svg"));
        connect(mRemovePortToolButtons.last(), SIGNAL(clicked(bool)), this, SLOT(removePortRow()));
    }
    mpPortsTable->setCellWidget(row,0,mRemovePortToolButtons.at(row-1));
    adjustTableSize(mpPortsTable);
    mpPortsTable->setVisible(true);
}

void NewComponentDialog::removeConstantRow()
{
    QToolButton *pSender = qobject_cast<QToolButton*>(sender());
    int i = mRemoveConstantToolButtons.indexOf(pSender);
    mpConstantsTable->removeRow(i+1);
    mRemoveConstantToolButtons.remove(i);
    adjustTableSize(mpConstantsTable);
    mpConstantsTable->setVisible(mpConstantsTable->rowCount() > 1);
}

void NewComponentDialog::removeInputVariableRow()
{
    QToolButton *pSender = qobject_cast<QToolButton*>(sender());
    int i = mRemoveInputVariableToolButtons.indexOf(pSender);
    mpInputVariablesTable->removeRow(i+1);
    mRemoveInputVariableToolButtons.remove(i);
    adjustTableSize(mpInputVariablesTable);
    mpInputVariablesTable->setVisible(mpInputVariablesTable->rowCount() > 1);
}

void NewComponentDialog::removeOutputVariableRow()
{
    QToolButton *pSender = qobject_cast<QToolButton*>(sender());
    int i = mRemoveOutputVariableToolButtons.indexOf(pSender);
    mpOutputVariablesTable->removeRow(i+1);
    mRemoveOutputVariableToolButtons.remove(i);
    adjustTableSize(mpOutputVariablesTable);
    mpOutputVariablesTable->setVisible(mpOutputVariablesTable->rowCount() > 1);
}

void NewComponentDialog::removePortRow()
{
    QToolButton *pSender = qobject_cast<QToolButton*>(sender());
    int i = mRemovePortToolButtons.indexOf(pSender);
    mpPortsTable->cellWidget(i+1,3)->deleteLater();   //Delete combo box
    mpPortsTable->cellWidget(i+1,4)->deleteLater();   //Delete check box
    mpPortsTable->removeRow(i+1);
    mRemovePortToolButtons.remove(i);
    adjustTableSize(mpPortsTable);
    mpPortsTable->setVisible(mpPortsTable->rowCount() > 1);

}

void NewComponentDialog::updateIntegrationMethodComboBoxVisibility()
{
    mpIntegrationMethodComboBox->setEnabled(mpLanguageComboBox->currentIndex() == NewComponentLanguage::Modelica);
}

void NewComponentDialog::adjustTableSize(QTableWidget *pTable)
{
    int size = 0;
    for(int r=0; r<pTable->rowCount(); ++r) {
        size += pTable->rowHeight(r);
    }
    pTable->setFixedHeight(size);
}

void LibraryHandler::createNewLibrary() {
    QString libName = QInputDialog::getText(gpMainWindowWidget, "Create New Library", "Select library name:");
    if(!isNameValid(libName)) {
        gpMessageHandler->addErrorMessage("Invalid library name!");
        return;
    }
    if(!libName.isEmpty()) {
        QString libDirPath = QFileDialog::getExistingDirectory(gpMainWindowWidget, "Choose Library Directory", gpConfig->getStringSetting(CFG_EXTERNALLIBDIR));
        QDir libDir(libDirPath);
        if(libDir.entryList(QDir::AllDirs).contains(libName)) {
            QMessageBox existWarningBox(QMessageBox::Warning, "Warning", "Directory already contains a sub-folder with specified type name. Do you want to create new library here anyway?", nullptr, nullptr);
            existWarningBox.addButton("Yes", QMessageBox::AcceptRole);
            existWarningBox.addButton("No", QMessageBox::RejectRole);
            existWarningBox.setWindowIcon(gpMainWindowWidget->windowIcon());
            if(existWarningBox.exec() != QMessageBox::AcceptRole) {
                return;
            }
        }
        if(!libDir.mkdir(libName)) {
            gpMessageHandler->addErrorMessage("Unable to create subdirectory \""+libName+"\" in directory \""+libDir.absolutePath()+"\"");
            return;
        }
        libDir.cd(libName);
        auto pGenerator = createDefaultGenerator(true);
        pGenerator->generateLibrary(libDir.absolutePath(), QStringList());

        loadLibrary(libDir.absoluteFilePath(libName+"_lib.xml"));
    }
}

void LibraryHandler::addComponentToLibrary(SharedComponentLibraryPtrT pLibrary, SaveTargetEnumT newOrExisting, QStringList folders)
{
    auto pGenerator = createDefaultGenerator(true);
    ComponentSpecification spec;
    if(newOrExisting == NewFile) {
        if(mpDialog->exec() == QDialog::Rejected) {
            return;
        }

        spec = mpDialog->getSpecification();

        QString targetPath = QFileInfo(pLibrary->xmlFilePath).absolutePath()+"/"+folders.join("/");
        pGenerator->addComponentToLibrary(pLibrary->xmlFilePath, targetPath, spec.typeName, spec.displayName, spec.cqsType, spec.transform,
                                          spec.constantNames, spec.constantDescriptions, spec.constantUnits, spec.constantInits,
                                          spec.inputNames, spec.inputDescriptions, spec.inputUnits, spec.inputInits,
                                          spec.outputNames, spec.outputDescriptions, spec.outputUnits, spec.outputInits,
                                          spec.portNames, spec.portDescriptions, spec.portTypes, spec.portsRequired, spec.modelica);
    }
    else {  //ExistingFile
        QString libPath = QFileInfo(pLibrary->xmlFilePath).absolutePath();
        QString cafFileName = QFileDialog::getOpenFileName(gpMainWindowWidget, "Add Existing Component", libPath, "XML files (*.xml)");
        QString cafPath = QFileInfo(cafFileName).absolutePath();
        if(!cafPath.startsWith(libPath)) {
            gpMessageHandler->addErrorMessage("Can only add component files inside library folder or a sub-directory");
            return;
        }
        pGenerator->addComponentToLibrary(pLibrary->xmlFilePath, cafFileName);
    }
    gpModelHandler->saveState();
    // First unload the library
    QString libPath = pLibrary->xmlFilePath;
    if (unloadLibrary(pLibrary)) {
        // Now reload the library
        loadLibrary(libPath,ExternalLib,Visible,NoRecompile);
    }
    gpModelHandler->restoreState();

    if(newOrExisting == NewFile) {
        QString path =QFileInfo(pLibrary->getLibraryMainFilePath()).absolutePath()+"/"+folders.join("/");
        ComponentLibraryEntry entry = getEntry(spec.typeName);
        gpModelHandler->loadTextFile(QDir(path).filePath(entry.pAppearance->getSourceCodeFile()));
    }
}


void LibraryHandler::removeComponentFromLibrary(const QString &typeName, SharedComponentLibraryPtrT pLibrary, DeleteOrKeepFilesEnumT deleteOrKeepFiles)
{
    ComponentLibraryEntry entry = getEntry(typeName);

    auto pGenerator = createDefaultGenerator(true);
    QString cafPath = entry.pAppearance->getXMLFile().absoluteFilePath();
    QString hppPath = QDir(entry.pAppearance->getBasePath()).absoluteFilePath(entry.pAppearance->getSourceCodeFile());
    bool deleteFiles = (DeleteFiles == deleteOrKeepFiles);
    pGenerator->removeComponentFromLibrary(pLibrary->xmlFilePath, cafPath, hppPath, deleteFiles);

    gpModelHandler->saveState();
    // First unload the library
    QString libPath = pLibrary->xmlFilePath;
    if (unloadLibrary(pLibrary)) {
        // Now reload the library
        loadLibrary(libPath,ExternalLib,Visible,NoRecompile);
    }
    gpModelHandler->restoreState();
}


//! @brief Unloads library by component type name
//! @param typeName Type name of any component in the library
bool LibraryHandler::unloadLibraryByComponentType(QString typeName)
{
    // Find the library that the component belongs to
    ComponentLibraryEntry selectedEntry = getEntry(typeName);
    if(!selectedEntry.isValid())
    {
        qDebug() << "Component: " << typeName << " not found.";
        return false; //No component found, probably already unloaded
    }
    if(!selectedEntry.pLibrary)
    {
        qDebug() << "Library with component: " << typeName << " not found.";
        return false; //No library found, ignore (should normally never happen)
    }
    qDebug() << "Unloading component: " << typeName << ".";
    return unloadLibrary(selectedEntry.pLibrary);
}

//! @brief Unloads fmu library by fmu name
//! @param fmuName Name of the fmu to unload
bool LibraryHandler::unloadLibraryFMU(QString fmuName)
{
    // Find the library entery that has fmuName (and is an fmu)
    ComponentLibraryEntry fmuEntry = getFMUEntry(fmuName);
    if(!fmuEntry.isValid())
    {
        qDebug() << "fmuEntry: " << fmuName << " not found.";
        return false;
    }
    return unloadLibrary(fmuEntry.pLibrary);
}

bool LibraryHandler::unloadLibrary(SharedComponentLibraryPtrT pLibrary)
{
    if(pLibrary)
    {
        CoreLibraryAccess core;
        QStringList components, nodes;  //Components and nodes to remove

        components.append(pLibrary->guiOnlyComponents);

        //Generate list of all components and nodes in library
        core.getLibraryContents(pLibrary->libFilePath, components, nodes);

        //Unload the library from HopsanCore
        core.unLoadComponentLib(pLibrary->libFilePath);

        //Remove all unloaded components from library
        QMutableMapIterator<QString, ComponentLibraryEntry> it(mLibraryEntries);
        while (it.hasNext()) {
            it.next();
            if (it.value().pLibrary == pLibrary)
                it.remove();
        }
        for(int c=0; c<components.size(); ++c)
        {
            mLibraryEntries.remove(components[c]);
        }

        // Forget library
        gpConfig->removeUserLib(pLibrary->getLibraryMainFilePath());

        //Remove library from list of loaded libraries
        for(int l=0; l<mLoadedLibraries.size(); ++l)
        {

            if (mLoadedLibraries[l]->getLibraryMainFilePath() == pLibrary->getLibraryMainFilePath())
            {
                mLoadedLibraries.removeAt(l);
                --l;
            }
        }

        gpMessageHandler->collectHopsanCoreMessages();
        emit contentsChanged();

        return true;
    }
    return false;
}

bool LibraryHandler::loadLibrary(SharedComponentLibraryPtrT pLibrary, LibraryTypeEnumT type, HiddenVisibleEnumT visibility, RecompileEnumT recompile)
{
    CoreLibraryAccess coreAccess;

    QFileInfo libraryMainFileInfo;
    bool isXmlLib=false;
    // Decide the library root dir and if this is an xml library or if it is a "fall-back" dll library
    if (!pLibrary->xmlFilePath.isEmpty())
    {
        isXmlLib = true;
        libraryMainFileInfo.setFile(pLibrary->xmlFilePath);
    }
    else if (!pLibrary->libFilePath.isEmpty())
    {
        gpMessageHandler->addErrorMessage("Fall-back " SHAREDLIB_SUFFIX " loading is no longer possible, your library must have a library xml file");
        return false;
    }
    QFileInfo libraryDLLFileInfo(pLibrary->libFilePath);
    QDir libraryRootDir = libraryMainFileInfo.absoluteDir();

    // If this lib has already been loaded then skip it
    if ( isLibraryLoaded(libraryMainFileInfo.canonicalFilePath(), libraryDLLFileInfo.canonicalFilePath()) )
    {
        gpMessageHandler->addWarningMessage("Library: "+libraryMainFileInfo.canonicalFilePath()+" is already loaded");
        return true; // We return true anyway, since the library is actually loaded
    }

    // Load the xml library file
    if (isXmlLib)
    {
        QFile file(libraryMainFileInfo.canonicalFilePath());
        if(file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QDomDocument domDocument;
            QString errorStr;
            int errorLine, errorColumn;
            if(domDocument.setContent(&file, false, &errorStr, &errorLine, &errorColumn))
            {
                QDomElement xmlRoot = domDocument.documentElement();
                if(xmlRoot.tagName() == QString(XML_LIBRARY))
                {
                    pLibrary->version = xmlRoot.attribute(XML_VERSION);
                    pLibrary->recompilable = parseAttributeBool(xmlRoot, XML_RECOMPILABLE, true);
                    // Read name of library
                    pLibrary->name = xmlRoot.firstChildElement(XML_LIBRARY_NAME).text();
                    if (pLibrary->name.isEmpty()) {
                        // Try fall-back loading deprecated name attribute
                        if(xmlRoot.hasAttribute(XML_LIBRARY_NAME)) {
                            pLibrary->name = xmlRoot.attribute(XML_LIBRARY_NAME);
                        }
                    }

                    // A name is required so aborting if it is not present
                    if (pLibrary->name.isEmpty()) {
                        gpMessageHandler->addErrorMessage(QString("Library: %1 is missing the <name> element, or name is empty.")
                                                          .arg(libraryMainFileInfo.canonicalFilePath()));
                        return false;
                    }

                    // Read id of library
                    pLibrary->id = xmlRoot.firstChildElement(XML_LIBRARY_ID).text();
                    if (pLibrary->id.isEmpty()) {
                        gpMessageHandler->addWarningMessage(QString("Library: %1 is missing the <id> element, or id is empty. Using name '%2' as fall-back.")
                                                            .arg(libraryMainFileInfo.canonicalFilePath()).arg(pLibrary->name));
                        pLibrary->id = pLibrary->name;
                    }

                    // Abort loading if a library with the samme ID is already loaded
                    auto alreadyLoadedLibrary = getLibrary(pLibrary->id);
                    if (!alreadyLoadedLibrary.isNull()) {
                        gpMessageHandler->addErrorMessage(QString("A library with ID: %1 is already loaded. Its name is '%2'. If the library you are trying to"
                                                                  " load: '%3' is a copy, it must have a new unique id given in its xml file.")
                                                          .arg(alreadyLoadedLibrary->id).arg(alreadyLoadedLibrary->name).arg(pLibrary->name));
                        return false;
                    }

                    // Read library share lib file
                    QDomElement libElement = xmlRoot.firstChildElement(XML_LIBRARY_LIB);
                    if(!libElement.isNull())
                    {
                        pLibrary->debugExtension = libElement.attribute(XML_LIBRARY_LIB_DBGEXT,"");
                        pLibrary->libFilePath = libraryMainFileInfo.canonicalPath()+"/"+QString(LIBPREFIX)+libElement.text();
#ifdef HOPSAN_BUILD_TYPE_DEBUG
                        pLibrary->libFilePath += pLibrary->debugExtension;
#endif
                        pLibrary->libFilePath += QString(LIBEXT);
                    }

                    // Read build flags
                    QDomElement bfElement = xmlRoot.firstChildElement("buildflags").firstChildElement();
                    while (!bfElement.isNull())
                    {
                        if (bfElement.tagName() == "cflags")
                        {
                            pLibrary->cflags.append(" "+bfElement.text());
                        }
                        else if (bfElement.tagName() == "lflags")
                        {
                            pLibrary->lflags.append(" "+bfElement.text());
                        }
                        //! @todo handle other elements such as includepath libpath libflag defineflag and such
                        bfElement = bfElement.nextSiblingElement();
                    }

                    QDomElement includePathElement = xmlRoot.firstChildElement(QString(XML_LIBRARY_INCLUDEPATH));
                    while(!includePathElement.isNull()) {
                        pLibrary->includePaths.append(includePathElement.text());
                        includePathElement = includePathElement.nextSiblingElement(QString(XML_LIBRARY_INCLUDEPATH));
                    }

                    QDomElement linkPathElement = xmlRoot.firstChildElement(QString(XML_LIBRARY_LINKPATH));
                    while(!linkPathElement.isNull()) {
                        pLibrary->linkPaths.append(linkPathElement.text());
                        linkPathElement = linkPathElement.nextSiblingElement(QString(XML_LIBRARY_LINKPATH));
                    }

                    QDomElement linkLibraryElement = xmlRoot.firstChildElement(QString(XML_LIBRARY_LINKLIBRARY));
                    while(!linkLibraryElement.isNull()) {
                        pLibrary->linkLibraries.append(linkLibraryElement.text());
                        linkLibraryElement = linkLibraryElement.nextSiblingElement(QString(XML_LIBRARY_LINKLIBRARY));
                    }

                    // Read source files
                    QDomElement sourceElement = xmlRoot.firstChildElement(QString(XML_LIBRARY_SOURCE));
                    while(!sourceElement.isNull())
                    {
                        pLibrary->sourceFiles.append(QFileInfo(file).canonicalPath()+"/"+sourceElement.text());
                        sourceElement = sourceElement.nextSiblingElement(QString(XML_LIBRARY_SOURCE));
                    }

                    // Read extra source files
                    QDomElement extraSourceElement = xmlRoot.firstChildElement(QString(XML_LIBRARY_EXTRA_SOURCE));
                    while(!extraSourceElement.isNull())
                    {
                        pLibrary->sourceFiles.append(QFileInfo(file).canonicalPath()+"/"+extraSourceElement.text());
                        extraSourceElement = extraSourceElement.nextSiblingElement(QString(XML_LIBRARY_EXTRA_SOURCE));
                    }

                    // Read components
                    if(!libraryFormatVersionLessThen(pLibrary->version.toDouble(), 0.3)) {
                        QDomElement cafElement = xmlRoot.firstChildElement(QString(XML_COMPONENT_XML));
                        while(!cafElement.isNull()) {
                            pLibrary->cafFiles.append(libraryRootDir.absoluteFilePath(cafElement.text()));
                            cafElement = cafElement.nextSiblingElement(QString(XML_COMPONENT_XML));
                        }
                        qDebug() << "CAF files: " << pLibrary->cafFiles;
                    }

                    // Remember library (we do this here even if no DLL/SO files are loaded as we might load internal or "gui only" components
                    mLoadedLibraries.append(pLibrary);

                    // Try to load specified compiled library "plugin" file (if available)
                    if(!pLibrary->libFilePath.isEmpty())
                    {
                        if(!coreAccess.loadComponentLib(pLibrary->libFilePath) && (recompile == Recompile))
                        {
                            // Failed to load
                            gpMessageHandler->collectHopsanCoreMessages();
                            gpMessageHandler->addErrorMessage("Failed to load library: "+pLibrary->libFilePath);
                            // Attempt recompilation if we have a compiler set
                            if (!gpConfig->getGCCPath().isEmpty())
                            {
                                gpMessageHandler->addInfoMessage("Attempting to recompile library: "+pLibrary->name+"...");
                                recompileLibrary(pLibrary,true);
                                gpMessageHandler->collectHopsanCoreMessages();

                                // Try to load again
                                if(!coreAccess.loadComponentLib(pLibrary->libFilePath))
                                {
                                    // Still no success, recompilation failed. Ignore and go on.
                                    gpMessageHandler->addErrorMessage("Failed to load recompiled library!");
                                }
                                else
                                {
                                    // Successful loading after recompilation
                                    gpMessageHandler->addInfoMessage("Success loading recompiled library!");
                                }
                            }
                            else
                            {
                                gpMessageHandler->addWarningMessage("No compiler path set, will not try to recompile the library!");
                            }
                        }
                    }
                }
                else
                {
                    gpMessageHandler->addErrorMessage(QString("The specified XML file does not have Hopsan library root element. Expected: %1, Found: %2, In: %3")
                                                      .arg(XML_LIBRARY).arg(xmlRoot.tagName()).arg(libraryMainFileInfo.canonicalFilePath()));
                    return false;
                }
            }
            else
            {
                gpMessageHandler->addErrorMessage(QString("Could not parse File: %1, Error: %2, Line: %3, Column: %4. Is it a Library XML file?")
                                                  .arg(libraryMainFileInfo.canonicalFilePath()).arg(errorStr).arg(errorLine).arg(errorColumn));
                return false;
            }
        }
        else
        {
            gpMessageHandler->addErrorMessage(QString("Could not open (read) Library XML file: %1").arg(visibleFilePath(libraryMainFileInfo)));
            return false;
        }
        file.close();
    }

    // Determine where to store any backups of updated appearance xml files
    mUpdateXmlBackupDir.setPath(gpDesktopHandler->getBackupPath() + "updateXML_" + QDate::currentDate().toString("yyMMdd")  + "_" + QTime::currentTime().toString("HHmm"));

    // Recurse sub directories and find all xml caf files
    //! @todo why not search allways, and in >=0.3 case warn if we find caf files that were not in the library XML ? In case someone write these xml files by hand
    if(libraryFormatVersionLessThen(pLibrary->version.toDouble(), 0.3)) {
        libraryRootDir.setFilter(QDir::Files | QDir::Dirs | QDir::NoDot | QDir::NoDotDot);
        libraryRootDir.setNameFilters(QStringList() << "*.xml");
        QDirIterator it(libraryRootDir, QDirIterator::Subdirectories);
        while(it.hasNext())
        {
            //Read from the xml file
            QFile cafFile(it.next());
            QFileInfo cafFileInfo(cafFile);

            //Iterate over all xml files in folder and subfolders
            if(cafFile.open(QIODevice::ReadOnly | QIODevice::Text))
            {
                QDomDocument domDocument;
                QString errorStr;
                int errorLine, errorColumn;
                if(domDocument.setContent(&cafFile, false, &errorStr, &errorLine, &errorColumn))
                {
                    QDomElement cafRoot = domDocument.documentElement();
                    if(cafRoot.tagName() == QString(CAF_ROOT))
                    {
                        pLibrary->cafFiles.append(cafFileInfo.absoluteFilePath());
                    }
                }
                else
                {
                    gpMessageHandler->addErrorMessage(QString("When loading component appearance files. Could not parse file: %1, Error: %2, Line: %3, Column: %4. Is it a component XML file?")
                                                      .arg(cafFileInfo.canonicalFilePath()).arg(errorStr).arg(errorLine).arg(errorColumn));
                }
            }
            else
            {
                gpMessageHandler->addErrorMessage(QString("When loading component appearance files. Could not open (read) file: %1").arg(visibleFilePath(cafFileInfo)));
            }
            cafFile.close();
        }
    }

    if (pLibrary->cafFiles.isEmpty()) {
        gpMessageHandler->addWarningMessage(QString("Did not find any component XML files when loading library: %1").arg(pLibrary->getLibraryMainFilePath()));
    }

    for(const QString& cafFileName : pLibrary->cafFiles) {
        QFile cafFile(cafFileName);
        QFileInfo cafFileInfo(cafFileName);

        if(cafFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QDomDocument domDocument;
            QString errorStr;
            int errorLine, errorColumn;
            if(domDocument.setContent(&cafFile, false, &errorStr, &errorLine, &errorColumn)) {
                QDomElement cafRoot = domDocument.documentElement();
                if(cafRoot.tagName() == QString(CAF_ROOT))  {
                    //Read appearance data from the caf xml file, begin with the first
                    QDomElement xmlModelObjectAppearance = cafRoot.firstChildElement(CAF_MODELOBJECT); //! @todo extend this code to be able to read many appearance objects from same file
                    SharedModelObjectAppearanceT pAppearanceData = SharedModelObjectAppearanceT(new ModelObjectAppearance);
                    pAppearanceData->setBasePath(QFileInfo(cafFile).absolutePath()+"/");
                    pAppearanceData->setXMLFile(QFileInfo(cafFile));
                    pAppearanceData->readFromDomElement(xmlModelObjectAppearance);
                    pAppearanceData->cacheIcons();

                    // Check CAF version, and ask user if they want to update to latest version
                    QString caf_version = cafRoot.attribute(CAF_VERSION);

                    if (caf_version < CAF_VERSIONNUM) {
                        bool doSave=false;
                        if (mUpConvertAllCAF==UndecidedToAll) {
                            QMessageBox questionBox(gpMainWindowWidget);
                            QString text;
                            QTextStream ts(&text);
                            ts << cafFile.fileName() << "\n"
                               << "Your file format is older than the newest version! " << caf_version << "<" << CAF_VERSIONNUM << " Do you want to auto update to the latest format?\n\n"
                               << "NOTE! Your old files will be copied to the hopsan Backup folder, but you should make sure that you have a backup in case something goes wrong.\n"
                               << "NOTE! All non-standard Hopsan contents will be lost\n"
                               << "NOTE! Attributes may change order within a tag (but the order is not important)\n\n"
                               << "If you want to update manually, see the documentation about the latest format version.";
                            questionBox.setWindowTitle("New appearance data format available");
                            questionBox.setText(text);
                            QPushButton* pYes = questionBox.addButton(QMessageBox::Yes);
                            questionBox.addButton(QMessageBox::No);
                            QPushButton* pYesToAll = questionBox.addButton(QMessageBox::YesToAll);
                            QPushButton* pNoToAll = questionBox.addButton(QMessageBox::NoToAll);
                            questionBox.setDefaultButton(QMessageBox::No);
                            emit closeSplashScreen();
                            questionBox.exec();
                            QAbstractButton* pClickedButton = questionBox.clickedButton();

                            if ( (pClickedButton == pYes) || (pClickedButton == pYesToAll) ) {
                                doSave = true;
                            }

                            if (pClickedButton == pYesToAll) {
                                mUpConvertAllCAF = YesToAll;
                            }
                            else if (pClickedButton == pNoToAll) {
                                mUpConvertAllCAF = NoToAll;
                            }
                        }
                        else if (mUpConvertAllCAF==YesToAll) {
                            doSave = true;
                        }

                        if (doSave) {
                            // Make backup of original file
                            QFileInfo newBakFile(mUpdateXmlBackupDir.absolutePath());
                            QDir dir;
                            dir.mkpath(newBakFile.absoluteDir().absolutePath());
                            cafFile.copy(newBakFile.absoluteFilePath());

                            // Save (overwrite original file)
                            pAppearanceData->saveToXMLFile(cafFile.fileName());
                        }
                    }

                    // Verify appearance data loaded from caf file
                    bool existsInCore = true;
                    const QString typeName = pAppearanceData->getTypeName();
                    // Do not check in case it is a Subsystem or SystemPort
                    if( !((typeName==HOPSANGUISYSTEMTYPENAME) || (typeName==HOPSANGUICONDITIONALSYSTEMTYPENAME) || (typeName==HOPSANGUISYSTEMPORTTYPENAME)) ) {
                        //! @todo maybe they should be reserved in hopsan core instead, then we could aske the core if the exist
                        // Check so that there is such a component available in the Core, or if the component points to an external model file
                        existsInCore = coreAccess.hasComponent(typeName) || !pAppearanceData->getHmfFile().isEmpty();
                        if(!existsInCore) {
                            gpMessageHandler->addWarningMessage("Failed to load component of type: "+pAppearanceData->getFullTypeName(), "failedtoloadcomp");
                        }
                    }

                    auto pLibraryBeingLoaded = mLoadedLibraries.last();

                    ComponentLibraryEntry newEntry;
                    if(!existsInCore) {
                        newEntry.state = Disabled;
                    }
                    newEntry.pLibrary = pLibraryBeingLoaded;

                    // Store appearance data
                    newEntry.pAppearance = pAppearanceData;
                    QString subTypeName = pAppearanceData->getSubTypeName();
                    QString fullTypeName = makeFullTypeString(newEntry.pAppearance->getTypeName(), subTypeName);

                    if (!subTypeName.isEmpty()) {
                        // Find what library this component depend on (main component type) and make sure that library knows bout this dependency
                        QString libPath;
                        coreAccess.getLibPathForComponent(pAppearanceData->getTypeName(), libPath);
                        if( !libPath.isEmpty() ) {
                            for(SharedComponentLibraryPtrT pLib : mLoadedLibraries) {
                                if( pLib->libFilePath == libPath )
                                {
                                    pLib->guiOnlyComponents.append(fullTypeName);
                                    break;
                                }
                            }
                        }

                        // Make this library remember this entry as a gui only component
                        pLibraryBeingLoaded->guiOnlyComponents.append(fullTypeName);
                    }

                    // Calculate path to show in library
                    QString relDir = QDir(libraryMainFileInfo.canonicalPath()).relativeFilePath(cafFileInfo.canonicalFilePath());
                    newEntry.displayPath = relDir.split("/");
                    newEntry.displayPath.removeLast();
                    QString libName = newEntry.pLibrary->name;
                    if (libName.isEmpty()) {
                        libName = libraryRootDir.dirName();
                    }
                    if(type == ExternalLib) {
                        newEntry.displayPath.prepend(libName);
                        newEntry.displayPath.prepend(componentlibrary::roots::externalLibraries);
                    }
                    else if(type == FmuLib) {
                        newEntry.displayPath.prepend(libName);
                        newEntry.displayPath.prepend(componentlibrary::roots::fmus);
                    }

                    // Store visibility
                    newEntry.visibility = visibility;

                    // Store new entry, but only if it does not already exist
                    if(!mLibraryEntries.contains(fullTypeName)) {
                        mLibraryEntries.insert(fullTypeName, newEntry);
                        emit showSplashScreenMessage("Loaded component: " + pAppearanceData->getTypeName());
                    }
                    else {
                        const auto& existingEntery = mLibraryEntries[fullTypeName];
                        gpMessageHandler->addWarningMessage(QString("A component with type name '%1' was already registered by library '%2'. Ignoring version in library '%3'.")
                                                            .arg(fullTypeName)
                                                            .arg(existingEntery.pLibrary ? existingEntery.pLibrary->name : "Unknown")
                                                            .arg(newEntry.pLibrary ? newEntry.pLibrary->name : "Unknown"));
                    }

                }
            }
            else
            {
                gpMessageHandler->addErrorMessage(QString("When loading component appearance files. Could not parse file: %1, Error: %2, Line: %3, Column: %4. Is it a component XML file?")
                                                  .arg(cafFileInfo.canonicalFilePath()).arg(errorStr).arg(errorLine).arg(errorColumn));
            }
        }
        else
        {
            gpMessageHandler->addErrorMessage(QString("When loading component appearance files. Could not open (read) file: %1").arg(visibleFilePath(cafFileInfo)));
        }
        cafFile.close();
    }
    gpMessageHandler->collectHopsanCoreMessages();


    if(type != InternalLib)
    {
        gpConfig->addUserLib(pLibrary->getLibraryMainFilePath(), type);
    }
    emit contentsChanged();
    return true;
}

void NewComponentDialog::addLabelItem(QTableWidget *pTable, int r, int c, QString text)
{
    pTable->setItem(r,c,new QTableWidgetItem(text));
    QFont labelFont = pTable->item(r,c)->font();
    labelFont.setBold(true);
    pTable->item(r,c)->setFlags(pTable->item(r,c)->flags() ^ Qt::ItemIsSelectable ^ Qt::ItemIsEditable);
    pTable->item(r,c)->setBackgroundColor(QColor(161,224,228));
}

void NewComponentDialog::addInputItem(QTableWidget *pTable, int r, int c, QString defaultValue)
{
    pTable->setItem(r,c,new QTableWidgetItem(defaultValue));
}

bool LibraryHandler::isTypeNamesOkToUnload(const QStringList &typeNames)
{
    QStringList models;
    for(int m=0; m<gpModelHandler->count(); ++m)
    {
        bool hasUnloadingComponent=false;
        SystemObject *pSystem = gpModelHandler->getTopLevelSystem(m);
        for(const QString &comp : pSystem->getModelObjectNames()) {
            if(typeNames.contains(pSystem->getModelObject(comp)->getTypeName()))
            {
                hasUnloadingComponent = true;
            }
        }
        if(hasUnloadingComponent)
        {
            models.append(pSystem->getName());
        }
    }

    if(!models.isEmpty())
    {
        QString msg = "The following models are using components from the library:\n\n";
        for(const QString &model : models) {
            msg.append(model+"\n");
        }
        msg.append("\nThey must be closed before unloading.");
        QMessageBox::critical(gpMainWindowWidget, "Unload failed", msg);
        return false;
    }

    return true;
}


//! @brief Returns a list of all loaded component type names
QStringList LibraryHandler::getLoadedTypeNames() const
{
    return mLibraryEntries.keys();
}


//! @brief Returns a component entry in the library
//! @param typeName Type name of component
//! @param subTypeName of component (optional)
ComponentLibraryEntry LibraryHandler::getEntry(const QString &typeName, const QString &subTypeName) const
{
    QString fullTypeString = makeFullTypeString(typeName, subTypeName);
    return mLibraryEntries.value(fullTypeString, ComponentLibraryEntry() );
}

//! @brief Returns an FMU component entry in the library
//! @param rFmuName The FMU name
//! @returns The library entery for given fmu name or an invalid library entery if fmu name not found
ComponentLibraryEntry LibraryHandler::getFMUEntry(const QString &rFmuName) const
{
    //! @todo I dont think we can have multiple component in the same FMU so this should be safe (for now)
    //QString fullTypeString = makeFullTypeString(typeName, subTypeName);
    for (const ComponentLibraryEntry &le : mLibraryEntries.values())
    {
        if (le.pLibrary && le.pLibrary->type == FmuLib)
        {
            // Here it is assumed that the load code prepends FMULIBSTR before the actual path
            if (le.displayPath.last() == rFmuName)
            {
                return le;
            }
        }
    }
    return ComponentLibraryEntry();
}


const SharedModelObjectAppearanceT LibraryHandler::getModelObjectAppearancePtr(const QString &typeName, const QString &subTypeName) const
{
    auto it = mLibraryEntries.find((makeFullTypeString(typeName, subTypeName)));
    if(it != mLibraryEntries.end())
    {
        return it.value().pAppearance;
    }
    else
    {
        return SharedModelObjectAppearanceT();
    }
}

void LibraryHandler::addReplacement(QString type1, QString type2)
{
    //qDebug() << "Adding replacement: " << type1 << ", " << type2;

    if(mReplacementsMap.contains(type1))
    {
        if(!mReplacementsMap.find(type1).value().contains(type2))
        {
            mReplacementsMap.find(type1).value().append(type2);
        }
    }
    else
    {
        mReplacementsMap.insert(type1, QStringList() << type2);
    }

    if(mReplacementsMap.contains(type2))
    {
        if(!mReplacementsMap.find(type2).value().contains(type1))
        {
            mReplacementsMap.find(type2).value().append(type1);
        }
    }
    else
    {
        mReplacementsMap.insert(type2, QStringList() << type1);
    }
}


//! @brief Returns a list of all replacements for specified component
//! @param type Type name to look for
QStringList LibraryHandler::getReplacements(QString type)
{
    if(mReplacementsMap.contains(type))
    {
        return mReplacementsMap.find(type).value();
    }
    return QStringList();
}


//! @brief Imports a functional mock-up unit from a file dialog
void LibraryHandler::importFmu()
{
    //Load .fmu file and create paths
    QString filePath = QFileDialog::getOpenFileName(gpMainWindowWidget, tr("Import Functional Mockup Unit (FMU)"),
                                                    gpConfig->getStringSetting(CFG_FMUIMPORTDIR),
                                                    tr("Functional Mockup Unit (*.fmu)"));
    if(filePath.isEmpty())      //Cancelled by user
        return;

    QFileInfo fmuFileInfo = QFileInfo(filePath);
    if(!fmuFileInfo.exists())
    {
        gpMessageHandler->addErrorMessage("File not found: "+filePath);
        return;
    }
    gpConfig->setStringSetting(CFG_FMUIMPORTDIR, fmuFileInfo.absolutePath());

    SystemObject *pSystem = gpModelHandler->getCurrentTopLevelSystem();
    if(pSystem) {
        QPointF pos = pSystem->getGraphicsViewport().mCenter;
        ModelObject *pFmuComponent = pSystem->addModelObject("FMIWrapper", pos);
        if(pFmuComponent) {
            pFmuComponent->setParameterValue("path", fmuFileInfo.absoluteFilePath());
        }
    }
}


//! @brief Recompiles specified component library (safe to use with opened models)
//! @param lib Component library to recompile
//! @param solver Solver to use (for Modelica code only)
void LibraryHandler::recompileLibrary(SharedComponentLibraryPtrT pLib, bool dontUnloadAndLoad)
{
    if(!pLib->recompilable) {
        gpMessageHandler->addErrorMessage("Library is not recompilable.");
        return;
    }

    CoreLibraryAccess coreLibrary;
    auto spGenerator = createDefaultImportGenerator();

    if(!dontUnloadAndLoad)
    {
        //Save GUI state
        gpModelHandler->saveState();

        //Unload library from core
        coreLibrary.unLoadComponentLib(pLib->libFilePath);
    }

    //Generate C++ code from Modelica if source files are Modelica code
    bool modelicaFailed=false;
    for(const QString &caf : pLib->cafFiles)
    {
        QFile cafFile(caf);
        cafFile.open(QFile::ReadOnly);
        QString code = cafFile.readAll();
        cafFile.close();
        QString path;
        QString sourceFile = code.section("sourcecode=\"",1,1).section("\"",0,0);
        path = QFileInfo(cafFile).path();
        qDebug() << "PATH: " << path;
        if(sourceFile.endsWith(".mo"))
        {
            qDebug() << "GENERATING: " << path+"/"+sourceFile;
            if (!spGenerator->generateFromModelica(path+"/"+sourceFile, HopsanGeneratorGUI::CompileT::DoNotCompile))
            {
                modelicaFailed = true;
                gpMessageHandler->addErrorMessage("Failed to translate Modelica to C++");
            }
        }
    }

    // Add extra compiler flags for FMU libraries (to make sure we compile with the correct fmi library version shiped with Hopsan)
    // Note we only add the paths here, in case the paths in the FMU wrapper library xml are incorrect (old export)
    QString extraCFlags, extraLFlags;
    if(pLib->type == FmuLib)
    {
        const QString fmiLibDir="/dependencies/fmilibrary";
        extraCFlags = QString("-I\"%1\"").arg(gpDesktopHandler->getMainPath()+fmiLibDir+"/include");
        extraLFlags = QString("-L\"%1\" -l").arg(gpDesktopHandler->getMainPath()+fmiLibDir+"/lib");
#ifdef _WIN32
    extraLFlags += "libfmilib_shared";
#else
    extraLFlags += "fmilib_shared";  //Remove extra "lib" prefix in Linux
#endif
    }

    //Call compile utility
    QString libfile = QFileInfo(pLib->xmlFilePath).absoluteFilePath();
    if(!modelicaFailed) {
        if (!spGenerator->compileComponentLibrary(libfile, extraCFlags, extraLFlags))
        {
            gpMessageHandler->addErrorMessage(QString("Failed to compile component library: %1").arg(libfile));
        }
    }

    if(!dontUnloadAndLoad)
    {
        //Load library again
        coreLibrary.loadComponentLib(pLib->libFilePath);

        //Restore GUI state
        gpModelHandler->restoreState();
    }
}

bool ComponentLibraryEntry::isValid() const
{
    return (pLibrary!=nullptr) && (pAppearance!=nullptr) && (!displayPath.isEmpty()) ;
}


QString GUIComponentLibrary::getLibraryMainFilePath() const
{
    if (xmlFilePath.isEmpty())
    {
        if (!libFilePath.isEmpty())
        {
            return libFilePath;
        }
    }
    else
    {
        return xmlFilePath;
    }
    return "";
}
