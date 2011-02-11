//!
//! @file   loadObjects.cpp
//! @author Flumes <flumes@lists.iei.liu.se>
//! @date   2010-01-01
//!
//! @brief Contains classes and functions used to recreate models from load data
//!
//$Id$

#include "loadObjects.h"

#include "GUIObjects/GUIModelObject.h"
#include "GUIObjects/GUIContainerObject.h"
#include "GUIObjects/GUISystem.h"

#include "GUIConnector.h"
#include "GUIPort.h"

#include "Widgets/LibraryWidget.h"

#include "MainWindow.h"
#include "UndoStack.h"

#include <QMap>


//! @brief Reads the ModelObject load data from an XML DOM element
//! @param[in] rDomElement The DOM element to read from
void ModelObjectLoadData::readDomElement(QDomElement &rDomElement)
{
    //Read core specific data
    type = rDomElement.attribute(HMF_TYPETAG);
    name = rDomElement.attribute(HMF_NAMETAG);

    //Read gui specific data
    readGuiDataFromDomElement(rDomElement);
}

//! @brief Reads the GUI specific ModelObject load data from an XML DOM element
//! @param[in] rDomElement The DOM element to read from
void ModelObjectLoadData::readGuiDataFromDomElement(QDomElement &rDomElement)
{
    QDomElement guiData = rDomElement.firstChildElement(HMF_HOPSANGUITAG);
    parsePoseTag(guiData.firstChildElement(HMF_POSETAG), posX, posY, rotation, isFlipped);
    nameTextPos = guiData.firstChildElement(HMF_NAMETEXTTAG).attribute("position").toInt();
    textVisible = guiData.firstChildElement(HMF_NAMETEXTTAG).attribute("visible").toInt(); //should be bool, +0.5 to roound to int on truncation
    portsHidden = guiData.firstChildElement(HMF_PORTSTAG).attribute("hidden").toInt();
    namesHidden = guiData.firstChildElement(HMF_NAMESTAG).attribute("hidden").toInt();

    QDomElement defaultParameterTag = guiData.firstChildElement(HMF_DEFAULTPARAMETERTAG);
    while(!defaultParameterTag.isNull())
    {
        defaultParameterMap.insert(defaultParameterTag.attribute("name"), defaultParameterTag.attribute("value").toDouble());
        defaultParameterTag = defaultParameterTag.nextSiblingElement(HMF_DEFAULTPARAMETERTAG);
    }

}

//! @brief Reads the System load data from an XML DOM element
//! @param[in] rDomElement The DOM element to read from
void SystemLoadData::readDomElement(QDomElement &rDomElement)
{
    //Read default ModelObjectStuff, core adn gui data
    ModelObjectLoadData::readDomElement(rDomElement);

    //Overwrite the typename with the gui specific one for systems, or set the type if it is missing (which is should be)
    //! @todo or maybe core should contain system typename for systems
    type = HOPSANGUISYSTEMTYPENAME;

    //Read system specific corea nd gui data
    externalfilepath = rDomElement.attribute(HMF_EXTERNALPATHTAG);

    //Save the domElement to read embeded system
    if (externalfilepath.isEmpty())
    {
        embededSystemDomElement = rDomElement;
    }
}

//! @brief Reads the Connector load data from an XML DOM element
//! @param[in] rDomElement The DOM element to read from
void ConnectorLoadData::readDomElement(QDomElement &rDomElement)
{
    //Read core specific stuff
    startComponentName = rDomElement.attribute(HMF_CONNECTORSTARTCOMPONENTTAG);
    startPortName = rDomElement.attribute(HMF_CONNECTORSTARTPORTTAG);
    endComponentName = rDomElement.attribute(HMF_CONNECTORENDCOMPONENTTAG);
    endPortName = rDomElement.attribute(HMF_CONNECTORENDPORTTAG);

    //Read gui specific stuff
    qreal x,y;
    QDomElement guiData = rDomElement.firstChildElement(HMF_HOPSANGUITAG);
    QDomElement guiCoordinates = guiData.firstChildElement(HMF_COORDINATES);
    QDomElement coordTag = guiCoordinates.firstChildElement(HMF_COORDINATETAG);
    while (!coordTag.isNull())
    {
        parseCoordinateTag(coordTag, x, y);
        pointVector.push_back(QPointF(x,y));
        coordTag = coordTag.nextSiblingElement(HMF_COORDINATETAG);
    }
    QDomElement guiGeometries = guiData.firstChildElement(HMF_GEOMETRIES);
    QDomElement geometryTag = guiGeometries.firstChildElement(HMF_GEOMETRYTAG);
    while (!geometryTag.isNull())
    {
        geometryList.append(geometryTag.text());
        geometryTag = geometryTag.nextSiblingElement(HMF_GEOMETRYTAG);
    }
}

//! @brief Reads the Parameter load data from an XML DOM element
//! @param[in] rDomElement The DOM element to read from
void ParameterLoadData::readDomElement(QDomElement &rDomElement)
{
    parameterName = rDomElement.attribute(HMF_NAMETAG);
    parameterValue = rDomElement.attribute(HMF_VALUETAG);
}

//! @brief Reads the StartValue load data from an XML DOM element
//! @param[in] rDomElement The DOM element to read from
void StartValueLoadData::readDomElement(QDomElement &rDomElement)
{
    portName = rDomElement.attribute("portname");
    variable = rDomElement.attribute("variable");
    startValue = rDomElement.attribute("value");
}


//! @brief Loads a System Object from the supplied load data
//! @param[in] rData The SystemLoadData to load from
//! @param[in] pLibrary a pointer to the library widget which holds appearance data
//! @param[in] pContainer The Container Object to load into
//! @param[in] undoSettings Wheter or not to register undo for the operation
GUIModelObject* loadGUISystemObject(SystemLoadData &rData, LibraryWidget* pLibrary, GUIContainerObject* pContainer, undoStatus undoSettings)
{
    //! @todo maybe create a loadGUIObject function that takes appearance data instead of pLibrary (when special apperance are to be used)
    //Load the system the normal way (and add it)
    GUIModelObject* pSys = loadGUIModelObject(rData, pLibrary, pContainer, undoSettings);

    //Check if we should load a embeded or external system
    if (rData.externalfilepath.isEmpty())
    {
        //Load embeded system
        pSys->loadFromDomElement(rData.embededSystemDomElement);
    }
    else
    {
        //Now read the external file to change appearance and populate the system
        //! @todo assumes that the supplied path is rellative, need to make sure that this does not crash if that is not the case
        QString path = pContainer->mModelFileInfo.absolutePath() + "/" + rData.externalfilepath;
        QFile file(path);
        if (!(file.exists()))
        {
            qDebug() << "file: " << path << " does not exist";
        }
        QDomDocument domDocument;
        QDomElement externalRoot = loadXMLDomDocument(file, domDocument, HMF_ROOTTAG);
        QDomElement externalSystemRoot = externalRoot.firstChildElement(HMF_SYSTEMTAG);
        //! @todo set the modefile info, maybe we should have built in helpfunction for loading directly from file in System
        pSys->setModelFileInfo(file);
        pSys->loadFromDomElement(externalSystemRoot);
        //! @todo this code is duplicated with the one in system->loadfromdomelement (external code) that code will never run, as this will take care of it. When we have embeded subsystems will will need to fix this

        //Overwrite any loaded external name with the one that was stored in the main file from which we are loading
        if (!rData.name.isEmpty())
        {
            pSys->setName(rData.name);
        }

    }

    return pSys;
}


//! @brief Loads a Connector from the supplied load data
//! @param[in] rData The ConnectorLoadData to load from
//! @param[in] pContainer The Container Object to load into
//! @param[in] undoSettings Wheter or not to register undo for the operation
void loadConnector(const ConnectorLoadData &rData, GUIContainerObject* pContainer, undoStatus undoSettings)
{
    //qDebug() << "loadConnector: " << rData.startComponentName << " " << rData.endComponentName << " " << pSystem->getCoreSystemAccessPtr()->getRootSystemName();
    bool success = pContainer->getCoreSystemAccessPtr()->connect(rData.startComponentName, rData.startPortName, rData.endComponentName, rData.endPortName);
    if (success)
    {
        //! @todo all of this (above and bellow) should be inside some conventiant function like "connect"
        //! @todo Need some error handling here to avoid crash if components or ports do not exist
        GUIPort *startPort = pContainer->getGUIModelObject(rData.startComponentName)->getPort(rData.startPortName);
        GUIPort *endPort = pContainer->getGUIModelObject(rData.endComponentName)->getPort(rData.endPortName);

        GUIConnector *pTempConnector = new GUIConnector(startPort, endPort, rData.pointVector, pContainer, rData.geometryList);
        pContainer->getContainedScenePtr()->addItem(pTempConnector);

        //Hide connected ports
        startPort->hide();
        endPort->hide();

        QObject::connect(startPort->getGuiModelObject(),SIGNAL(objectDeleted()),pTempConnector,SLOT(deleteMeWithNoUndo()));
        QObject::connect(endPort->getGuiModelObject(),SIGNAL(objectDeleted()),pTempConnector,SLOT(deleteMeWithNoUndo()));

        pContainer->mSubConnectorList.append(pTempConnector);

        if(undoSettings == UNDO)
        {
            pContainer->mUndoStack->registerAddedConnector(pTempConnector);
        }
    }
    else
    {
        qDebug() << "Unsuccessful connection atempt" << endl;
    }
}

//! @brief Loads a SystemParameter from the supplied load data
//! @param[in] rData The SystemParameterLoadData to load from
//! @param[in] pContainer The Container Object to load into
void loadSystemParameter(const SystemParameterLoadData &rData, GUIContainerObject* pContainer)
{
    pContainer->getCoreSystemAccessPtr()->setSystemParameter(rData.name, rData.value);
}


//! @brief Loads a FavouriteParameter from the supplied load data
//! @param[in] rData The FavoriteParameterLoadData to load from
//! @param[in] pContainer The Container Object to load into (Must be a system)
void loadFavoriteParameter(const FavoriteParameterLoadData &rData, GUIContainerObject *pContainer)
{
    //! @todo is FAvouriteParameter suposted to be favourite plot varibales or something? rename in such case,
    //! @todo why do we need to make sure that a plotwidget is created every where
    gpMainWindow->makeSurePlotWidgetIsCreated();
    dynamic_cast<GUISystem *>(pContainer)->setFavoriteParameter(rData.componentName, rData.portName, rData.dataName, rData.dataUnit);
}


//! @brief xml version
void loadParameterValue(const ParameterLoadData &rData, GUIModelObject* pObject, undoStatus undoSettings)
{
//    bool isDbl;
//    //Assumes that if it is convertible to a double it is a plain value otherwise it is assumed to be mapped to a System parameter
//    double value = rData.parameterValue.toDouble(&isDbl);
//    if(isDbl)
//    {
//        pObject->setParameterValue(rData.parameterName, value);
//    }
//    else
    {
        //Use the setParameter method that mapps to System parameter
        pObject->setParameterValue(rData.parameterName, rData.parameterValue);
    }
}

//! @brief xml version
void loadParameterValue(QDomElement &rDomElement, GUIModelObject* pObject, undoStatus undoSettings)
{
    ParameterLoadData data;
    data.readDomElement(rDomElement);
    loadParameterValue(data, pObject, undoSettings);
}


//! @brief xml version
void loadStartValue(const StartValueLoadData &rData, GUIModelObject* pObject, undoStatus undoSettings)
{
//    bool isDbl;
//    //Assumes that if it is convertible to a double it is a plain value otherwise it is assumed to be mapped to a System parameter
//    double value = rData.startValue.toDouble(&isDbl);
//    if(isDbl)
//    {
//        pObject->setStartValue(rData.portName, rData.variable, value);
//    }
//    else
    {
        //Use the setStartValue method that mapps to System parameter
        pObject->setStartValue(rData.portName, rData.variable, rData.startValue);
    }
}

//! @brief xml version
void loadStartValue(QDomElement &rDomElement, GUIModelObject* pObject, undoStatus undoSettings)
{
    StartValueLoadData data;
    data.readDomElement(rDomElement);
    loadStartValue(data, pObject, undoSettings);
}


//! @brief Loads a ModelObject from the supplied load data
//! @param[in] rData The ModelObjectLoadData to load from
//! @param[in] pLibrary a pointer to the library widget which holds appearance data
//! @param[in] pContainer The Container Object to load into
//! @param[in] undoSettings Wheter or not to register undo for the operation
GUIModelObject* loadGUIModelObject(const ModelObjectLoadData &rData, LibraryWidget* pLibrary, GUIContainerObject* pContainer, undoStatus undoSettings)
{
    GUIModelObjectAppearance *pAppearanceData = pLibrary->getAppearanceData(rData.type);
    if (pAppearanceData != 0)
    {
        GUIModelObjectAppearance appearanceData = *pAppearanceData; //Make a copy
        appearanceData.setName(rData.name);

        nameVisibility nameStatus;
        if(rData.textVisible)
        {
           nameStatus = NAMEVISIBLE;
        }
        else
        {
            nameStatus = NAMENOTVISIBLE;
        }

        GUIModelObject* pObj = pContainer->addGUIModelObject(&appearanceData, QPoint(rData.posX, rData.posY), 0, DESELECTED, nameStatus, undoSettings);
        pObj->setNameTextPos(rData.nameTextPos);

        QMap<QString, double>::iterator it;
        QMap<QString, double> map = QMap<QString, double>(rData.defaultParameterMap);
        for(it=map.begin(); it!=map.end(); ++it)
        {
            pObj->mDefaultParameters.insert(it.key(), it.value());
        }

        if (rData.isFlipped)
        {
            pObj->flipHorizontal(undoSettings);
        }
        while(pObj->rotation() != rData.rotation)
        {
            pObj->rotate90cw(undoSettings);
        }
        return pObj;
    }
    else
    {
        qDebug() << "loadGUIObj Some errer happend pAppearanceData == 0";
        //! @todo Some error message
        return 0;
    }
}


//! @brief Conveniance function if you dont want to manipulate the loaded data
GUIModelObject* loadGUIModelObject(QDomElement &rDomElement, LibraryWidget* pLibrary, GUIContainerObject* pSystem, undoStatus undoSettings)
{
    ModelObjectLoadData data;
    data.readDomElement(rDomElement);
    return loadGUIModelObject(data, pLibrary, pSystem, undoSettings);
}

GUIModelObject* loadGUISystemObject(QDomElement &rDomElement, LibraryWidget* pLibrary, GUIContainerObject* pSystem, undoStatus undoSettings)
{
    SystemLoadData data;
    data.readDomElement(rDomElement);
    return loadGUISystemObject(data, pLibrary, pSystem, undoSettings);
}

//! @brief Loads a containerport object from a xml dom element
GUIModelObject* loadContainerPortObject(QDomElement &rDomElement, LibraryWidget* pLibrary, GUIContainerObject* pSystem, undoStatus undoSettings)
{
    ModelObjectLoadData data;
    data.readDomElement(rDomElement);
    data.type = HOPSANGUICONTAINERPORTTYPENAME; //Set the typename for the gui, or overwrite if anything was actaully given in the HMF file (should not be)
    return loadGUIModelObject(data, pLibrary, pSystem, undoSettings); //We use the loadGUIModelObject function as it does what is needed
}

//! @brief Conveniance function if you dont want to manipulate the loaded data
void loadConnector(QDomElement &rDomElement, GUIContainerObject* pSystem, undoStatus undoSettings)
{
    ConnectorLoadData data;
    data.readDomElement(rDomElement);
    loadConnector(data, pSystem, undoSettings);
}

//! @brief Conveniance function if you dont want to manipulate the loaded data
void loadSystemParameter(QDomElement &rDomElement, GUIContainerObject* pSystem)
{
    SystemParameterLoadData data;
    data.readDomElement(rDomElement);
    loadSystemParameter(data, pSystem);
}


void loadFavoriteParameter(QDomElement &rDomElement, GUIContainerObject* pSystem)
{
    FavoriteParameterLoadData data;
    data.readDomElement(rDomElement);
    loadFavoriteParameter(data, pSystem);
}


void TextWidgetLoadData::readDomElement(QDomElement &rDomElement)
{
    //Read gui specific stuff
    QDomElement guiData = rDomElement.firstChildElement(HMF_HOPSANGUITAG);

    if(!guiData.isNull())
    {
        QDomElement textobjectTag = guiData.firstChildElement("textobject");
        text = textobjectTag.attribute("text");
        font.fromString(textobjectTag.attribute("font"));
        fontcolor.setNamedColor(textobjectTag.attribute("fontcolor"));

        QDomElement poseTag = guiData.firstChildElement(HMF_POSETAG);
        point.setX(poseTag.attribute("x").toInt());
        point.setY(poseTag.attribute("y").toInt());
    }
}

void loadTextWidget(QDomElement &rDomElement, GUIContainerObject *pSystem, undoStatus undoSettings)
{
    qDebug() << "1";
    TextWidgetLoadData data;
    data.readDomElement(rDomElement);
    qDebug() << "2";
    pSystem->addTextWidget(data.point, NOUNDO);
    pSystem->mTextWidgetList.last()->setText(data.text);
    pSystem->mTextWidgetList.last()->setTextFont(data.font);
    pSystem->mTextWidgetList.last()->setTextColor(data.fontcolor);
    qDebug() << "3";
    if(undoSettings == UNDO)
    {
        pSystem->mUndoStack->registerAddedBoxWidget(pSystem->mBoxWidgetList.last());
    }
    qDebug() << "4";
}


void BoxWidgetLoadData::readDomElement(QDomElement &rDomElement)
{
    //Read gui specific stuff
    QDomElement guiData = rDomElement.firstChildElement(HMF_HOPSANGUITAG);

    if(!guiData.isNull())
    {
        QDomElement sizeTag = guiData.firstChildElement("size");
        width = sizeTag.attribute("width").toDouble();
        height = sizeTag.attribute("height").toDouble();

        QDomElement lineTag = guiData.firstChildElement("line");
        linewidth = lineTag.attribute("width").toDouble();
        linestyle = lineTag.attribute("style");
        linecolor.setNamedColor(lineTag.attribute("color"));

        QDomElement poseTag = guiData.firstChildElement(HMF_POSETAG);
        point.setX(poseTag.attribute("x").toInt());
        point.setY(poseTag.attribute("y").toInt());
    }
}


void SystemParameterLoadData::readDomElement(QDomElement &rDomElement)
{
    name = rDomElement.attribute("name");
    value = rDomElement.attribute("value").toDouble();
}


void FavoriteParameterLoadData::readDomElement(QDomElement &rDomElement)
{
    componentName = rDomElement.attribute("componentname");
    portName = rDomElement.attribute("portname"),
    dataName = rDomElement.attribute("dataname");
    dataUnit = rDomElement.attribute("dataunit");
}


//! @brief Convenience function for loading a box widget from a dom element
void loadBoxWidget(QDomElement &rDomElement, GUIContainerObject *pSystem, undoStatus undoSettings)
{
    BoxWidgetLoadData data;
    data.readDomElement(rDomElement);

    pSystem->addBoxWidget(data.point, NOUNDO);
    pSystem->mBoxWidgetList.last()->setSize(data.width, data.height);
    pSystem->mBoxWidgetList.last()->setLineWidth(data.linewidth);

    if(data.linestyle == "solidline")
        pSystem->mBoxWidgetList.last()->setLineStyle(Qt::SolidLine);
    if(data.linestyle == "dashline")
        pSystem->mBoxWidgetList.last()->setLineStyle(Qt::DashLine);
    if(data.linestyle == "dotline")
        pSystem->mBoxWidgetList.last()->setLineStyle(Qt::DotLine);
    if(data.linestyle == "dashdotline")
        pSystem->mBoxWidgetList.last()->setLineStyle(Qt::DashDotLine);

    pSystem->mBoxWidgetList.last()->setLineColor(data.linecolor);
    pSystem->mBoxWidgetList.last()->setSelected(true);
    pSystem->mBoxWidgetList.last()->setSelected(false);     //For some reason this is needed

    if(undoSettings == UNDO)
    {
        pSystem->mUndoStack->registerAddedBoxWidget(pSystem->mBoxWidgetList.last());
    }
}
