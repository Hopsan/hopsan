#include "loadObjects.h"

#include <QObject> //!< @todo maybe not this one for connect()

#include "Widgets/LibraryWidget.h"
#include "GraphicsView.h"
#include "CoreAccess.h"
#include "GUIConnector.h"
#include "GUIPort.h"
#include "Widgets/MessageWidget.h"
#include "version.h"
#include "GUIObjects/GUISystem.h"
#include "Utilities/GUIUtilities.h"
#include "UndoStack.h"

void HeaderLoadData::read(QTextStream &rStream)
{
    QString inputWord;

    //Read and discard title block
    rStream.readLine();
    rStream.readLine();
    rStream.readLine();

    //Read the three version numbers
    for (int i=0; i<3; ++i)
    {
        rStream >> inputWord;

        if ( inputWord == "HOPSANGUIVERSION")
        {
            rStream >> hopsangui_version;
        }
        else if ( inputWord == "HMFVERSION")
        {
            rStream >> hmf_version;
        }
        else if ( inputWord == "CAFVERSION") //CAF = Component Appearance File
        {
            rStream >> caf_version;
        }
        else
        {
            //! @todo handle errors
            //pMessageWidget->printGUIErrorMessage(QString("Error: unknown HEADER command: " + inputWord));
        }
    }

    //Remove end line and dashed line
    rStream.readLine();
    rStream.readLine();

    //Read Simulation time
    rStream >> inputWord;
    if (inputWord == "SIMULATIONTIME")
    {
        rStream >> startTime >> timeStep >> stopTime;
    }
    else
    {
        qDebug() << QString("ERROR SIMULATIONTIME EXPECTED, got: ") + inputWord;
        //! @todo handle errors
    }

    //Read viewport
    rStream >> inputWord;
    if (inputWord == "VIEWPORT")
    {
        rStream >> viewport_x >> viewport_y >> viewport_zoomfactor;
    }
    else
    {
        qDebug() << QString("ERROR VIEWPORT EXPECTED, got") + inputWord;
        //! @todo handle errors
    }

    //Remove newline and dashed ending line
    rStream.readLine();
    rStream.readLine();
}

void ModelObjectLoadData::read(QTextStream &rStream)
{
    type = readName(rStream);
    name = readName(rStream);  //Now read the name, assume that the name is contained within quotes signs, "name"
    rStream >> posX;
    rStream >> posY;

    //! @todo if not end of stream do this, to allow incomplete load_data
    rStream >> rotation;
    rStream >> nameTextPos;
    rStream >> textVisible;

    isFlipped = false; //default to false
}

void ModelObjectLoadData::readDomElement(QDomElement &rDomElement)
{
    //Read core specific data
    type = rDomElement.attribute(HMF_TYPETAG);
    name = rDomElement.attribute(HMF_NAMETAG);

    //Read gui specific data
    readGuiDataFromDomElement(rDomElement);
}

void ModelObjectLoadData::readGuiDataFromDomElement(QDomElement &rDomElement)
{
    QDomElement guiData = rDomElement.firstChildElement(HMF_HOPSANGUITAG);
    parsePoseTag(guiData.firstChildElement(HMF_POSETAG), posX, posY, rotation, isFlipped);
    nameTextPos = guiData.firstChildElement(HMF_NAMETEXTTAG).attribute("position").toInt();
    textVisible = guiData.firstChildElement(HMF_NAMETEXTTAG).attribute("visible").toInt(); //should be bool, +0.5 to roound to int on truncation
}

void SubsystemLoadData::read(QTextStream &rStream)
{
    type = "Subsystem";
    loadtype = readName(rStream);
    name = readName(rStream);
    cqs_type = readName(rStream);

    if (loadtype == "EXTERNAL")
    {
        externalfilepath = readName(rStream);

        //Read the gui stuff
        rStream >> posX;
        rStream >> posY;

        //! @todo if not end of stream do this, to allow incomplete load_data
        rStream >> rotation;
        rStream >> nameTextPos;
        rStream >> textVisible;
    }
//    else if (loadtype == "embeded")
//    {
//        //not implemented yet
//        //! @todo handle error
//        assert(false);
//    }
    else
    {
        //incorrect type
        qDebug() << QString("This loadtype is not supported: ") + loadtype;
        //! @todo handle error
    }
}

void SubsystemLoadData::readDomElement(QDomElement &rDomElement)
{
    type = "Subsystem"; //Hardcode the type, regardles of hmf contents (should not contain type
//    name = rDomElement.attribute(HMF_NAMETAG);
//    cqs_type = rDomElement.attribute(HMF_CQSTYPETAG);
    externalfilepath = rDomElement.attribute(HMF_EXTERNALPATHTAG);

    //Read gui specific data
    this->readGuiDataFromDomElement(rDomElement);
}

//! @brief Reads system appearnce data from stream
//! Assumes that this data ends when commandword has - as first sign
//! Header must have been removed (read) first
void SystemAppearanceLoadData::read(QTextStream &rStream)
{
    QString commandword;

    while ( !commandword.startsWith("-") )
    {
        rStream >> commandword;
        //qDebug() << commandword;

        //! @todo maybe do this the same way as read apperance data, will examine this later
        if (commandword == "ISOICON")
        {
            isoicon_path = readName(rStream);
        }
        else if (commandword == "USERICON")
        {
            usericon_path = readName(rStream);
        }
        else if (commandword == "PORT")
        {
            QString name = readName(rStream);
            qreal x,y,th;
            rStream >> x >> y >> th;

            portnames.append(name);
            port_xpos.append(x);
            port_ypos.append(y);
            port_angle.append(th);
        }
    }
}


void ConnectorLoadData::read(QTextStream &rStream)
{
    startComponentName = readName(rStream);
    startPortName = readName(rStream);
    endComponentName = readName(rStream);
    endPortName = readName(rStream);

    qreal tempX, tempY;
    QString restOfLineString = rStream.readLine();
    QTextStream restOfLineStream(&restOfLineString);
    while( !restOfLineStream.atEnd() )
    {
        restOfLineStream >> tempX;
        restOfLineStream >> tempY;
        pointVector.push_back(QPointF(tempX, tempY));
    }
}

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


void ParameterLoadData::read(QTextStream &rStream)
{
    componentName = readName(rStream);
    parameterName = readName(rStream);
    rStream >> parameterValue;
}

void ParameterLoadData::readDomElement(QDomElement &rDomElement)
{
    parameterName = rDomElement.attribute(HMF_NAMETAG);
    parameterValue = rDomElement.attribute(HMF_VALUETAG).toDouble();
    parameterGlobalKey = rDomElement.attribute(HMF_SystemParameterTAG);
}

void StartValueLoadData::readDomElement(QDomElement &rDomElement)
{
    portName = rDomElement.attribute("portname");
    variable = rDomElement.attribute("variable");
    startValue = rDomElement.attribute("value").toDouble();
}


GUIObject* loadSubsystemGUIObject(const SubsystemLoadData &rData, LibraryWidget* pLibrary, GUIContainerObject* pSystem, undoStatus undoSettings)
{
    //! @todo We need to be able to save a custom display name for external subsystems that overwrite the name loaded from the external system

    //! @todo can only handle external subsystems for now

    //! @todo maybe create a loadGUIObject function that takes appearance data instead of pLibrary (when special apperance are to be used)
    //Load the system the normal way (and add it)
    GUIModelObject* pSys = loadGUIModelObject(rData, pLibrary, pSystem, undoSettings);

    //Now read the external file to change appearance and populate the system
    //! @todo assumes that the supplied path is rellative, need to make sure that this does not crash if that is not the case
    //! @todo what if the parent system does not have a path (embeded systems)
    QString path = pSystem->mModelFileInfo.absolutePath() + "/" + rData.externalfilepath;
    QFile file(path);
    if (!(file.exists()));
    {
        qDebug() << "file: " << path << " does not exist";
    }
    QDomDocument domDocument;
    QDomElement externalRoot = loadXMLDomDocument(file, domDocument, HMF_ROOTTAG);
    QDomElement systemRoot = externalRoot.firstChildElement(HMF_SYSTEMTAG);
    //! @todo set the modefile info, maybe we should have built in helpfunction for loading directly from file in System
    pSys->setModelFileInfo(file);
    pSys->loadFromDomElement(systemRoot);
    //! @todo this code is duplicated with the one in system->loadfromdomelement (external code) that code will never run, as this will take care of it. When we have embeded subsystems will will need to fix this

    //Set the cqs type of the system
    //pSys->setTypeCQS(rData.cqs_type); //!< @todo is this necessary isnt cqs set insed loadfromdoelement

    return pSys;
}



void loadConnector(const ConnectorLoadData &rData, GUIContainerObject* pSystem, undoStatus undoSettings)
{
    //qDebug() << "loadConnector: " << rData.startComponentName << " " << rData.endComponentName << " " << pSystem->getCoreSystemAccessPtr()->getRootSystemName();
    bool success = pSystem->getCoreSystemAccessPtr()->connect(rData.startComponentName, rData.startPortName, rData.endComponentName, rData.endPortName);
    if (success)
    {
//        //Check if the component names are the same as the guiroot system name in such cases we should search for the actual systemport gui object instead
//        //!< @todo this is extremely strange, some day we need to figure out a way that allways works the same way, this will likly mean MAJOR changes
//        QString startGuiObjName, endGuiObjName;
//        if (rData.startComponentName == pSystem->getCoreSystemAccessPtr()->getRootSystemName())
//        {
//            startGuiObjName = rData.startPortName;
//        }
//        else
//        {
//            startGuiObjName = rData.startComponentName;
//        }
//        if (rData.endComponentName == pSystem->getCoreSystemAccessPtr()->getRootSystemName())
//        {
//            endGuiObjName = rData.endPortName;
//        }
//        else
//        {
//            endGuiObjName = rData.endComponentName;
//        }

        //! @todo all of this (above and bellow) should be inside some conventiant function like "connect"
        //! @todo Need some error handling here to avoid crash if components or ports do not exist
        GUIPort *startPort = pSystem->getGUIModelObject(rData.startComponentName)->getPort(rData.startPortName);
        GUIPort *endPort = pSystem->getGUIModelObject(rData.endComponentName)->getPort(rData.endPortName);

        GUIConnector *pTempConnector = new GUIConnector(startPort, endPort, rData.pointVector, pSystem, rData.geometryList);
        pSystem->getContainedScenePtr()->addItem(pTempConnector);

        //Hide connected ports
        startPort->hide();
        endPort->hide();

        QObject::connect(startPort->getGuiModelObject(),SIGNAL(objectDeleted()),pTempConnector,SLOT(deleteMeWithNoUndo()));
        QObject::connect(endPort->getGuiModelObject(),SIGNAL(objectDeleted()),pTempConnector,SLOT(deleteMeWithNoUndo()));

        pSystem->mSubConnectorList.append(pTempConnector);

        if(undoSettings == UNDO)
        {
            pSystem->mUndoStack->registerAddedConnector(pTempConnector);
        }
    }
    else
    {
        qDebug() << "Unsuccessful connection atempt" << endl;
    }
}


void loadSystemParameter(const SystemParameterLoadData &rData, GUIContainerObject* pSystem)
{
    pSystem->getCoreSystemAccessPtr()->setSystemParameter(rData.name, rData.value);
}


//! @brief text version
void loadParameterValues(const ParameterLoadData &rData, GUIContainerObject* pSystem, undoStatus undoSettings)
{
    //qDebug() << "Parameter: " << componentName << " " << parameterName << " " << parameterValue;
    //qDebug() << "count" << pSystem->mGUIObjectMap.count(rData.componentName);
    qDebug() << "load Parameter value for component: " << rData.componentName  << " in " << pSystem->getName();
    //qDebug() << "Parameter: " << rData.parameterName << " " << rData.parameterValue;
    GUIModelObject* ptr = pSystem->mGUIModelObjectMap.find(rData.componentName).value();
    qDebug() << ptr->getName();
    if (ptr != 0)
        ptr->setParameterValue(rData.parameterName, rData.parameterValue);
    else
        assert(false);
}

//! @brief xml version
void loadParameterValue(const ParameterLoadData &rData, GUIModelObject* pObject, undoStatus undoSettings)
{
    if(rData.parameterGlobalKey.isEmpty())
    {
        pObject->setParameterValue(rData.parameterName, rData.parameterValue);
    }
    else
    {
        pObject->mapParameterToSystemParameter(rData.parameterName, rData.parameterGlobalKey);
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
    pObject->setStartValue(rData.portName, rData.variable, rData.startValue);
}

//! @brief xml version
void loadStartValue(QDomElement &rDomElement, GUIModelObject* pObject, undoStatus undoSettings)
{
    StartValueLoadData data;
    data.readDomElement(rDomElement);
    loadStartValue(data, pObject, undoSettings);
}


GUIModelObject* loadGUIModelObject(const ModelObjectLoadData &rData, LibraryWidget* pLibrary, GUIContainerObject* pSystem, undoStatus undoSettings)
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

        GUIModelObject* pObj = pSystem->addGUIModelObject(&appearanceData, QPoint(rData.posX, rData.posY), 0, DESELECTED, nameStatus, undoSettings);
        pObj->setNameTextPos(rData.nameTextPos);

        if (rData.isFlipped)
        {
            pObj->flipHorizontal(undoSettings);
        }
        while(pObj->rotation() != rData.rotation)
        {
            pObj->rotate(undoSettings);
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
GUIModelObject* loadGUIModelObject(QTextStream &rStream, LibraryWidget* pLibrary, GUIContainerObject* pSystem, undoStatus undoSettings)
{
    ModelObjectLoadData data;
    data.read(rStream);
    return loadGUIModelObject(data, pLibrary, pSystem, undoSettings);
}

//! @brief Conveniance function if you dont want to manipulate the loaded data
GUIModelObject* loadGUIModelObject(QDomElement &rDomElement, LibraryWidget* pLibrary, GUIContainerObject* pSystem, undoStatus undoSettings)
{
    ModelObjectLoadData data;
    data.readDomElement(rDomElement);
    return loadGUIModelObject(data, pLibrary, pSystem, undoSettings);
}

GUIObject* loadSubsystemGUIObject(QTextStream &rStream, LibraryWidget* pLibrary, GUIContainerObject* pSystem, undoStatus undoSettings)
{
    SubsystemLoadData data;
    data.read(rStream);
    return loadSubsystemGUIObject(data, pLibrary, pSystem, undoSettings);
}

GUIObject* loadSubsystemGUIObject(QDomElement &rDomElement, LibraryWidget* pLibrary, GUIContainerObject* pSystem, undoStatus undoSettings)
{
    SubsystemLoadData data;
    data.readDomElement(rDomElement);
    return loadSubsystemGUIObject(data, pLibrary, pSystem, undoSettings);
}

//! @brief Conveniance function if you dont want to manipulate the loaded data
void loadConnector(QTextStream &rStream, GUIContainerObject* pSystem, undoStatus undoSettings)
{
    ConnectorLoadData data;
    data.read(rStream);
    loadConnector(data, pSystem, undoSettings);
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

void loadTextWidget(QDomElement &rDomElement, GUIContainerObject *pSystem)
{
    TextWidgetLoadData data;
    data.readDomElement(rDomElement);

    pSystem->addTextWidget(data.point);
    pSystem->mTextWidgetList.last()->setText(data.text);
    qDebug() << "Font = " << data.font.toString();
    pSystem->mTextWidgetList.last()->setTextFont(data.font);
    pSystem->mTextWidgetList.last()->setTextColor(data.fontcolor);
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

//! @brief Convenience function for loading a box widget from a dom element
void loadBoxWidget(QDomElement &rDomElement, GUIContainerObject *pSystem)
{
    BoxWidgetLoadData data;
    data.readDomElement(rDomElement);

    pSystem->addBoxWidget(data.point);
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
    pSystem->mBoxWidgetList.last()->setSelected(false);

}


//! @brief Conveniance function if you dont want to manipulate the loaded data
void loadParameterValues(QTextStream &rStream, GUIContainerObject* pSystem, undoStatus undoSettings)
{
    ParameterLoadData data;
    data.read(rStream);
    loadParameterValues(data, pSystem, undoSettings);
}

//! @brief Loads the hmf file HEADER data and checks version numbers
HeaderLoadData readHeader(QTextStream &rInputStream, MessageWidget *pMessageWidget)
{
    HeaderLoadData headerData;
    headerData.read(rInputStream);

    if(headerData.hopsangui_version > QString(HOPSANGUIVERSION))
    {
        pMessageWidget->printGUIWarningMessage(QString("Warning: File was saved in newer version of Hopsan"));
    }
    else if(headerData.hopsangui_version < QString(HOPSANGUIVERSION))
    {
        pMessageWidget->printGUIWarningMessage(QString("Warning: File was saved in older version of Hopsan"));
    }

    if(headerData.hmf_version > QString(HMFVERSION))
    {
        pMessageWidget->printGUIWarningMessage(QString("Warning: File was saved in newer version of Hopsan"));
    }
    else if(headerData.hmf_version < QString(HMFVERSION))
    {
        pMessageWidget->printGUIWarningMessage(QString("Warning: File was saved in older version of Hopsan"));
    }

    if(headerData.caf_version > QString(CAFVERSION))
    {
        pMessageWidget->printGUIWarningMessage(QString("Warning: File was saved in newer version of Hopsan"));
    }
    else if(headerData.caf_version < QString(CAFVERSION))
    {
        pMessageWidget->printGUIWarningMessage(QString("Warning: File was saved in older version of Hopsan"));
    }

    return headerData;
}

////! @todo thois should be in a save related file, or make this file both save and load
//void writeHeader(QTextStream &rStream)
//{
//    //Make sure that the readHeader function is synced with changes here

//    //Write Header to save file
//    rStream << "--------------------------------------------------------------\n";
//    rStream << "-------------------  HOPSAN NG MODEL FILE  -------------------\n";
//    rStream << "--------------------------------------------------------------\n";
//    rStream << "HOPSANGUIVERSION " << HOPSANGUIVERSION << "\n";
//    rStream << "HMFVERSION " << HMFVERSION << "\n";
//    rStream << "CAFVERSION " << CAFVERSION << "\n";
//    rStream << "--------------------------------------------------------------\n";

//    //! @todo wite more header data like time and viewport
//}

//void addHMFHeader(QDomElement &rDomElement)
//{
//    QDomElement xmlHeader = appendDomElement(rDomElement,"versionnumbers");
//    appendDomTextNode(xmlHeader, "hopsanguiversion", HOPSANGUIVERSION);
//    appendDomTextNode(xmlHeader, "hmfversion", HMFVERSION);
//    appendDomTextNode(xmlHeader, "cafversion", CAFVERSION);
//}

QDomElement appendHMFRootElement(QDomDocument &rDomDocument)
{
    QDomElement hmfRoot = rDomDocument.createElement(HMF_ROOTTAG);
    rDomDocument.appendChild(hmfRoot);
    hmfRoot.setAttribute(HMF_VERSIONTAG,HMFVERSION);
    hmfRoot.setAttribute(HMF_HOPSANGUIVERSIONTAG, HOPSANGUIVERSION);
    hmfRoot.setAttribute(HMF_HOPSANCOREVERSIONTAG, "0"); //! @todo need to get this data in here somehow, maybe have a global that is set when the hopsan core is instansiated
    return hmfRoot;
}

