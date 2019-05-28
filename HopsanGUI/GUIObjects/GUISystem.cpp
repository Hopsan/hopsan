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
//! @file   GUISystem.cpp
//! @author Flumes <flumes@lists.iei.liu.se>
//! @date   2010-01-01
//!
//! @brief Contains the GUI System class, representing system components
//!
//$Id$

#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>

#include "global.h"
#include "GUISystem.h"
#include "GraphicsView.h"
#include "CoreAccess.h"
#include "loadFunctions.h"
#include "GUIConnector.h"
#include "UndoStack.h"
#include "version_gui.h"
#include "LibraryHandler.h"
#include "MessageHandler.h"
#include "Widgets/ModelWidget.h"
//#include "Dialogs/ContainerPropertiesDialog.h"
#include "Utilities/GUIUtilities.h"
#include "GUIObjects/GUIWidgets.h"
#include "Widgets/PyDockWidget.h"
#include "Configuration.h"
#include "GUIContainerObject.h"
#include "GUIComponent.h"
#include "GUIPort.h"
#include "Dialogs/ComponentPropertiesDialog3.h"
#include "DesktopHandler.h"
#include "GeneratorUtils.h"


SystemContainer::SystemContainer(QPointF position, double rotation, const ModelObjectAppearance* pAppearanceData, ContainerObject *pParentContainer, SelectionStatusEnumT startSelected, GraphicsTypeEnumT gfxType)
    : ContainerObject(position, rotation, pAppearanceData, startSelected, gfxType, pParentContainer, pParentContainer)
{
    this->mpModelWidget = pParentContainer->mpModelWidget;
    this->commonConstructorCode();
}

//Root system specific constructor
SystemContainer::SystemContainer(ModelWidget *parentModelWidget, QGraphicsItem *pParent)
    : ContainerObject(QPointF(0,0), 0, 0, Deselected, UserGraphics, 0, pParent)
{
    this->mModelObjectAppearance = *(gpLibraryHandler->getModelObjectAppearancePtr(HOPSANGUISYSTEMTYPENAME)); //This will crash if Subsystem not already loaded
    this->mpModelWidget = parentModelWidget;
    this->commonConstructorCode();
    this->mpUndoStack->newPost();
}

void SystemContainer::deleteInHopsanCore()
{
    this->setUndoEnabled(false, true); //The last true means DONT ASK
    //qDebug() << ",,,,,,,,,,,,,,,,,,,,,,,,,GUISystem destructor";
    //First remove all contents
    this->clearContents();

    if (mpParentContainerObject != 0)
    {
        mpParentContainerObject->getCoreSystemAccessPtr()->removeSubComponent(this->getName(), true);
    }
    else
    {
        mpCoreSystemAccess->deleteRootSystemPtr();
    }

    delete mpCoreSystemAccess;
}

//! @brief This code is common among the two constructors, we use one function to avoid code duplication
void SystemContainer::commonConstructorCode()
{
    // Set default values
    mLoadType = "EMBEDED";
    mNumberOfLogSamples = 2048;
    mLogStartTime = 0;
    mSaveUndoStack = false;       //Do not save undo stack by default

    // Connect propagation signal when alias is changed
    connect(this, SIGNAL(aliasChanged(QString,QString)), mpModelWidget, SIGNAL(aliasChanged(QString,QString)));

    // Create the object in core, and update name
    if (this->mpParentContainerObject == 0)
    {
        //Create root system
        qDebug() << "creating ROOT access system";
        mpCoreSystemAccess = new CoreSystemAccess();
        this->setName("RootSystem");
        //qDebug() << "the core root system name: " << mpCoreSystemAccess->getRootSystemName();
    }
    else
    {
        //Create subsystem
        qDebug() << "creating subsystem and setting name in " << mpParentContainerObject->getCoreSystemAccessPtr()->getSystemName();
        if(this->getTypeName() == HOPSANGUICONDITIONALSYSTEMTYPENAME)
        {
            mName = mpParentContainerObject->getCoreSystemAccessPtr()->createConditionalSubSystem(this->getName());
        }
        else
        {
            mName = mpParentContainerObject->getCoreSystemAccessPtr()->createSubSystem(this->getName());
        }
        refreshDisplayName();
        qDebug() << "creating CoreSystemAccess for this subsystem, name: " << this->getName() << " parentname: " << mpParentContainerObject->getName();
        mpCoreSystemAccess = new CoreSystemAccess(this->getName(), mpParentContainerObject->getCoreSystemAccessPtr());
    }

    if(!isTopLevelContainer())
    {
        refreshAppearance();
        refreshExternalPortsAppearanceAndPosition();
        refreshDisplayName(); //Make sure name window is correct size for center positioning
    }

    if(mpParentContainerObject)
    {
        connect(mpParentContainerObject, SIGNAL(showOrHideSignals(bool)), this, SLOT(setVisibleIfSignal(bool)));
    }
}


//! @brief This function sets the desired subsystem name
//! @param [in] newName The new name
void SystemContainer::setName(QString newName)
{
    if (mpParentContainerObject == 0)
    {
        mName = mpCoreSystemAccess->setSystemName(newName);
    }
    else
    {
        mpParentContainerObject->renameModelObject(this->getName(), newName);
    }
    refreshDisplayName();
}


//! Returns a string with the sub system type.
QString SystemContainer::getTypeName() const
{
     return mModelObjectAppearance.getTypeName();
}

//! @brief Get the system cqs type
//! @returns A string containing the CQS type
QString SystemContainer::getTypeCQS()
{
    return mpCoreSystemAccess->getSystemTypeCQS();
}

//! @brief get The parameter names of this system
//! @returns A QStringList containing the parameter names
QStringList SystemContainer::getParameterNames()
{
    return mpCoreSystemAccess->getSystemParameterNames();
}

//! @brief Get a vector contain data from all parameters
//! @param [out] rParameterDataVec A vector that will contain parameter data
void SystemContainer::getParameters(QVector<CoreParameterData> &rParameterDataVec)
{
    mpCoreSystemAccess->getSystemParameters(rParameterDataVec);
}

//! @brief Function that returns the specified parameter value
//! @param name Name of the parameter to return value from
QString SystemContainer::getParameterValue(const QString paramName)
{
    return mpCoreSystemAccess->getSystemParameterValue(paramName);
}

bool SystemContainer::hasParameter(const QString &rParamName)
{
    return mpCoreSystemAccess->hasSystemParameter(rParamName);
}

//! @brief Get parameter data for a specific parameter
//! @param [out] rData The parameter data
void SystemContainer::getParameter(const QString paramName, CoreParameterData &rData)
{
    return mpCoreSystemAccess->getSystemParameter(paramName, rData);
}

//! @brief Get a pointer the the CoreSystemAccess object that this system is representing
CoreSystemAccess* SystemContainer::getCoreSystemAccessPtr()
{
    return mpCoreSystemAccess;
}

//! @brief Overloaded version that returns self if root system
ContainerObject *SystemContainer::getParentContainerObject()
{
    if (mpParentContainerObject==0)
    {
        return this;
    }
    else
    {
        return mpParentContainerObject;
    }
}


int SystemContainer::type() const
{
    return Type;
}

QString SystemContainer::getHmfTagName() const
{
    return HMF_SYSTEMTAG;
}


//! @brief Saves the System specific core data to XML DOM Element
//! @param[in] rDomElement The DOM Element to save to
void SystemContainer::saveCoreDataToDomElement(QDomElement &rDomElement, SaveContentsEnumT contents)
{
    ModelObject::saveCoreDataToDomElement(rDomElement);

    if (mLoadType == "EXTERNAL" && contents == FullModel)
    {
        // Determine the relative path
        QFileInfo parentModelPath(mpParentContainerObject->getModelFilePath());
        QString relPath = relativePath(getModelFilePath(), parentModelPath.absolutePath());

        // This information should ONLY be used to indicate that a subsystem is external, it SHOULD NOT be included in the actual external system
        // If it would be, the load function will fail
        rDomElement.setAttribute( HMF_EXTERNALPATHTAG, relPath );
    }

    if (mLoadType != "EXTERNAL" && contents == FullModel)
    {
        appendSimulationTimeTag(rDomElement, mpModelWidget->getStartTime().toDouble(), this->getTimeStep(), mpModelWidget->getStopTime().toDouble(), this->doesInheritTimeStep());
        appendLogSettingsTag(rDomElement, getLogStartTime(), getNumberOfLogSamples());
    }

    // Save the NumHop script
    if (!mNumHopScript.isEmpty())
    {
        appendDomTextNode(rDomElement, HMF_NUMHOPSCRIPT, mNumHopScript);
    }

    // Save the parameter values for the system
    QVector<CoreParameterData> paramDataVector;
    this->getParameters(paramDataVector);
    QDomElement xmlParameters = appendDomElement(rDomElement, HMF_PARAMETERS);
    for(int i=0; i<paramDataVector.size(); ++i)
    {
        QDomElement xmlParameter = appendDomElement(xmlParameters, HMF_PARAMETERTAG);
        xmlParameter.setAttribute(HMF_NAMETAG, paramDataVector[i].mName);
        xmlParameter.setAttribute(HMF_VALUETAG, paramDataVector[i].mValue);
        xmlParameter.setAttribute(HMF_TYPE, paramDataVector[i].mType);
        if (!paramDataVector[i].mQuantity.isEmpty())
        {
            xmlParameter.setAttribute(HMF_QUANTITY, paramDataVector[i].mQuantity);
        }
        if (!paramDataVector[i].mUnit.isEmpty())
        {
            xmlParameter.setAttribute(HMF_UNIT, paramDataVector[i].mUnit);
        }
        if (!paramDataVector[i].mDescription.isEmpty())
        {
            xmlParameter.setAttribute(HMF_DESCRIPTIONTAG, paramDataVector[i].mDescription);
        }
    }

    // Save the alias names in this system
    QDomElement xmlAliases = appendDomElement(rDomElement, HMF_ALIASES);
    QStringList aliases = getAliasNames();
    //! @todo need one function that gets both alias and full maybe
    for (int i=0; i<aliases.size(); ++i)
    {
        QDomElement alias = appendDomElement(xmlAliases, HMF_ALIAS);
        alias.setAttribute(HMF_TYPE, "variable"); //!< @todo not manual type
        alias.setAttribute(HMF_NAMETAG, aliases[i]);
        QString fullName = getFullNameFromAlias(aliases[i]);
        appendDomTextNode(alias, "fullname",fullName );
    }
}

//! @brief Defines the right click menu for container objects.
void SystemContainer::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    // This will prevent context menus from appearing automatically - they are started manually from mouse release event.
    if(event->reason() == QGraphicsSceneContextMenuEvent::Mouse)
        return;

    bool allowFullEditing = (!isLocallyLocked() && (getModelLockLevel() == NotLocked));
    //bool allowLimitedEditing = (!isLocallyLocked() && (getModelLockLevel() <= LimitedLock));

    QMenu menu;
    QAction *loadAction = menu.addAction(tr("Load Subsystem File"));
    QAction *saveAction = menu.addAction(tr("Save Subsystem As"));
    QAction *saveAsComponentAction = menu.addAction(tr("Save As Component"));
    QAction *enterAction = menu.addAction(tr("Enter Subsystem"));
    loadAction->setEnabled(allowFullEditing);
    if(!mModelFileInfo.filePath().isEmpty())
    {
        loadAction->setDisabled(true);
    }
    if(isExternal())
    {
        saveAction->setDisabled(true);
        saveAsComponentAction->setDisabled(true);
    }

    //qDebug() << "ContainerObject::contextMenuEvent";
    QAction *pAction = this->buildBaseContextMenu(menu, event);
    if (pAction == loadAction)
    {
        QString modelFilePath = QFileDialog::getOpenFileName(gpMainWindowWidget, tr("Choose Subsystem File"),
                                                             gpConfig->getStringSetting(CFG_SUBSYSTEMDIR),
                                                             tr("Hopsan Model Files (*.hmf)"));
        if (!modelFilePath.isNull())
        {
            QFile file;
            file.setFileName(modelFilePath);
            QFileInfo fileInfo(file);
            gpConfig->setStringSetting(CFG_SUBSYSTEMDIR, fileInfo.absolutePath());

            bool doIt = true;
            if (mModelObjectMap.size() > 0)
            {
                QMessageBox clearAndLoadQuestionBox(QMessageBox::Warning, tr("Warning"),tr("All current contents of the system will be replaced. Do you want to continue?"), 0, 0);
                clearAndLoadQuestionBox.addButton(tr("&Yes"), QMessageBox::AcceptRole);
                clearAndLoadQuestionBox.addButton(tr("&No"), QMessageBox::RejectRole);
                clearAndLoadQuestionBox.setWindowIcon(gpMainWindowWidget->windowIcon());
                doIt = (clearAndLoadQuestionBox.exec() == QMessageBox::AcceptRole);
            }

            if (doIt)
            {
                this->clearContents();

                QDomDocument domDocument;
                QDomElement hmfRoot = loadXMLDomDocument(file, domDocument, HMF_ROOTTAG);
                if (!hmfRoot.isNull())
                {
                    //! @todo Check version numbers
                    //! @todo check if we could load else give error message and don't attempt to load
                    QDomElement systemElement = hmfRoot.firstChildElement(HMF_SYSTEMTAG);
                    this->setModelFileInfo(file); //Remember info about the file from which the data was loaded
                    QFileInfo fileInfo(file);
                    this->setAppearanceDataBasePath(fileInfo.absolutePath());
                    this->loadFromDomElement(systemElement);
                }
            }
        }
    }
    else if(pAction == saveAction)
    {
        //Get file name
        QString modelFilePath;
        modelFilePath = QFileDialog::getSaveFileName(gpMainWindowWidget, tr("Save Subsystem As"),
                                                     gpConfig->getStringSetting(CFG_LOADMODELDIR),
                                                     tr("Hopsan Model Files (*.hmf)"));

        if(modelFilePath.isEmpty())     //Don't save anything if user presses cancel
        {
            return;
        }


        //! @todo Duplicated code, but we cannot use code from ModelWidget, because it can only save top level system...
        QFile file(modelFilePath);   //Create a QFile object
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))  //open file
        {
            gpMessageHandler->addErrorMessage("Could not open the file: "+file.fileName()+" for writing." );
            return;
        }

        //Save xml document
        QDomDocument domDocument;
        QDomElement rootElement;
        rootElement = appendHMFRootElement(domDocument, HMF_VERSIONNUM, HOPSANGUIVERSION, getHopsanCoreVersion());

        // Save the required external library names
        QStringList requiredLibraries = this->getRequiredComponentLibraries();
        //! @todo need HMF defines for hardcoded strings
        QDomElement reqDom = appendDomElement(rootElement, "requirements");
        for (const auto& libID : requiredLibraries)
        {
            auto pLibrary = gpLibraryHandler->getLibrary(libID);
            if (pLibrary) {
                auto libdom = appendDomElement(reqDom, "componentlibrary");
                appendDomTextNode(libdom, "id", libID);
                appendDomTextNode(libdom, "name", pLibrary->name);
            }
        }

        //Save the model component hierarchy
        this->saveToDomElement(rootElement, FullModel);

        //Save to file
        QFile xmlFile(modelFilePath);
        if (!xmlFile.open(QIODevice::WriteOnly | QIODevice::Text))  //open file
        {
            gpMessageHandler->addErrorMessage("Could not save to file: " + modelFilePath);
            return;
        }
        QTextStream out(&xmlFile);
        appendRootXMLProcessingInstruction(domDocument); //The xml "comment" on the first line
        domDocument.save(out, XMLINDENTATION);

        //Close the file
        xmlFile.close();

       // mpModelWidget->saveTo(modelFilePath, FullModel);
    }
    else if(pAction == saveAsComponentAction)
    {
        //Get file name
        QString cafFilePath;
        cafFilePath = QFileDialog::getSaveFileName(gpMainWindowWidget, tr("Save Subsystem As"),
                                                   gpConfig->getStringSetting(CFG_LOADMODELDIR),
                                                   tr("Hopsan Component Appearance Files (*.xml)"));

        if(cafFilePath.isEmpty())     //Don't save anything if user presses cancel
        {
            return;
        }

        QString iconFileName = QFileInfo(getIconPath(UserGraphics, Absolute)).fileName();
        QString modelFileName = QFileInfo(cafFilePath).baseName()+".hmf";

        //! @todo why is graphics copied twice
        QFile::copy(getIconPath(UserGraphics, Absolute), QFileInfo(cafFilePath).path()+"/"+iconFileName);
        QFile::copy(getIconPath(UserGraphics, Absolute), getAppearanceData()->getBasePath()+"/"+iconFileName);

        bool ok;
        QString subtype = QInputDialog::getText(gpMainWindowWidget, tr("Decide a unique Subtype"),
                                                tr("Decide a unique subtype name for this component:"), QLineEdit::Normal,
                                                QString(""), &ok);
        if (!ok || subtype.isEmpty())
        {
            gpMessageHandler->addErrorMessage("You must specify a subtype name. Aborting!");
            return;
        }

        //! @todo it would be better if this xml would only include hmffile attribute and all otehr info loaded from there
        QString cafStr = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
        cafStr.append(QString("<hopsanobjectappearance version=\"0.3\">\n"));
        cafStr.append(QString("    <modelobject hmffile=\"%1\" displayname=\"%2\" typename=\"%3\" subtypename=\"%4\">\n").arg(modelFileName).arg(getName()).arg("Subsystem").arg(subtype));
        cafStr.append(QString("        <icons>\n"));
        cafStr.append(QString("            <icon scale=\"1\" path=\"%1\" iconrotation=\"ON\" type=\"user\"/>\n").arg(iconFileName));
        cafStr.append(QString("        </icons>\n"));
        cafStr.append(QString("    </modelobject>\n"));
        cafStr.append(QString("</hopsanobjectappearance>\n"));

        QFile cafFile(cafFilePath);
        if(!cafFile.open(QFile::Text | QFile::WriteOnly))
        {
            gpMessageHandler->addErrorMessage("Could not open the file: "+cafFile.fileName()+" for writing.");
            return;
        }
        cafFile.write(cafStr.toUtf8());
        cafFile.close();

        QString modelFilePath = QFileInfo(cafFilePath).path()+"/"+QFileInfo(cafFilePath).baseName()+".hmf";

        QString orgIconPath = this->getIconPath(UserGraphics, Relative);
        this->setIconPath(iconFileName, UserGraphics, Relative);

        //! @todo Duplicated code, but we cannot use code from ModelWidget, because it can only save top level system...
        QFile file(modelFilePath);   //Create a QFile object
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))  //open file
        {
            gpMessageHandler->addErrorMessage("Could not open the file: "+file.fileName()+" for writing." );
            return;
        }

        //Save xml document
        QDomDocument domDocument;
        QDomElement rootElement;
        rootElement = appendHMFRootElement(domDocument, HMF_VERSIONNUM, HOPSANGUIVERSION, getHopsanCoreVersion());

        // Save the required external library names
        QStringList requiredLibraries = this->getRequiredComponentLibraries();
        //! @todo need HMF defines for hardcoded strings
        QDomElement reqDom = appendDomElement(rootElement, "requirements");
        for (const auto& libID : requiredLibraries)
        {
            auto pLibrary = gpLibraryHandler->getLibrary(libID);
            if (pLibrary) {
                auto libdom = appendDomElement(reqDom, "componentlibrary");
                appendDomTextNode(libdom, "id", libID);
                appendDomTextNode(libdom, "name", pLibrary->name);
            }
        }

        //Save the model component hierarchy
        QString old_subtype = this->getAppearanceData()->getSubTypeName();
        this->getAppearanceData()->setSubTypeName(subtype);
        this->saveToDomElement(rootElement, FullModel);
        this->getAppearanceData()->setSubTypeName(old_subtype);

        //Save to file
        QFile xmlFile(modelFilePath);
        if (!xmlFile.open(QIODevice::WriteOnly | QIODevice::Text))  //open file
        {
            gpMessageHandler->addErrorMessage("Could not save to file: " + modelFilePath);
            return;
        }
        QTextStream out(&xmlFile);
        appendRootXMLProcessingInstruction(domDocument); //The xml "comment" on the first line
        domDocument.save(out, XMLINDENTATION);

        //Close the file
        xmlFile.close();

        this->setIconPath(orgIconPath, UserGraphics, Relative);

        QFile::remove(getModelFilePath()+"/"+iconFileName);
    }
    else if (pAction == enterAction)
    {
        enterContainer();
    }

    // Don't call GUIModelObject::contextMenuEvent as that will open an other menu after this one is closed
}


//! @brief Defines the double click event for container objects (used to enter containers).
void SystemContainer::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsWidget::mouseDoubleClickEvent(event);
    if (isExternal())
    {
        openPropertiesDialog();
    }
    else
    {
        enterContainer();
    }
}

void SystemContainer::saveSensitivityAnalysisSettingsToDomElement(QDomElement &rDomElement)
{
    QDomElement XMLsens = appendDomElement(rDomElement, HMF_SENSITIVITYANALYSIS);
    QDomElement XMLsetting = appendDomElement(XMLsens, HMF_SETTINGS);
    appendDomIntegerNode(XMLsetting, HMF_ITERATIONS, mSensSettings.nIter);
    if(mSensSettings.distribution == SensitivityAnalysisSettings::UniformDistribution)
    {
        appendDomTextNode(XMLsetting, HMF_DISTRIBUTIONTYPE, HMF_UNIFORMDIST);
    }
    else if(mSensSettings.distribution == SensitivityAnalysisSettings::NormalDistribution)
    {
        appendDomTextNode(XMLsetting, HMF_DISTRIBUTIONTYPE, HMF_NORMALDIST);
    }

    //Parameters
    QDomElement XMLparameters = appendDomElement(XMLsens, HMF_PARAMETERS);
    for(int i = 0; i < mSensSettings.parameters.size(); ++i)
    {
        QDomElement XMLparameter = appendDomElement(XMLparameters, HMF_PARAMETERTAG);
        appendDomTextNode(XMLparameter, HMF_COMPONENTTAG, mSensSettings.parameters.at(i).compName);
        appendDomTextNode(XMLparameter, HMF_PARAMETERTAG, mSensSettings.parameters.at(i).parName);
        appendDomValueNode2(XMLparameter, HMF_MINMAX, mSensSettings.parameters.at(i).min, mSensSettings.parameters.at(i).max);
        appendDomValueNode(XMLparameter, HMF_AVERAGE, mSensSettings.parameters.at(i).aver);
        appendDomValueNode(XMLparameter, HMF_SIGMA, mSensSettings.parameters.at(i).sigma);
    }

    //Variables
    QDomElement XMLobjectives = appendDomElement(XMLsens, HMF_PLOTVARIABLES);
    for(int i = 0; i < mSensSettings.variables.size(); ++i)
    {
        QDomElement XMLobjective = appendDomElement(XMLobjectives, HMF_PLOTVARIABLE);
        appendDomTextNode(XMLobjective, HMF_COMPONENTTAG, mSensSettings.variables.at(i).compName);
        appendDomTextNode(XMLobjective, HMF_PORTTAG, mSensSettings.variables.at(i).portName);
        appendDomTextNode(XMLobjective, HMF_PLOTVARIABLE, mSensSettings.variables.at(i).varName);
    }
}


void SystemContainer::loadSensitivityAnalysisSettingsFromDomElement(QDomElement &rDomElement)
{
    qDebug() << rDomElement.toDocument().toString();

    QDomElement settingsElement = rDomElement.firstChildElement(HMF_SETTINGS);
    if(!settingsElement.isNull())
    {
        mSensSettings.nIter = parseDomIntegerNode(settingsElement.firstChildElement(HMF_ITERATIONS), mSensSettings.nIter);
        QDomElement distElement = settingsElement.firstChildElement(HMF_DISTRIBUTIONTYPE);
        if(!distElement.isNull() && distElement.text() == HMF_UNIFORMDIST)
        {
            mSensSettings.distribution = SensitivityAnalysisSettings::UniformDistribution;
        }
        else if(!distElement.isNull() && distElement.text() == HMF_NORMALDIST)
        {
            mSensSettings.distribution = SensitivityAnalysisSettings::NormalDistribution;
        }
    }

    QDomElement parametersElement = rDomElement.firstChildElement(HMF_PARAMETERS);
    if(!parametersElement.isNull())
    {
        QDomElement parameterElement =parametersElement.firstChildElement(HMF_PARAMETERTAG);
        while (!parameterElement.isNull())
        {
            SensitivityAnalysisParameter par;
            par.compName = parameterElement.firstChildElement(HMF_COMPONENTTAG).text();
            par.parName = parameterElement.firstChildElement(HMF_PARAMETERTAG).text();
            parseDomValueNode2(parameterElement.firstChildElement(HMF_MINMAX), par.min, par.max);
            par.aver = parseDomValueNode(parameterElement.firstChildElement(HMF_AVERAGE), 0);
            par.sigma = parseDomValueNode(parameterElement.firstChildElement(HMF_SIGMA), 0);
            mSensSettings.parameters.append(par);

            parameterElement = parameterElement.nextSiblingElement(HMF_PARAMETERTAG);
        }
    }

    QDomElement variablesElement = rDomElement.firstChildElement(HMF_PLOTVARIABLES);
    if(!variablesElement.isNull())
    {
        QDomElement variableElement = variablesElement.firstChildElement(HMF_PLOTVARIABLE);
        while (!variableElement.isNull())
        {
            SensitivityAnalysisVariable var;

            var.compName = variableElement.firstChildElement(HMF_COMPONENTTAG).text();
            var.portName = variableElement.firstChildElement(HMF_PORTTAG).text();
            var.varName = variableElement.firstChildElement(HMF_PLOTVARIABLE).text();
            mSensSettings.variables.append(var);

            variableElement = variableElement.nextSiblingElement((HMF_PLOTVARIABLE));
        }
    }
}


void SystemContainer::saveOptimizationSettingsToDomElement(QDomElement &rDomElement)
{
    QDomElement XMLopt = appendDomElement(rDomElement, HMF_OPTIMIZATION);
    QDomElement XMLsetting = appendDomElement(XMLopt, HMF_SETTINGS);
    appendDomTextNode(XMLsetting, HMF_SCRIPTFILETAG, mOptSettings.mScriptFile);
    appendDomIntegerNode(XMLsetting, HMF_ITERATIONS, mOptSettings.mNiter);
    appendDomIntegerNode(XMLsetting, HMF_SEARCHPOINTS, mOptSettings.mNsearchp);
    appendDomValueNode(XMLsetting, HMF_REFLCOEFF, mOptSettings.mRefcoeff);
    appendDomValueNode(XMLsetting, HMF_RANDOMFACTOR, mOptSettings.mRandfac);
    appendDomValueNode(XMLsetting, HMF_FORGETTINGFACTOR, mOptSettings.mForgfac);
    appendDomValueNode(XMLsetting, HMF_PARTOL, mOptSettings.mPartol);
    appendDomBooleanNode(XMLsetting, HMF_PLOT, mOptSettings.mPlot);
    appendDomBooleanNode(XMLsetting, HMF_SAVECSV, mOptSettings.mSavecsv);
    appendDomBooleanNode(XMLsetting, HMF_SAVECSV, mOptSettings.mFinalEval);

    //Parameters
    appendDomBooleanNode(XMLsetting, HMF_LOGPAR, mOptSettings.mlogPar);
    QDomElement XMLparameters = appendDomElement(XMLopt, HMF_PARAMETERS);
    for(int i = 0; i < mOptSettings.mParamters.size(); ++i)
    {
        QDomElement XMLparameter = appendDomElement(XMLparameters, HMF_PARAMETERTAG);
        appendDomTextNode(XMLparameter, HMF_COMPONENTTAG, mOptSettings.mParamters.at(i).mComponentName);
        appendDomTextNode(XMLparameter, HMF_PARAMETERTAG, mOptSettings.mParamters.at(i).mParameterName);
        appendDomValueNode2(XMLparameter, HMF_MINMAX, mOptSettings.mParamters.at(i).mMin, mOptSettings.mParamters.at(i).mMax);
    }

    //Objective Functions
    QDomElement XMLobjectives = appendDomElement(XMLopt, HMF_OBJECTIVES);
    for(int i = 0; i < mOptSettings.mObjectives.size(); ++i)
    {
        QDomElement XMLobjective = appendDomElement(XMLobjectives, HMF_OBJECTIVE);
        appendDomTextNode(XMLobjective, HMF_FUNCNAME, mOptSettings.mObjectives.at(i).mFunctionName);
        appendDomValueNode(XMLobjective, HMF_WEIGHT, mOptSettings.mObjectives.at(i).mWeight);
        appendDomValueNode(XMLobjective, HMF_NORM, mOptSettings.mObjectives.at(i).mNorm);
        appendDomValueNode(XMLobjective, HMF_EXP, mOptSettings.mObjectives.at(i).mExp);

        QDomElement XMLObjectiveVariables = appendDomElement(XMLobjective, HMF_PLOTVARIABLES);
        if(!(mOptSettings.mObjectives.at(i).mVariableInfo.isEmpty()))
        {
            for(int j = 0; j < mOptSettings.mObjectives.at(i).mVariableInfo.size(); ++j)
            {
                QDomElement XMLObjectiveVariable = appendDomElement(XMLObjectiveVariables, HMF_PLOTVARIABLE);
                appendDomTextNode(XMLObjectiveVariable, HMF_COMPONENTTAG, mOptSettings.mObjectives.at(i).mVariableInfo.at(j).at(0));
                appendDomTextNode(XMLObjectiveVariable, HMF_PORTTAG, mOptSettings.mObjectives.at(i).mVariableInfo.at(j).at(1));
                appendDomTextNode(XMLObjectiveVariable, HMF_PLOTVARIABLE, mOptSettings.mObjectives.at(i).mVariableInfo.at(j).at(2));
            }
        }


        if(!(mOptSettings.mObjectives.at(i).mData.isEmpty()))
        {
            QDomElement XMLdata = appendDomElement(XMLobjective, HMF_DATA);
            for(int j = 0; j < mOptSettings.mObjectives.at(i).mData.size(); ++j)
            {
                appendDomTextNode(XMLdata, HMF_PARAMETERTAG, mOptSettings.mObjectives.at(i).mData.at(j));
            }
        }
    }
}


void SystemContainer::loadOptimizationSettingsFromDomElement(QDomElement &rDomElement)
{
    qDebug() << rDomElement.toDocument().toString();

    QDomElement settingsElement = rDomElement.firstChildElement(HMF_SETTINGS);
    if(!settingsElement.isNull())
    {
        mOptSettings.mScriptFile = parseDomStringNode(settingsElement.firstChildElement(HMF_SCRIPTFILETAG), mOptSettings.mScriptFile);
        mOptSettings.mNiter = parseDomIntegerNode(settingsElement.firstChildElement(HMF_ITERATIONS), mOptSettings.mNiter);
        mOptSettings.mNsearchp = parseDomIntegerNode(settingsElement.firstChildElement(HMF_SEARCHPOINTS), mOptSettings.mNsearchp);
        mOptSettings.mRefcoeff = parseDomValueNode(settingsElement.firstChildElement(HMF_REFLCOEFF), mOptSettings.mRefcoeff);
        mOptSettings.mRandfac = parseDomValueNode(settingsElement.firstChildElement(HMF_RANDOMFACTOR), mOptSettings.mRandfac);
        mOptSettings.mForgfac = parseDomValueNode(settingsElement.firstChildElement(HMF_FORGETTINGFACTOR), mOptSettings.mForgfac);
        mOptSettings.mPartol = parseDomValueNode(settingsElement.firstChildElement(HMF_PARTOL), mOptSettings.mPartol);
        mOptSettings.mPlot = parseDomBooleanNode(settingsElement.firstChildElement(HMF_PLOT), mOptSettings.mPlot);
        mOptSettings.mSavecsv = parseDomBooleanNode(settingsElement.firstChildElement(HMF_SAVECSV), mOptSettings.mSavecsv);
        mOptSettings.mFinalEval = parseDomBooleanNode(settingsElement.firstChildElement(HMF_FINALEVAL), mOptSettings.mFinalEval);
        mOptSettings.mlogPar = parseDomBooleanNode(settingsElement.firstChildElement(HMF_LOGPAR), mOptSettings.mlogPar);
    }

    QDomElement parametersElement = rDomElement.firstChildElement(HMF_PARAMETERS);
    if(!parametersElement.isNull())
    {
        QDomElement parameterElement = parametersElement.firstChildElement(HMF_PARAMETERTAG);
        while (!parameterElement.isNull())
        {
            OptParameter parameter;
            parameter.mComponentName = parameterElement.firstChildElement(HMF_COMPONENTTAG).text();
            parameter.mParameterName = parameterElement.firstChildElement(HMF_PARAMETERTAG).text();
            parseDomValueNode2(parameterElement.firstChildElement(HMF_MINMAX), parameter.mMin, parameter.mMax);
            mOptSettings.mParamters.append(parameter);

            parameterElement = parameterElement.nextSiblingElement(HMF_PARAMETERTAG);
        }
    }

    QDomElement objectivesElement = rDomElement.firstChildElement(HMF_OBJECTIVES);
    if(!objectivesElement.isNull())
    {
        QDomElement objElement = objectivesElement.firstChildElement(HMF_OBJECTIVE);
        while (!objElement.isNull())
        {
            Objectives objectives;

            objectives.mFunctionName = objElement.firstChildElement(HMF_FUNCNAME).text();
            objectives.mWeight = objElement.firstChildElement(HMF_WEIGHT).text().toDouble();
            objectives.mNorm = objElement.firstChildElement(HMF_NORM).text().toDouble();
            objectives.mExp = objElement.firstChildElement(HMF_EXP).text().toDouble();

            QDomElement variablesElement = objElement.firstChildElement(HMF_PLOTVARIABLES);
            if(!variablesElement.isNull())
            {
                QDomElement varElement = variablesElement.firstChildElement(HMF_PLOTVARIABLE);
                while (!varElement.isNull())
                {
                    QStringList variableInfo;

                    variableInfo.append(varElement.firstChildElement(HMF_COMPONENTTAG).text());
                    variableInfo.append(varElement.firstChildElement(HMF_PORTTAG).text());
                    variableInfo.append(varElement.firstChildElement(HMF_PLOTVARIABLE).text());

                    objectives.mVariableInfo.append(variableInfo);

                    varElement = varElement.nextSiblingElement(HMF_PLOTVARIABLE);
                }
            }

            QDomElement dataElement = objElement.firstChildElement(HMF_DATA);
            if(!dataElement.isNull())
            {
                QDomElement parElement = dataElement.firstChildElement(HMF_PARAMETERTAG);
                while (!parElement.isNull())
                {
                    objectives.mData.append(parElement.text());

                    parElement = parElement.nextSiblingElement(HMF_PARAMETERTAG);
                }
            }

            objElement = objElement.nextSiblingElement(HMF_OBJECTIVE);

            mOptSettings.mObjectives.append(objectives);
        }
    }
}


void SystemContainer::getSensitivityAnalysisSettings(SensitivityAnalysisSettings &sensSettings)
{
    sensSettings = mSensSettings;
}


void SystemContainer::setSensitivityAnalysisSettings(SensitivityAnalysisSettings &sensSettings)
{
    mSensSettings = sensSettings;
}


void SystemContainer::getOptimizationSettings(OptimizationSettings &optSettings)
{
    optSettings = mOptSettings;
}


void SystemContainer::setOptimizationSettings(OptimizationSettings &optSettings)
{
    mOptSettings = optSettings;
}


//! @brief Saves the System specific GUI data to XML DOM Element
//! @param[in] rDomElement The DOM Element to save to
QDomElement SystemContainer::saveGuiDataToDomElement(QDomElement &rDomElement)
{
    QDomElement guiStuff = ModelObject::saveGuiDataToDomElement(rDomElement);

    //Save animation disabled setting
    QDomElement animationElement = guiStuff.firstChildElement(HMF_ANIMATION);
    animationElement.setAttribute(HMF_DISABLEDTAG, bool2str(mAnimationDisabled));

    //Should we try to append appearancedata stuff, we don't want this in external systems as they contain their own appearance
    if (mLoadType!="EXTERNAL")
    {
        //Append system meta info
        QString author, email, affiliation, description;
        getModelInfo(author, email, affiliation, description);
        if (!(author.isEmpty() && email.isEmpty() && affiliation.isEmpty() && description.isEmpty()))
        {
            QDomElement infoElement = appendDomElement(guiStuff, HMF_INFOTAG);
            appendDomTextNode(infoElement, HMF_AUTHORTAG, author);
            appendDomTextNode(infoElement, HMF_EMAILTAG, email);
            appendDomTextNode(infoElement, HMF_AFFILIATIONTAG, affiliation);
            appendDomTextNode(infoElement, HMF_DESCRIPTIONTAG, description);
        }

        GraphicsViewPort vp = this->getGraphicsViewport();
        appendViewPortTag(guiStuff, vp.mCenter.x(), vp.mCenter.y(), vp.mZoom);

        QDomElement portsHiddenElement = appendDomElement(guiStuff, HMF_PORTSTAG);
        portsHiddenElement.setAttribute("hidden", !mShowSubComponentPorts);
        QDomElement namesHiddenElement = appendDomElement(guiStuff, HMF_NAMESTAG);
        namesHiddenElement.setAttribute("hidden", !mShowSubComponentNames);

        QString gfxType = "iso";
        if(mGfxType == UserGraphics)
            gfxType = "user";
        QDomElement gfxTypeElement = appendDomElement(guiStuff, HMF_GFXTAG);
        gfxTypeElement.setAttribute("type", gfxType);

        QDomElement scriptFileElement = appendDomElement(guiStuff, HMF_SCRIPTFILETAG);
        scriptFileElement.setAttribute("path", mScriptFilePath);

        this->refreshExternalPortsAppearanceAndPosition();
        QDomElement xmlApp = appendOrGetCAFRootTag(guiStuff);

        //Before we save the modelobjectappearance data we need to set the correct basepath, (we ask our parent it will know)
        if (this->getParentContainerObject() != 0)
        {
            this->mModelObjectAppearance.setBasePath(this->getParentContainerObject()->getAppearanceData()->getBasePath());
        }
        this->mModelObjectAppearance.saveToDomElement(xmlApp);
    }

    saveOptimizationSettingsToDomElement(guiStuff);
    saveSensitivityAnalysisSettingsToDomElement(guiStuff);

    //Save undo stack if setting is activated
    if(mSaveUndoStack)
    {
        guiStuff.appendChild(mpUndoStack->toXml());
    }

    return guiStuff;
}

//! @brief Overloaded special XML DOM save function for System Objects
//! @param[in] rDomElement The DOM Element to save to
void SystemContainer::saveToDomElement(QDomElement &rDomElement, SaveContentsEnumT contents)
{
    //qDebug() << "Saving to dom node in: " << this->mModelObjectAppearance.getName();
    QDomElement xmlSubsystem = appendDomElement(rDomElement, getHmfTagName());

    //! @todo maybe use enums instead of strings
    //! @todo should not need to set this here
    if (mpParentContainerObject==0)
    {
        mLoadType = "ROOT"; //!< @todo this is a temporary hack for the xml save function (see bellow)
    }
    else if (!mModelFileInfo.filePath().isEmpty())
    {
        mLoadType = "EXTERNAL";
    }
    else
    {
        mLoadType = "EMBEDED";
    }

    // Save Core related stuff
    this->saveCoreDataToDomElement(xmlSubsystem, contents);

    if(contents==FullModel)
    {
        // Save gui object stuff
        this->saveGuiDataToDomElement(xmlSubsystem);
    }

    //Replace volunector with connectors and component
    QList<Connector*> volunectorPtrs;
    QList<Connector*> tempConnectorPtrs;  //To be removed later
    QList<ModelObject*> tempComponentPtrs; //To be removed later
    for(int i=0; i<mSubConnectorList.size(); ++i)
    {
        if(mSubConnectorList[i]->isVolunector())
        {
            Connector *pVolunector = mSubConnectorList[i];
            volunectorPtrs.append(pVolunector);
            mSubConnectorList.removeAll(pVolunector);
            --i;

            tempComponentPtrs.append(pVolunector->getVolunectorComponent());

            tempConnectorPtrs.append(new Connector(this));
            tempConnectorPtrs.last()->setStartPort(pVolunector->getStartPort());
            tempConnectorPtrs.last()->setEndPort(tempComponentPtrs.last()->getPort("P1"));
            QVector<QPointF> points;
            QStringList geometries;
            points.append(pVolunector->mapToScene(pVolunector->getLine(0)->line().p1()));
            for(int j=0; j<pVolunector->getNumberOfLines(); ++j)
            {
                points.append(pVolunector->mapToScene(pVolunector->getLine(j)->line().p2()));
                if(pVolunector->getGeometry(j) == Horizontal)
                    geometries.append("horizontal");
                else if(pVolunector->getGeometry(j) == Vertical)
                    geometries.append("vertical");
                else
                    geometries.append("diagonal");
            }
            tempConnectorPtrs.last()->setPointsAndGeometries(points, geometries);

            tempConnectorPtrs.append(new Connector(this));
            tempConnectorPtrs.last()->setStartPort(tempComponentPtrs.last()->getPort("P2"));
            tempConnectorPtrs.last()->setEndPort(pVolunector->getEndPort());
        }

        for(int j=0; j<tempComponentPtrs.size(); ++j)
        {
            mModelObjectMap.insert(tempComponentPtrs[j]->getName(), tempComponentPtrs[j]);
        }
    }
    mSubConnectorList.append(tempConnectorPtrs);

        //Save all of the sub objects
    if (mLoadType=="EMBEDED" || mLoadType=="ROOT")
    {
            //Save subcomponents and subsystems
        QDomElement xmlObjects = appendDomElement(xmlSubsystem, HMF_OBJECTS);
        ModelObjectMapT::iterator it;
        for(it = mModelObjectMap.begin(); it!=mModelObjectMap.end(); ++it)
        {
            it.value()->saveToDomElement(xmlObjects, contents);
            if(tempComponentPtrs.contains(it.value()))
            {
                xmlObjects.lastChildElement().setAttribute("volunector", "true");
            }
        }

        if(contents==FullModel)
        {
                //Save all widgets
            QMap<size_t, Widget *>::iterator itw;
            for(itw = mWidgetMap.begin(); itw!=mWidgetMap.end(); ++itw)
            {
                itw.value()->saveToDomElement(xmlObjects);
            }

                //Save the connectors
            QDomElement xmlConnections = appendDomElement(xmlSubsystem, HMF_CONNECTIONS);
            for(int i=0; i<mSubConnectorList.size(); ++i)
            {
                mSubConnectorList[i]->saveToDomElement(xmlConnections);
            }
        }
    }

    //Remove temporary connectors/components and re-add volunectors
    for(int i=0; i<tempConnectorPtrs.size(); ++i)
    {
        mSubConnectorList.removeAll(tempConnectorPtrs[i]);

        Connector *pConnector = tempConnectorPtrs[i];
        Port *pStartPort = pConnector->getStartPort();
        ModelObject *pStartComponent = pStartPort->getParentModelObject();
        Port *pEndPort = pConnector->getEndPort();
        ModelObject *pEndComponent = pEndPort->getParentModelObject();

        pStartPort->forgetConnection(pConnector);
        pStartComponent->forgetConnector(pConnector);
        pEndPort->forgetConnection(pConnector);
        pEndComponent->forgetConnector(pConnector);

        delete(tempConnectorPtrs[i]);
    }
    for(int i=0; i<volunectorPtrs.size(); ++i)
    {
        mSubConnectorList.append(volunectorPtrs[i]);
    }
    for(int i=0; i<tempComponentPtrs.size(); ++i)
    {
        mModelObjectMap.remove(tempComponentPtrs[i]->getName());
    }

}

//! @brief Loads a System from an XML DOM Element
//! @param[in] rDomElement The element to load from
void SystemContainer::loadFromDomElement(QDomElement domElement)
{
    // Loop back up to root level to get version numbers
    QString hmfFormatVersion = domElement.ownerDocument().firstChildElement(HMF_ROOTTAG).attribute(HMF_VERSIONTAG, "0");
    QString coreHmfVersion = domElement.ownerDocument().firstChildElement(HMF_ROOTTAG).attribute(HMF_HOPSANCOREVERSIONTAG, "0");

    // Check if the subsystem is external or internal, and load appropriately
    QString external_path = domElement.attribute(HMF_EXTERNALPATHTAG);
    if (external_path.isEmpty())
    {
        // Load embedded subsystem
        // 0. Load core and gui stuff
        //! @todo might need some error checking here in case some fields are missing
        // Now load the core specific data, might need inherited function for this
        this->setName(domElement.attribute(HMF_NAMETAG));

        // Load the NumHop script
        setNumHopScript(parseDomStringNode(domElement.firstChildElement(HMF_NUMHOPSCRIPT), ""));

        // Begin loading GUI stuff like appearance data and viewport
        QDomElement guiStuff = domElement.firstChildElement(HMF_HOPSANGUITAG);
        mModelObjectAppearance.readFromDomElement(guiStuff.firstChildElement(CAF_ROOT).firstChildElement(CAF_MODELOBJECT));
        refreshDisplayName(); // This must be done because in some occasions the loadAppearanceData line above will overwrite the correct name

        QDomElement animationElement = guiStuff.firstChildElement(HMF_ANIMATION);
        bool animationDisabled = false;
        if(!animationElement.isNull())
        {
            animationDisabled = parseAttributeBool(animationElement, HMF_DISABLEDTAG, false);
        }
        setAnimationDisabled(animationDisabled);

        // Load system/model info
        QDomElement infoElement = domElement.parentNode().firstChildElement(HMF_INFOTAG); //!< @deprecated info tag is in the system from 0.7.5 an onwards, this line loads from old models
        if (infoElement.isNull())
        {
            infoElement = guiStuff.firstChildElement(HMF_INFOTAG);
        }
        if(!infoElement.isNull())
        {
            QString author, email, affiliation, description;
            QDomElement authorElement = infoElement.firstChildElement(HMF_AUTHORTAG);
            if(!authorElement.isNull())
            {
                author = authorElement.text();
            }
            QDomElement emailElement = infoElement.firstChildElement(HMF_EMAILTAG);
            if(!emailElement.isNull())
            {
                email = emailElement.text();
            }
            QDomElement affiliationElement = infoElement.firstChildElement(HMF_AFFILIATIONTAG);
            if(!affiliationElement.isNull())
            {
                affiliation = affiliationElement.text();
            }
            QDomElement descriptionElement = infoElement.firstChildElement(HMF_DESCRIPTIONTAG);
            if(!descriptionElement.isNull())
            {
                description = descriptionElement.text();
            }

            this->setModelInfo(author, email, affiliation, description);
        }

        // Now lets check if the icons were loaded successfully else we may want to ask the library widget for the graphics (components saved as subsystems)
        if (!mModelObjectAppearance.iconValid(UserGraphics) || !mModelObjectAppearance.iconValid(ISOGraphics))
        {
            SharedModelObjectAppearanceT pApp = gpLibraryHandler->getModelObjectAppearancePtr(mModelObjectAppearance.getTypeName(), mModelObjectAppearance.getSubTypeName());
            if (pApp)
            {
                // If our user graphics is invalid but library has valid data then set from library
                if (!mModelObjectAppearance.iconValid(UserGraphics) && pApp->iconValid(UserGraphics))
                {
                    setIconPath(pApp->getIconPath(UserGraphics, Absolute), UserGraphics, Absolute);
                }

                // If our iso graphics is invalid but library has valid data then set from library
                if (!mModelObjectAppearance.iconValid(ISOGraphics) && pApp->iconValid(ISOGraphics))
                {
                    setIconPath(pApp->getIconPath(ISOGraphics, Absolute), ISOGraphics, Absolute);
                }
            }
        }

        // Continue loading GUI stuff like appearance data and viewport
        this->mShowSubComponentNames = !parseAttributeBool(guiStuff.firstChildElement(HMF_NAMESTAG),"hidden",true);
        this->mShowSubComponentPorts = !parseAttributeBool(guiStuff.firstChildElement(HMF_PORTSTAG),"hidden",true);
        QString gfxType = guiStuff.firstChildElement(HMF_GFXTAG).attribute("type");
        if(gfxType == "user") { mGfxType = UserGraphics; }
        else if(gfxType == "iso") { mGfxType = ISOGraphics; }
        //! @todo these two should not be set here
        gpToggleNamesAction->setChecked(mShowSubComponentNames);
        gpTogglePortsAction->setChecked(mShowSubComponentPorts);
        double x = guiStuff.firstChildElement(HMF_VIEWPORTTAG).attribute("x").toDouble();
        double y = guiStuff.firstChildElement(HMF_VIEWPORTTAG).attribute("y").toDouble();
        double zoom = guiStuff.firstChildElement(HMF_VIEWPORTTAG).attribute("zoom").toDouble();
        setScriptFile(guiStuff.firstChildElement(HMF_SCRIPTFILETAG).attribute("path"));

        bool dontClearUndo = false;
        if(!guiStuff.firstChildElement(HMF_UNDO).isNull())
        {
            QDomElement undoElement = guiStuff.firstChildElement(HMF_UNDO);
            mpUndoStack->fromXml(undoElement);
            dontClearUndo = true;
            mSaveUndoStack = true;      //Set save undo stack setting to true if loading a hmf file with undo stack saved
        }

        // Only set viewport and zoom if the system being loaded is the one shown in the view
        // But make system remember the setting anyway
        this->setGraphicsViewport(GraphicsViewPort(x,y,zoom));
        if (mpModelWidget->getViewContainerObject() == this)
        {
            mpModelWidget->getGraphicsView()->setViewPort(GraphicsViewPort(x,y,zoom));
        }

        //Load simulation time
        QString startT,stepT,stopT;
        bool inheritTs;
        parseSimulationTimeTag(domElement.firstChildElement(HMF_SIMULATIONTIMETAG), startT, stepT, stopT, inheritTs);
        this->setTimeStep(stepT.toDouble());
        mpCoreSystemAccess->setInheritTimeStep(inheritTs);

        // Load number of log samples
        parseLogSettingsTag(domElement.firstChildElement(HMF_SIMULATIONLOGSETTINGS), mLogStartTime, mNumberOfLogSamples);
        //! @deprecated 20131002 we keep this below for backwards compatibility for a while
        if(domElement.hasAttribute(HMF_LOGSAMPLES))
        {
            mNumberOfLogSamples = domElement.attribute(HMF_LOGSAMPLES).toInt();
        }

        // Only set start stop time for the top level system
        if (mpParentContainerObject == 0)
        {
            mpModelWidget->setTopLevelSimulationTime(startT,stepT,stopT);
        }

        //1. Load global parameters
        QDomElement xmlParameters = domElement.firstChildElement(HMF_PARAMETERS);
        QDomElement xmlSubObject = xmlParameters.firstChildElement(HMF_PARAMETERTAG);
        while (!xmlSubObject.isNull())
        {
            loadSystemParameter(xmlSubObject, true, hmfFormatVersion, this);
            xmlSubObject = xmlSubObject.nextSiblingElement(HMF_PARAMETERTAG);
        }

        //2. Load all sub-components
        QList<ModelObject*> volunectorObjectPtrs;
        QDomElement xmlSubObjects = domElement.firstChildElement(HMF_OBJECTS);
        xmlSubObject = xmlSubObjects.firstChildElement(HMF_COMPONENTTAG);
        while (!xmlSubObject.isNull())
        {
            verifyHmfComponentCompatibility(xmlSubObject, hmfFormatVersion, coreHmfVersion);
            ModelObject* pObj = loadModelObject(xmlSubObject, this, NoUndo);
            if(pObj == nullptr)
            {
                gpMessageHandler->addErrorMessage(QString("Model contains component from a library that has not been loaded. TypeName: ") +
                                                                    xmlSubObject.attribute(HMF_TYPENAME) + QString(", Name: ") + xmlSubObject.attribute(HMF_NAMETAG));

                // Insert missing component dummy instead
                xmlSubObject.setAttribute(HMF_TYPENAME, "MissingComponent");
                pObj = loadModelObject(xmlSubObject, this, NoUndo);
            }
            else
            {



                //! @deprecated This StartValue load code is only kept for up converting old files, we should keep it here until we have some other way of up converting old formats
                //Load start values //Is not needed, start values are saved as ordinary parameters! This code snippet can probably be removed.
                QDomElement xmlStartValues = xmlSubObject.firstChildElement(HMF_STARTVALUES);
                QDomElement xmlStartValue = xmlStartValues.firstChildElement(HMF_STARTVALUE);
                while (!xmlStartValue.isNull())
                {
                    loadStartValue(xmlStartValue, pObj, NoUndo);
                    xmlStartValue = xmlStartValue.nextSiblingElement(HMF_STARTVALUE);
                }
            }
            if(xmlSubObject.attribute("volunector") == "true")
            {
                volunectorObjectPtrs.append(pObj);
            }

//            if(pObj && pObj->getTypeName().startsWith("CppComponent"))
//            {
//                recompileCppComponents(pObj);
//            }

            xmlSubObject = xmlSubObject.nextSiblingElement(HMF_COMPONENTTAG);
        }

        //3. Load all text box widgets
        xmlSubObject = xmlSubObjects.firstChildElement(HMF_TEXTBOXWIDGETTAG);
        while (!xmlSubObject.isNull())
        {
            loadTextBoxWidget(xmlSubObject, this, NoUndo);
            xmlSubObject = xmlSubObject.nextSiblingElement(HMF_TEXTBOXWIDGETTAG);
        }

        //5. Load all sub-systems
        xmlSubObject = xmlSubObjects.firstChildElement(HMF_SYSTEMTAG);
        while (!xmlSubObject.isNull())
        {
            loadModelObject(xmlSubObject, this, NoUndo);
            xmlSubObject = xmlSubObject.nextSiblingElement(HMF_SYSTEMTAG);
        }

        //6. Load all system ports
        xmlSubObject = xmlSubObjects.firstChildElement(HMF_SYSTEMPORTTAG);
        while (!xmlSubObject.isNull())
        {
            loadContainerPortObject(xmlSubObject, this, NoUndo);
            xmlSubObject = xmlSubObject.nextSiblingElement(HMF_SYSTEMPORTTAG);
        }

        //7. Load all connectors
        QDomElement xmlConnections = domElement.firstChildElement(HMF_CONNECTIONS);
        xmlSubObject = xmlConnections.firstChildElement(HMF_CONNECTORTAG);
        QList<QDomElement> failedConnections;
        while (!xmlSubObject.isNull())
        {
            if(!loadConnector(xmlSubObject, this, NoUndo))
            {
//                failedConnections.append(xmlSubObject);
            }
            xmlSubObject = xmlSubObject.nextSiblingElement(HMF_CONNECTORTAG);
        }
//        //If some connectors failed to load, it could mean that they were loaded in wrong order.
//        //Try again until they work, or abort if number of attempts are greater than maximum possible for success.
//        int stop=failedConnections.size()*(failedConnections.size()+1)/2;
//        int i=0;
//        while(!failedConnections.isEmpty())
//        {
//            if(!loadConnector(failedConnections.first(), this, NoUndo))
//            {
//                failedConnections.append(failedConnections.first());
//            }
//            failedConnections.removeFirst();
//            ++i;
//            if(i>stop) break;
//        }


        //8. Load system parameters again in case we need to reregister system port start values
        xmlParameters = domElement.firstChildElement(HMF_PARAMETERS);
        xmlSubObject = xmlParameters.firstChildElement(HMF_PARAMETERTAG);
        while (!xmlSubObject.isNull())
        {
            loadSystemParameter(xmlSubObject, false, hmfFormatVersion, this);
            xmlSubObject = xmlSubObject.nextSiblingElement(HMF_PARAMETERTAG);
        }

        //9. Load plot variable aliases
        QDomElement xmlAliases = domElement.firstChildElement(HMF_ALIASES);
        QDomElement xmlAlias = xmlAliases.firstChildElement(HMF_ALIAS);
        while (!xmlAlias.isNull())
        {
            loadPlotAlias(xmlAlias, this);
            xmlAlias = xmlAlias.nextSiblingElement(HMF_ALIAS);
        }

        //9.1 Load plot variable aliases
        //! @deprecated Remove in the future when hmf format stabilized and everyone has upgraded
        xmlSubObject = xmlParameters.firstChildElement(HMF_ALIAS);
        while (!xmlSubObject.isNull())
        {
            loadPlotAlias(xmlSubObject, this);
            xmlSubObject = xmlSubObject.nextSiblingElement(HMF_ALIAS);
        }

        //10. Load optimization settings
        xmlSubObject = guiStuff.firstChildElement(HMF_OPTIMIZATION);
        loadOptimizationSettingsFromDomElement(xmlSubObject);

        //11. Load sensitivity analysis settings
        xmlSubObject = guiStuff.firstChildElement(HMF_SENSITIVITYANALYSIS);
        loadSensitivityAnalysisSettingsFromDomElement(xmlSubObject);


        //Replace volunector components with volunectors
        for(int i=0; i<volunectorObjectPtrs.size(); ++i)
        {
            if(volunectorObjectPtrs[i]->getPort("P1")->isConnected() &&
                volunectorObjectPtrs[i]->getPort("P2")->isConnected())
            {


                Port *pP1 = volunectorObjectPtrs[i]->getPort("P1");
                Port *pP2 = volunectorObjectPtrs[i]->getPort("P2");
                Connector *pVolunector = pP1->getAttachedConnectorPtrs().first();
                Connector *pExcessiveConnector = pP2->getAttachedConnectorPtrs().first();
                Port *pEndPort = pExcessiveConnector->getEndPort();
                ModelObject *pEndComponent = pEndPort->getParentModelObject();
                ModelObject *pVolunectorObject = volunectorObjectPtrs[i];


                //Forget and remove excessive connector
                mSubConnectorList.removeAll(pExcessiveConnector);
                pVolunectorObject->forgetConnector(pExcessiveConnector);    //Start component
                pEndComponent->forgetConnector(pExcessiveConnector);        //Start port
                pP2->forgetConnection(pExcessiveConnector);                 //End component
                pEndPort->forgetConnection(pExcessiveConnector);            //End port
                delete(pExcessiveConnector);

                //Disconnect volunector from volunector component
                pVolunectorObject->forgetConnector(pVolunector);
                pP1->forgetConnection(pVolunector);

                //Re-connect volunector with end component
                pVolunector->setEndPort(pEndPort);

                //Make the connector a volunector
                pVolunector->makeVolunector(dynamic_cast<Component*>(pVolunectorObject));

                //Remove volunector object parent container object
                mModelObjectMap.remove(pVolunectorObject->getName());
                pVolunectorObject->setParent(0);

                //Re-draw connector object
                pVolunector->drawConnector();
            }
        }


        //Refresh the appearance of the subsystem and create the GUIPorts based on the loaded portappearance information
        //! @todo This is a bit strange, refreshAppearance MUST be run before create ports or create ports will not know some necessary stuff
        this->refreshAppearance();
        this->refreshExternalPortsAppearanceAndPosition();
        //this->createPorts();

        //Deselect all components
        this->deselectAll();
        if(!dontClearUndo)
        {
            this->mpUndoStack->clear();
        }
        //Only do this for the root system
        //! @todo maybe can do this for subsystems to (even if we don't see them right now)
        if (this->mpParentContainerObject == nullptr)
        {
            //mpParentModelWidget->getGraphicsView()->centerView();
            mpModelWidget->getGraphicsView()->updateViewPort();
        }
        this->mpModelWidget->setSaved(true);

#ifdef USEPYTHONQT
        gpPythonTerminalWidget->runPyScript(mScriptFilePath);
#endif

        emit systemParametersChanged(); // Make sure we refresh the syspar widget
        emit checkMessages();
    }
    else
    {
        gpMessageHandler->addErrorMessage("A system you tried to load is taged as an external system, but the ContainerSystem load function only loads embeded systems");
    }
}


void SystemContainer::exportToLabView()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(gpMainWindowWidget, tr("Export to LabVIEW/SIT"),
                                  "This will create source code for a LabVIEW/SIT DLL-file from current model. The  HopsanCore source code is included but you will neee Visual Studio 2003 to compile it.\nContinue?",
                                  QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::No)
        return;

    //Open file dialog and initialize the file stream
    QString filePath;
    filePath = QFileDialog::getSaveFileName(gpMainWindowWidget, tr("Export Project to HopsanRT Wrapper Code"),
                                            gpConfig->getStringSetting(CFG_LABVIEWEXPORTDIR),
                                            tr("C++ Source File (*.cpp)"));
    if(filePath.isEmpty()) return;    //Don't save anything if user presses cancel

    QFileInfo file(filePath);
    gpConfig->setStringSetting(CFG_LABVIEWEXPORTDIR, file.absolutePath());

    auto spGenerator = createDefaultExportGenerator();
    if (!spGenerator->generateToLabViewSIT(filePath, mpCoreSystemAccess->getCoreSystemPtr()))
    {
        gpMessageHandler->addErrorMessage("LabView SIT export failed");
    }
}

void SystemContainer::exportToFMU1_32()
{
    exportToFMU("", 1, ArchitectureEnumT::x86);
}

void SystemContainer::exportToFMU1_64()
{
    exportToFMU("", 1, ArchitectureEnumT::x64);
}

void SystemContainer::exportToFMU2_32()
{
    exportToFMU("", 2, ArchitectureEnumT::x86);
}

void SystemContainer::exportToFMU2_64()
{
    exportToFMU("", 2, ArchitectureEnumT::x64);
}




void SystemContainer::exportToFMU(QString savePath, int version, ArchitectureEnumT arch)
{
    if(savePath.isEmpty())
    {
        //Open file dialog and initialize the file stream
        QDir fileDialogSaveDir;
        savePath = QFileDialog::getExistingDirectory(gpMainWindowWidget, tr("Create Functional Mockup Unit"),
                                                        gpConfig->getStringSetting(CFG_FMUEXPORTDIR),
                                                        QFileDialog::ShowDirsOnly
                                                        | QFileDialog::DontResolveSymlinks);
        if(savePath.isEmpty()) return;    //Don't save anything if user presses cancel

        QDir saveDir;
        saveDir.setPath(savePath);
        gpConfig->setStringSetting(CFG_FMUEXPORTDIR, saveDir.absolutePath());
        saveDir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);
        if(!saveDir.entryList().isEmpty())
        {
            qDebug() << saveDir.entryList();
            QMessageBox msgBox;
            msgBox.setWindowIcon(gpMainWindowWidget->windowIcon());
            msgBox.setText(QString("Folder is not empty!"));
            msgBox.setInformativeText("Are you sure you want to export files here?");
            msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            msgBox.setDefaultButton(QMessageBox::No);

            int answer = msgBox.exec();
            if(answer == QMessageBox::No)
            {
                return;
            }
        }
    }

    QDir saveDir(savePath);
    if(!saveDir.exists())
    {
        QDir().mkpath(savePath);
    }
    saveDir.setFilter(QDir::NoFilter);

    if(!mpModelWidget->isSaved())
    {
        QMessageBox::information(gpMainWindowWidget, "tr(Model not saved)", "tr(Please save your model before exporting an FMU)");
        return;
    }

    //Save model to hmf in export directory
    mpModelWidget->saveTo(savePath+"/"+mModelFileInfo.fileName().replace(" ", "_"));

    auto spGenerator = createDefaultExportGenerator();
    spGenerator->setCompilerPath(gpConfig->getCompilerPath(arch));

    HopsanGeneratorGUI::TargetArchitectureT garch;
    if (arch == ArchitectureEnumT::x64)
    {
        garch = HopsanGeneratorGUI::TargetArchitectureT::x64;
    }
    else
    {
        garch = HopsanGeneratorGUI::TargetArchitectureT::x86;
    }
    auto pCoreSystem = mpCoreSystemAccess->getCoreSystemPtr();
    auto fmuVersion = static_cast<HopsanGeneratorGUI::FmuVersionT>(version);
    QStringList externalLibraries;
    //! @todo an idea here is to always treat the default library as external, and export it as such (and never build it in by default), that would reduce special handling of the default library
    //! @todo This code prevents nesting an external fmu inside an export, not sure if we need to support this
    spGenerator->setAutoCloseWidgetsOnSuccess(true);
    for (const auto& pLib : gpLibraryHandler->getLibraries(this->getRequiredComponentLibraries(), LibraryTypeEnumT::ExternalLib)) {
        const auto mainFile = pLib->getLibraryMainFilePath();
        spGenerator->checkComponentLibrary(mainFile);
        externalLibraries.append(pLib->getLibraryMainFilePath());
    }
    spGenerator->setAutoCloseWidgetsOnSuccess(false);
    if (!spGenerator->generateToFmu(savePath, pCoreSystem, externalLibraries, fmuVersion, garch))
    {
        gpMessageHandler->addErrorMessage("Failed to export FMU");
    }
}

//void SystemContainer::exportToFMU()
//{
//    //Open file dialog and initialize the file stream
//    QDir fileDialogSaveDir;
//    QString savePath;
//    savePath = QFileDialog::getExistingDirectory(gpMainWindow, tr("Create Functional Mockup Unit"),
//                                                    gConfig.getFmuExportDir(),
//                                                    QFileDialog::ShowDirsOnly
//                                                    | QFileDialog::DontResolveSymlinks);
//    if(savePath.isEmpty()) return;    //Don't save anything if user presses cancel

//    QDir saveDir;
//    saveDir.setPath(savePath);
//    gConfig.setFmuExportDir(saveDir.absolutePath());
//    saveDir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);
//    if(!saveDir.entryList().isEmpty())
//    {
//        qDebug() << saveDir.entryList();
//        QMessageBox msgBox;
//        msgBox.setWindowIcon(gpMainWindow->windowIcon());
//        msgBox.setText(QString("Folder is not empty!"));
//        msgBox.setInformativeText("Are you sure you want to export files here?");
//        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
//        msgBox.setDefaultButton(QMessageBox::No);

//        int answer = msgBox.exec();
//        if(answer == QMessageBox::No)
//        {
//            return;
//        }
//    }
//    saveDir.setFilter(QDir::NoFilter);


//    //Tells if user selected the gcc compiler or not (= visual studio)
//    //bool gccCompiler = mpExportFmuGccRadioButton->isChecked();


//    //Write the FMU ID
//    int random = rand() % 1000;
//    QString randomString = QString().setNum(random);
//    QString ID = "{8c4e810f-3df3-4a00-8276-176fa3c9f"+randomString+"}";  //!< @todo How is this ID defined?


//    //Collect information about input ports
//    QStringList inputVariables;
//    QStringList inputComponents;
//    QStringList inputPorts;
//    QList<int> inputDatatypes;

//    ModelObjectMapT::iterator it;
//    for(it = mModelObjectMap.begin(); it!=mModelObjectMap.end(); ++it)
//    {
//        if(it.value()->getTypeName() == "SignalInputInterface")
//        {
//            inputVariables.append(it.value()->getName().remove(' '));
//            inputComponents.append(it.value()->getName());
//            inputPorts.append("out");
//            inputDatatypes.append(0);
//        }
//    }


//    //Collect information about output ports
//    QStringList outputVariables;
//    QStringList outputComponents;
//    QStringList outputPorts;
//    QList<int> outputDatatypes;

//    for(it = mModelObjectMap.begin(); it!=mModelObjectMap.end(); ++it)
//    {
//        if(it.value()->getTypeName() == "SignalOutputInterface")
//        {
//            outputVariables.append(it.value()->getName().remove(' '));
//            outputComponents.append(it.value()->getName());
//            outputPorts.append("in");
//            outputDatatypes.append(0);
//        }
//    }


//    //Create file objects for all files that shall be created
//    QFile modelSourceFile;
//    QString modelName = getModelFileInfo().fileName();
//    modelName.chop(4);
//    QString realModelName = modelName;          //Actual model name (used for hmf file)
//    modelName.replace(" ", "_");        //Replace white spaces with underscore, to avoid problems
//    modelSourceFile.setFileName(savePath + "/" + modelName + ".c");
//    if(!modelSourceFile.open(QIODevice::WriteOnly | QIODevice::Text))
//    {
//        gpMessageHandler->addErrorMessage("Failed to open " + modelName + ".c for writing.");
//        return;
//    }

//    QFile modelDescriptionFile;
//    modelDescriptionFile.setFileName(savePath + "/modelDescription.xml");
//    if(!modelDescriptionFile.open(QIODevice::WriteOnly | QIODevice::Text))
//    {
//        gpMessageHandler->addErrorMessage("Failed to open modelDescription.xml for writing.");
//        return;
//    }

//    QFile fmuHeaderFile;
//    fmuHeaderFile.setFileName(savePath + "/HopsanFMU.h");
//    if(!fmuHeaderFile.open(QIODevice::WriteOnly | QIODevice::Text))
//    {
//        gpMessageHandler->addErrorMessage("Failed to open HopsanFMU.h for writing.");
//        return;
//    }

//    QFile fmuSourceFile;
//    fmuSourceFile.setFileName(savePath + "/HopsanFMU.cpp");
//    if(!fmuSourceFile.open(QIODevice::WriteOnly | QIODevice::Text))
//    {
//        gpMessageHandler->addErrorMessage("Failed to open HopsanFMU.cpp for writing.");
//        return;
//    }

//#ifdef _WIN32
//    QFile clBatchFile;
//    clBatchFile.setFileName(savePath + "/compile.bat");
//    if(!clBatchFile.open(QIODevice::WriteOnly | QIODevice::Text))
//    {
//        gpMessageHandler->addErrorMessage("Failed to open compile.bat for writing.");
//        return;
//    }
//#endif

//    //progressBar.setLabelText("Writing modelDescription.xml");
//    //progressBar.setValue(1);

//    QTextStream modelDescriptionStream(&modelDescriptionFile);
//    modelDescriptionStream << "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n";       //!< @todo Encoding, should it be UTF-8?
//    modelDescriptionStream << "<fmiModelDescription\n";
//    modelDescriptionStream << "  fmiVersion=\"1.0\"\n";
//    modelDescriptionStream << "  modelName=\"" << modelName << "\"\n";               //!< @todo What's the difference between name and identifier?
//    modelDescriptionStream << "  modelIdentifier=\"" << modelName << "\"\n";
//    modelDescriptionStream << "  guid=\"" << ID << "\"\n";
//    modelDescriptionStream << "  numberOfContinuousStates=\"" << inputVariables.size() + outputVariables.size() << "\"\n";
//    modelDescriptionStream << "  numberOfEventIndicators=\"0\">\n";
//    modelDescriptionStream << "<ModelVariables>\n";
//    int i, j;
//    for(i=0; i<inputVariables.size(); ++i)
//    {
//        QString refString = QString().setNum(i);
//        modelDescriptionStream << "  <ScalarVariable name=\"" << inputVariables.at(i) << "\" valueReference=\""+refString+"\" description=\"input variable\" causality=\"input\">\n";
//        modelDescriptionStream << "     <Real start=\"0\" fixed=\"false\"/>\n";
//        modelDescriptionStream << "  </ScalarVariable>\n";
//    }
//    for(j=0; j<outputVariables.size(); ++j)
//    {
//        QString refString = QString().setNum(i+j);
//        modelDescriptionStream << "  <ScalarVariable name=\"" << outputVariables.at(j) << "\" valueReference=\""+refString+"\" description=\"output variable\" causality=\"output\">\n";
//        modelDescriptionStream << "     <Real start=\"0\" fixed=\"false\"/>\n";
//        modelDescriptionStream << "  </ScalarVariable>\n";
//    }
//    modelDescriptionStream << "</ModelVariables>\n";
//    modelDescriptionStream << "</fmiModelDescription>\n";
//    modelDescriptionFile.close();


//    //progressBar.setLabelText("Writing " + modelName + ".c");
//    //progressBar.setValue(2);


//    QTextStream modelSourceStream(&modelSourceFile);
//    modelSourceStream << "// Define class name and unique id\n";
//    modelSourceStream << "    #define MODEL_IDENTIFIER " << modelName << "\n";
//    modelSourceStream << "    #define MODEL_GUID \"" << ID << "\"\n\n";
//    modelSourceStream << "    // Define model size\n";
//    modelSourceStream << "    #define NUMBER_OF_REALS " << inputVariables.size() + outputVariables.size() << "\n";
//    modelSourceStream << "    #define NUMBER_OF_INTEGERS 0\n";
//    modelSourceStream << "    #define NUMBER_OF_BOOLEANS 0\n";
//    modelSourceStream << "    #define NUMBER_OF_STRINGS 0\n";
//    modelSourceStream << "    #define NUMBER_OF_STATES "<< inputVariables.size() + outputVariables.size() << "\n";        //!< @todo Does number of variables equal number of states?
//    modelSourceStream << "    #define NUMBER_OF_EVENT_INDICATORS 0\n\n";
//    modelSourceStream << "    // Include fmu header files, typedefs and macros\n";
//    modelSourceStream << "    #include \"fmuTemplate.h\"\n";
//    modelSourceStream << "    #include \"HopsanFMU.h\"\n\n";
//    modelSourceStream << "    // Define all model variables and their value references\n";
//    for(i=0; i<inputVariables.size(); ++i)
//        modelSourceStream << "    #define " << inputVariables.at(i) << "_ " << i << "\n\n";
//    for(j=0; j<outputVariables.size(); ++j)
//        modelSourceStream << "    #define " << outputVariables.at(j) << "_ " << j+i << "\n\n";
//    modelSourceStream << "    // Define state vector as vector of value references\n";
//    modelSourceStream << "    #define STATES { ";
//    i=0;
//    j=0;
//    if(!inputVariables.isEmpty())
//    {
//        modelSourceStream << inputVariables.at(0) << "_";
//        ++i;
//    }
//    else if(!outputVariables.isEmpty())
//    {
//        modelSourceStream << outputVariables.at(0) << "_";
//        ++j;
//    }
//    for(; i<inputVariables.size(); ++i)
//        modelSourceStream << ", " << inputVariables.at(i) << "_";
//    for(; j<outputVariables.size(); ++j)
//        modelSourceStream << ", " << outputVariables.at(j) << "_";
//    modelSourceStream << " }\n\n";
//    modelSourceStream << "    //Set start values\n";
//    modelSourceStream << "    void setStartValues(ModelInstance *comp) \n";
//    modelSourceStream << "    {\n";
//    for(i=0; i<inputVariables.size(); ++i)
//        modelSourceStream << "        r(" << inputVariables.at(i) << "_) = 0;\n";        //!< Fix start value handling
//    for(j=0; j<outputVariables.size(); ++j)
//        modelSourceStream << "        r(" << outputVariables.at(j) << "_) = 0;\n";        //!< Fix start value handling
//    modelSourceStream << "    }\n\n";
//    modelSourceStream << "    //Initialize\n";
//    modelSourceStream << "    void initialize(ModelInstance* comp, fmiEventInfo* eventInfo)\n";
//    modelSourceStream << "    {\n";
//    modelSourceStream << "        initializeHopsanWrapper(\""+realModelName+".hmf\");\n";
//    modelSourceStream << "        eventInfo->upcomingTimeEvent   = fmiTrue;\n";
//    modelSourceStream << "        eventInfo->nextEventTime       = 0.0005 + comp->time;\n";
//    modelSourceStream << "    }\n\n";
//    modelSourceStream << "    //Return variable of real type\n";
//    modelSourceStream << "    fmiReal getReal(ModelInstance* comp, fmiValueReference vr)\n";
//    modelSourceStream << "    {\n";
//    modelSourceStream << "        switch (vr) \n";
//    modelSourceStream << "       {\n";
//    for(i=0; i<inputVariables.size(); ++i)
//        modelSourceStream << "           case " << inputVariables.at(i) << "_: return getVariable(\"" << inputComponents.at(i) << "\", \"" << inputPorts.at(i) << "\", " << inputDatatypes.at(i) << ");\n";
//    for(j=0; j<outputVariables.size(); ++j)
//        modelSourceStream << "           case " << outputVariables.at(j) << "_: return getVariable(\"" << outputComponents.at(j) << "\", \"" << outputPorts.at(j) << "\", " << outputDatatypes.at(j) << ");\n";
//    modelSourceStream << "            default: return 1;\n";
//    modelSourceStream << "       }\n";
//    modelSourceStream << "    }\n\n";
//    modelSourceStream << "    void setReal(ModelInstance* comp, fmiValueReference vr, fmiReal value)\n";
//    modelSourceStream << "    {\n";
//    modelSourceStream << "        switch (vr) \n";
//    modelSourceStream << "       {\n";
//    for(i=0; i<inputVariables.size(); ++i)
//        modelSourceStream << "           case " << inputVariables.at(i) << "_: setVariable(\"" << inputComponents.at(i) << "\", \"" << inputPorts.at(i) << "\", " << inputDatatypes.at(i) << ", value);\n";
//    for(j=0; j<outputVariables.size(); ++j)
//        modelSourceStream << "           case " << outputVariables.at(j) << "_: setVariable(\"" << outputComponents.at(j) << "\", \"" << outputPorts.at(j) << "\", " << outputDatatypes.at(j) << ", value);\n";
//    modelSourceStream << "            default: return;\n";
//    modelSourceStream << "       }\n";
//    modelSourceStream << "    }\n\n";
//    modelSourceStream << "    //Update at time event\n";
//    modelSourceStream << "    void eventUpdate(ModelInstance* comp, fmiEventInfo* eventInfo)\n";
//    modelSourceStream << "    {\n";
//    modelSourceStream << "        simulateOneStep();\n";
//    modelSourceStream << "        eventInfo->upcomingTimeEvent   = fmiTrue;\n";
//    modelSourceStream << "        eventInfo->nextEventTime       = 0.0005 + comp->time;\n";      //!< @todo Hardcoded timestep
//    modelSourceStream << "    }\n\n";
//    modelSourceStream << "    // Include code that implements the FMI based on the above definitions\n";
//    modelSourceStream << "    #include \"fmuTemplate.c\"\n";
//    modelSourceFile.close();


//    //progressBar.setLabelText("Writing HopsanFMU.h");
//    //progressBar.setValue(4);


//    QTextStream fmuHeaderStream(&fmuHeaderFile);
//    QTextLineStream fmuHeaderLines(fmuHeaderStream);
//    fmuHeaderLines << "#ifndef HOPSANFMU_H";
//    fmuHeaderLines << "#define HOPSANFMU_H";
//    fmuHeaderLines << "";
//    fmuHeaderLines << "#ifdef WRAPPERCOMPILATION";
//    //fmuHeaderLines << "    #define DLLEXPORT __declspec(dllexport)";
//    fmuHeaderLines << "    extern \"C\" {";
//    fmuHeaderLines << "#else";
//    fmuHeaderLines << "    #define DLLEXPORT";
//    fmuHeaderLines << "#endif";
//    fmuHeaderLines << "";
//    fmuHeaderLines << "DLLEXPORT void initializeHopsanWrapper(char* filename);";
//    fmuHeaderLines << "DLLEXPORT void simulateOneStep();";
//    fmuHeaderLines << "DLLEXPORT double getVariable(char* component, char* port, size_t idx);";
//    fmuHeaderLines << "";
//    fmuHeaderLines << "DLLEXPORT void setVariable(char* component, char* port, size_t idx, double value);";
//    fmuHeaderLines << "";
//    fmuHeaderLines << "#ifdef WRAPPERCOMPILATION";
//    fmuHeaderLines << "}";
//    fmuHeaderLines << "#endif";
//    fmuHeaderLines << "#endif // HOPSANFMU_H";
//    fmuHeaderFile.close();


//    //progressBar.setLabelText("Writing HopsanFMU.cpp");
//    //progressBar.setValue(5);


//    QTextStream fmuSourceStream(&fmuSourceFile);
//    QTextLineStream fmuSrcLines(fmuSourceStream);

//    fmuSrcLines << "#include <iostream>";
//    fmuSrcLines << "#include <assert.h>";
//    fmuSrcLines << "#include \"HopsanCore.h\"";
//    fmuSrcLines << "#include \"HopsanFMU.h\"";
//    //fmuSrcLines << "#include \"include/ComponentEssentials.h\"";
//    //fmuSrcLines << "#include \"include/ComponentUtilities.h\"";
//    fmuSrcLines << "";
//    fmuSrcLines << "static double fmu_time=0;";
//    fmuSrcLines << "static hopsan::ComponentSystem *spCoreComponentSystem;";
//    fmuSrcLines << "static std::vector<std::string> sComponentNames;";
//    fmuSrcLines << "hopsan::HopsanEssentials gHopsanCore;";
//    fmuSrcLines << "";
//    fmuSrcLines << "void initializeHopsanWrapper(char* filename)";
//    fmuSrcLines << "{";
//    fmuSrcLines << "    double startT;      //Dummy variable";
//    fmuSrcLines << "    double stopT;       //Dummy variable";
//    fmuSrcLines << "    gHopsanCore.loadExternalComponentLib(\"../componentLibraries/defaultLibrary/components/libdefaultComponentLibrary.so\");";
//    fmuSrcLines << "    spCoreComponentSystem = gHopsanCore.loadHMFModel(filename, startT, stopT);\n";
//    fmuSrcLines << "    assert(spCoreComponentSystem);";
//    fmuSrcLines << "    spCoreComponentSystem->setDesiredTimestep(0.001);";           //!< @todo Time step should not be hard coded
//    fmuSrcLines << "    spCoreComponentSystem->initialize(0,10);";
//    fmuSrcLines << "";
//    fmuSrcLines << "    fmu_time = 0;";
//    fmuSrcLines << "}";
//    fmuSrcLines << "";
//    fmuSrcLines << "void simulateOneStep()";
//    fmuSrcLines << "{";
//    fmuSrcLines << "    if(spCoreComponentSystem->checkModelBeforeSimulation())";
//    fmuSrcLines << "    {";
//    fmuSrcLines << "        double timestep = spCoreComponentSystem->getDesiredTimeStep();";
//    fmuSrcLines << "        spCoreComponentSystem->simulate(fmu_time, fmu_time+timestep);";
//    fmuSrcLines << "        fmu_time = fmu_time+timestep;\n";
//    fmuSrcLines << "    }";
//    fmuSrcLines << "    else";
//    fmuSrcLines << "    {";
//    fmuSrcLines << "        std::cout << \"Simulation failed!\";";
//    fmuSrcLines << "    }";
//    fmuSrcLines << "}";
//    fmuSrcLines << "";
//    fmuSrcLines << "double getVariable(char* component, char* port, size_t idx)";
//    fmuSrcLines << "{";
//    fmuSrcLines << "    return spCoreComponentSystem->getSubComponentOrThisIfSysPort(component)->getPort(port)->readNode(idx);";
//    fmuSrcLines << "}";
//    fmuSrcLines << "";
//    fmuSrcLines << "void setVariable(char* component, char* port, size_t idx, double value)";
//    fmuSrcLines << "{";
//    fmuSrcLines << "    assert(spCoreComponentSystem->getSubComponentOrThisIfSysPort(component)->getPort(port) != 0);";
//    fmuSrcLines << "    return spCoreComponentSystem->getSubComponentOrThisIfSysPort(component)->getPort(port)->writeNode(idx, value);";
//    fmuSrcLines << "}";
//    fmuSourceFile.close();

//#ifdef _WIN32
//    //progressBar.setLabelText("Writing to compile.bat");
//    //progressBar.setValue(6);



//    //Write the compilation script file
//    QTextStream clBatchStream(&clBatchFile);
////    if(gccCompiler)
////    {
//        //! @todo Ship Mingw with Hopsan, or check if it exists in system and inform user if it does not.
//    clBatchStream << "g++ -DWRAPPERCOMPILATION -c -Wl,--rpath,'$ORIGIN/.' HopsanFMU.cpp -I./include\n";
//    clBatchStream << "g++ -shared -Wl,--rpath,'$ORIGIN/.' -o HopsanFMU.dll HopsanFMU.o -L./ -lhopsancore";
////    }
////    else
////    {
////        //! @todo Check that Visual Studio is installed, and warn user if not
////        clBatchStream << "echo Compiling Visual Studio libraries...\n";
////        clBatchStream << "if defined VS90COMNTOOLS (call \"%VS90COMNTOOLS%\\vsvars32.bat\") else ^\n";
////        clBatchStream << "if defined VS80COMNTOOLS (call \"%VS80COMNTOOLS%\\vsvars32.bat\")\n";
////        clBatchStream << "cl -LD -nologo -DWIN32 -DWRAPPERCOMPILATION HopsanFMU.cpp /I \\. /I \\include\\HopsanCore.h HopsanCore.lib\n";
////    }
//    clBatchFile.close();
//#endif

//    //progressBar.setLabelText("Copying binary files");
//    //progressBar.setValue(7);


//    //Copy binaries to export directory
//#ifdef _WIN32
//    QFile dllFile;
//    QFile libFile;
//    QFile expFile;
////    if(gccCompiler)
////    {
//        dllFile.setFileName(gDesktopHandler.getExecPath() + "HopsanCore.dll");
//        dllFile.copy(savePath + "/HopsanCore.dll");
////    }
////    else
////    {
////        //! @todo this seem a bit hardcoded
////        dllFile.setFileName(gDesktopHandler.getMSVC2008X86Path() + "HopsanCore.dll");
////        dllFile.copy(savePath + "/HopsanCore.dll");
////        libFile.setFileName(gDesktopHandler.getMSVC2008X86Path() + "HopsanCore.lib");
////        libFile.copy(savePath + "/HopsanCore.lib");
////        expFile.setFileName(gDesktopHandler.getMSVC2008X86Path() + "HopsanCore.exp");
////        expFile.copy(savePath + "/HopsanCore.exp");
////    }
//#elif linux
//    QFile soFile;
//    soFile.setFileName(gDesktopHandler.getExecPath() + "libhopsancore.so");
//    soFile.copy(savePath + "/libhopsancore.so");
//#endif


//    //progressBar.setLabelText("Copying include files");
//    //progressBar.setValue(8);


//    //Copy include files to export directory
//    copyIncludeFilesToDir(savePath);


//    progressBar.setLabelText("Writing "+realModelName+".hmf");
//    progressBar.setValue(9);


//    //Save model to hmf in export directory
//    //! @todo This code is duplicated from ModelWidget::saveModel(), make it a common function somehow
//    QDomDocument domDocument;
//    QDomElement hmfRoot = appendHMFRootElement(domDocument, HMF_VERSIONNUM, HOPSANGUIVERSION, getHopsanCoreVersion());
//    saveToDomElement(hmfRoot);
//    const int IndentSize = 4;
//    QFile xmlhmf(savePath + "/" + mModelFileInfo.fileName());
//    if (!xmlhmf.open(QIODevice::WriteOnly | QIODevice::Text))  //open file
//    {
//        gpMainWindow->mpMessageWidget->printGUIErrorMessage("Unable to open "+savePath+"/"+mModelFileInfo.fileName()+" for writing.");
//        return;
//    }
//    QTextStream out(&xmlhmf);
//    appendRootXMLProcessingInstruction(domDocument); //The xml "comment" on the first line
//    domDocument.save(out, IndentSize);

//    CoreGeneratorAccess *pCoreAccess = new CoreGeneratorAccess();
//    pCoreAccess->generateToFmu(savePath, this);
//    delete(pCoreAccess);

//}


void SystemContainer::exportToSimulink()
{
    QDialog *pExportDialog = new QDialog(gpMainWindowWidget);
    pExportDialog->setWindowTitle("Create Simulink Source Files");

    QLabel *pExportDialogLabel1 = new QLabel(tr("This will create source files for Simulink from the current model. These can be compiled into an S-function library by executing HopsanSimulinkCompile.m from Matlab console."), pExportDialog);
    pExportDialogLabel1->setWordWrap(true);

//    QGroupBox *pCompilerGroupBox = new QGroupBox(tr("Choose compiler:"), pExportDialog);
//    QRadioButton *pMSVC2008RadioButton = new QRadioButton(tr("Microsoft Visual Studio 2008"));
//    QRadioButton *pMSVC2010RadioButton = new QRadioButton(tr("Microsoft Visual Studio 2010"));
//    pMSVC2008RadioButton->setChecked(true);
//    QVBoxLayout *pCompilerLayout = new QVBoxLayout;
//    pCompilerLayout->addWidget(pMSVC2008RadioButton);
//    pCompilerLayout->addWidget(pMSVC2010RadioButton);
//    pCompilerLayout->addStretch(1);
//    pCompilerGroupBox->setLayout(pCompilerLayout);

//    QGroupBox *pArchitectureGroupBox = new QGroupBox(tr("Choose architecture:"), pExportDialog);
//    QRadioButton *p32bitRadioButton = new QRadioButton(tr("32-bit (x86)"));
//    QRadioButton *p64bitRadioButton = new QRadioButton(tr("64-bit (x64)"));
//    p32bitRadioButton->setChecked(true);
//    QVBoxLayout *pArchitectureLayout = new QVBoxLayout;
//    pArchitectureLayout->addWidget(p32bitRadioButton);
//    pArchitectureLayout->addWidget(p64bitRadioButton);
//    pArchitectureLayout->addStretch(1);
//    pArchitectureGroupBox->setLayout(pArchitectureLayout);

//    QLabel *pExportDialogLabel2 = new QLabel("Matlab must use the same compiler during compilation.    ", pExportDialog);

    QCheckBox *pDisablePortLabels = new QCheckBox("Disable port labels (for older versions of Matlab)");

    QDialogButtonBox *pExportButtonBox = new QDialogButtonBox(pExportDialog);
    QPushButton *pExportButtonOk = new QPushButton("Ok", pExportDialog);
    QPushButton *pExportButtonCancel = new QPushButton("Cancel", pExportDialog);
    pExportButtonBox->addButton(pExportButtonOk, QDialogButtonBox::AcceptRole);
    pExportButtonBox->addButton(pExportButtonCancel, QDialogButtonBox::RejectRole);

    QVBoxLayout *pExportDialogLayout = new QVBoxLayout(pExportDialog);
    pExportDialogLayout->addWidget(pExportDialogLabel1);
//    pExportDialogLayout->addWidget(pCompilerGroupBox);
//    pExportDialogLayout->addWidget(pArchitectureGroupBox);
//    pExportDialogLayout->addWidget(pExportDialogLabel2);
    pExportDialogLayout->addWidget(pDisablePortLabels);
    pExportDialogLayout->addWidget(pExportButtonBox);
    pExportDialog->setLayout(pExportDialogLayout);

    connect(pExportButtonBox, SIGNAL(accepted()), pExportDialog, SLOT(accept()));
    connect(pExportButtonBox, SIGNAL(rejected()), pExportDialog, SLOT(reject()));

    //connect(pExportButtonOk,        SIGNAL(clicked()), pExportDialog, SLOT(accept()));
    //connect(pExportButtonCancel,    SIGNAL(clicked()), pExportDialog, SLOT(reject()));

    if(pExportDialog->exec() == QDialog::Rejected)
    {
        return;
    }


    //QMessageBox::information(gpMainWindow, gpMainWindow->tr("Create Simulink Source Files"),
    //                         gpMainWindow->tr("This will create source files for Simulink from the current model. These can be compiled into an S-function library by executing HopsanSimulinkCompile.m from Matlab console.\n\nVisual Studio 2008 compiler is supported, although other versions might work as well.."));

    QString fileName;
    if(!mModelFileInfo.fileName().isEmpty())
    {
        fileName = mModelFileInfo.fileName();
    }
    else
    {
        fileName = "untitled.hmf";
    }


        //Open file dialog and initialize the file stream
    QString savePath;
    savePath = QFileDialog::getExistingDirectory(gpMainWindowWidget, tr("Create Simulink Source Files"),
                                                    gpConfig->getStringSetting(CFG_SIMULINKEXPORTDIR),
                                                    QFileDialog::ShowDirsOnly
                                                    | QFileDialog::DontResolveSymlinks);
    if(savePath.isEmpty()) return;    //Don't save anything if user presses cancel
    QFileInfo file(savePath);
    gpConfig->setStringSetting(CFG_SIMULINKEXPORTDIR, file.absolutePath());

    // Save xml document
    mpModelWidget->saveTo(savePath+"/"+fileName);

//    int compiler;
//    if(pMSVC2008RadioButton->isChecked() && p32bitRadioButton->isChecked())
//    {
//        compiler=0;
//    }
//    else if(pMSVC2008RadioButton->isChecked() && p64bitRadioButton->isChecked())
//    {
//        compiler=1;
//    }
//    else if(pMSVC2010RadioButton->isChecked() && p32bitRadioButton->isChecked())
//    {
//        compiler=2;
//    }
//    else/* if(pMSVC2010RadioButton->isChecked() && p64bitRadioButton->isChecked())*/
//    {
//        compiler=3;
//    }

    auto spGenerator = createDefaultExportGenerator();
    QString modelPath = getModelFileInfo().fileName();
    auto pCoreSystem = mpCoreSystemAccess->getCoreSystemPtr();
    QStringList externalLibraries;
    for (const auto& pLib : gpLibraryHandler->getLibraries(this->getRequiredComponentLibraries(), LibraryTypeEnumT::ExternalLib)) {
        externalLibraries.append(pLib->getLibraryMainFilePath());
    }
    auto portLabels = pDisablePortLabels->isChecked() ? HopsanGeneratorGUI::UsePortlablesT::DisablePortLables :
                                                        HopsanGeneratorGUI::UsePortlablesT::EnablePortLabels;

    if (!spGenerator->generateToSimulink(savePath, modelPath, pCoreSystem, externalLibraries, portLabels ))
    {
        gpMessageHandler->addErrorMessage("Simulink export generator failed");
    }


    //Clean up widgets that do not have a parent
    delete(pDisablePortLabels);
//    delete(pMSVC2008RadioButton);
//    delete(pMSVC2010RadioButton);
//    delete(p32bitRadioButton);
    //    delete(p64bitRadioButton);
}

void SystemContainer::exportToExecutableModel(QString savePath, ArchitectureEnumT arch)
{
    if(savePath.isEmpty())
    {
        //Open file dialog and initialize the file stream
        QDir fileDialogSaveDir;
        savePath = QFileDialog::getExistingDirectory(gpMainWindowWidget, tr("Compile Executable Model"),
                                                        gpConfig->getStringSetting(CFG_EXEEXPORTDIR),
                                                        QFileDialog::ShowDirsOnly
                                                        | QFileDialog::DontResolveSymlinks);
        if(savePath.isEmpty()) return;    //Don't save anything if user presses cancel

        QDir saveDir;
        saveDir.setPath(savePath);
        gpConfig->setStringSetting(CFG_EXEEXPORTDIR, saveDir.absolutePath());
        saveDir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);
        if(!saveDir.entryList().isEmpty())
        {
            qDebug() << saveDir.entryList();
            QMessageBox msgBox;
            msgBox.setWindowIcon(gpMainWindowWidget->windowIcon());
            msgBox.setText(QString("Folder is not empty!"));
            msgBox.setInformativeText("Are you sure you want to export files here?");
            msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            msgBox.setDefaultButton(QMessageBox::No);

            int answer = msgBox.exec();
            if(answer == QMessageBox::No)
            {
                return;
            }
        }
    }

    QDir saveDir(savePath);
    if(!saveDir.exists())
    {
        QDir().mkpath(savePath);
    }
    saveDir.setFilter(QDir::NoFilter);

    if(!mpModelWidget->isSaved())
    {
        QMessageBox::information(gpMainWindowWidget, "tr(Model not saved)", "tr(Please save your model before compiling an executable model)");
        return;
    }

    //Save model to hmf in export directory
    mpModelWidget->saveTo(savePath+"/"+mModelFileInfo.fileName().replace(" ", "_"));

    auto spGenerator = createDefaultExportGenerator();
    spGenerator->setCompilerPath(gpConfig->getCompilerPath(arch));

    HopsanGeneratorGUI::TargetArchitectureT garch;
    if (arch == ArchitectureEnumT::x64)
    {
        garch = HopsanGeneratorGUI::TargetArchitectureT::x64;
    }
    else
    {
        garch = HopsanGeneratorGUI::TargetArchitectureT::x86;
    }
    auto pCoreSystem = mpCoreSystemAccess->getCoreSystemPtr();
    QStringList externalLibraries;
    //! @todo an idea here is to always treat the default library as external, and export it as such (and never build it in by default), that would reduce special handling of the default library
    //! @todo This code prevents nesting an external fmu inside an export, not sure if we need to support this
    spGenerator->setAutoCloseWidgetsOnSuccess(true);
    for (const auto& pLib : gpLibraryHandler->getLibraries(this->getRequiredComponentLibraries(), LibraryTypeEnumT::ExternalLib)) {
        const auto mainFile = pLib->getLibraryMainFilePath();
        spGenerator->checkComponentLibrary(mainFile);
        externalLibraries.append(pLib->getLibraryMainFilePath());
    }
    spGenerator->setAutoCloseWidgetsOnSuccess(false);
    if (!spGenerator->generateToExe(savePath, pCoreSystem, externalLibraries, garch))
    {
        gpMessageHandler->addErrorMessage("Failed to compile executable model");
    }
}


//! @brief Sets the modelfile info from the file representing this system
//! @param[in] rFile The QFile objects representing the file we want to information about
//! @param[in] relModelPath Relative filepath to parent model file (model asset path)
void SystemContainer::setModelFileInfo(QFile &rFile, const QString relModelPath)
{
    mModelFileInfo.setFile(rFile);
    if (!relModelPath.isEmpty())
    {
        getCoreSystemAccessPtr()->setExternalModelFilePath(relModelPath);
    }
}


void SystemContainer::loadParameterFile(const QString &path)
{
    qDebug() << "loadParameterFile()";
    QString parameterFileName = path;
    if(path.isEmpty())
    {
        parameterFileName = QFileDialog::getOpenFileName(gpMainWindowWidget, tr("Load Parameter File"),
                                                             gpConfig->getStringSetting(CFG_LOADMODELDIR),
                                                             tr("Hopsan Parameter Files (*.hpf *.xml)"));
    }

    if(!parameterFileName.isEmpty())
    {
        mpCoreSystemAccess->loadParameterFile(parameterFileName);
        QFileInfo fileInfo = QFileInfo(parameterFileName);
        gpConfig->setStringSetting(CFG_LOADMODELDIR, fileInfo.absolutePath());
    }
}



//! @brief Function to set the time step of the current system
void SystemContainer::setTimeStep(const double timeStep)
{
    mpCoreSystemAccess->setDesiredTimeStep(timeStep);
    this->hasChanged();
}

void SystemContainer::setVisibleIfSignal(bool visible)
{
    if(this->getTypeCQS() == "S")
    {
        this->setVisible(visible);
    }
}

//! @brief Returns the time step value of the current project.
double SystemContainer::getTimeStep()
{
    return mpCoreSystemAccess->getDesiredTimeStep();
}

//! @brief Check if the system inherits timestep from its parent
bool SystemContainer::doesInheritTimeStep()
{
    return mpCoreSystemAccess->doesInheritTimeStep();
}


//! @brief Returns the number of samples value of the current project.
//! @see setNumberOfLogSamples(double)
size_t SystemContainer::getNumberOfLogSamples()
{
    return mNumberOfLogSamples;
}


//! @brief Sets the number of samples value for the current project
//! @see getNumberOfLogSamples()
void SystemContainer::setNumberOfLogSamples(size_t nSamples)
{
    mNumberOfLogSamples = nSamples;
}

double SystemContainer::getLogStartTime() const
{
    return mLogStartTime;
}

void SystemContainer::setLogStartTime(const double logStartT)
{
    mLogStartTime = logStartT;
}


OptimizationSettings::OptimizationSettings()
{
    //Defaulf values
    mScriptFile = QString();
    mNiter=100;
    mNsearchp=8;
    mRefcoeff=1.3;
    mRandfac=.3;
    mForgfac=0.0;
    mPartol=.0001;
    mPlot=true;
    mSavecsv=false;
    mFinalEval=true;
    mlogPar = false;
}


SensitivityAnalysisSettings::SensitivityAnalysisSettings()
{
    nIter = 100;
    distribution = UniformDistribution;
}
