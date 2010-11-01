#include "loadObjects.h"

#include <QObject> //!< @todo maybe not this one for connect()

#include "LibraryWidget.h"
#include "GraphicsView.h"
#include "GUIObject.h"
#include "CoreSystemAccess.h"
#include "GUIConnector.h"
#include "GUIPort.h"
#include "MessageWidget.h"
#include "version.h"
#include "GUISystem.h"
#include "GUIUtilities.h"
#include "GraphicsScene.h"

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

void ObjectLoadData::read(QTextStream &rStream)
{
    type = readName(rStream);
    name = readName(rStream);  //Now read the name, assume that the name is contained within quotes signs, "name"
    rStream >> posX;
    rStream >> posY;

    //! @todo if not end of stream do this, to allow incomplete load_data
    rStream >> rotation;
    rStream >> nameTextPos;
    rStream >> textVisible;
}

void ObjectLoadData::readDomElement(QDomElement &rDomElement)
{
    //Read core specific data
    type = rDomElement.firstChildElement(HMF_TYPETAG).text();
    name = rDomElement.firstChildElement(HMF_NAMETAG).text();

    //! What about parameters

    //Read gui specific data
    //! @todo maybe have common function for this
    QDomElement guiData = rDomElement.firstChildElement("hopsangui");
    parseDomValueNode3(guiData.firstChildElement("pose"), posX, posY, rotation);

    nameTextPos = parseDomValueNode(guiData.firstChildElement("nametextpos"));
    textVisible = parseDomValueNode(guiData.firstChildElement("visibel")); //should be bool
}

void SubsystemLoadData::read(QTextStream &rStream)
{
    type = "Subsystem";
    loadtype = readName(rStream);
    name = readName(rStream);
    cqs_type = readName(rStream);

    if (loadtype == "EXTERNAL")
    {
        filepath = readName(rStream);

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
    //! @todo should check if tagname is what we expect really, should do this in all readDomElement functions
    type = "Subsystem"; //Hardcode the type, regardles of hmf contents (should not contain type
    //loadtype = readName(rStream);
    name = rDomElement.firstChildElement("name").text();
    cqs_type = rDomElement.firstChildElement("cqs_type").text();
    filepath = rDomElement.firstChildElement("external_path").text();

    //! @todo loadtype should probably be removed
    if(filepath.isEmpty())
    {
        loadtype = "embeded";
    }
    else
    {
        loadtype = "EXTERNAL";
    }

    if (loadtype == "EXTERNAL")
    {
        //filepath = readName(rStream);

        //Read gui specific data
        //! @todo maybe have common function for this
        QDomElement guiData = rDomElement.firstChildElement("hopsangui");
        parseDomValueNode3(guiData.firstChildElement("pose"), posX, posY, rotation);

        nameTextPos = parseDomValueNode(guiData.firstChildElement("nametextpos"));
        textVisible = parseDomValueNode(guiData.firstChildElement("visibel")); //should be bool
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
    startComponentName = rDomElement.firstChildElement("startcomponent").text();
    startPortName = rDomElement.firstChildElement("startport").text();
    endComponentName = rDomElement.firstChildElement("endcomponent").text();
    endPortName = rDomElement.firstChildElement("endport").text();

    //Read gui specific stuff
    QDomElement guiData = rDomElement.firstChildElement(HMF_HOPSANGUITAG);
    qreal x,y;
    QDomElement xyNode = guiData.firstChildElement(HMF_XYTAG);
    while (!xyNode.isNull())
    {
        parseDomValueNode2(xyNode,x,y);
        pointVector.push_back(QPointF(x,y));
        xyNode = xyNode.nextSiblingElement(HMF_XYTAG);
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
    componentName = "";
    parameterName = rDomElement.firstChildElement("name").text();
    parameterValue = parseDomValueNode(rDomElement.firstChildElement("value"));
}



GUIObject* loadGUIObject(const ObjectLoadData &rData, LibraryWidget* pLibrary, GUISystem* pSystem, undoStatus undoSettings)
{
    AppearanceData *pAppearanceData = pLibrary->getAppearanceData(rData.type);
    if (pAppearanceData != 0)
    {
        AppearanceData appearanceData = *pAppearanceData; //Make a copy
        appearanceData.setName(rData.name);

        GUIObject* pObj = pSystem->addGUIObject(&appearanceData, QPoint(rData.posX, rData.posY), 0, DESELECTED, undoSettings);
        pObj->setNameTextPos(rData.nameTextPos);
        if(!rData.textVisible)
        {
            pObj->hideName();
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

GUIObject* loadSubsystemGUIObject(const SubsystemLoadData &rData, LibraryWidget* pLibrary, GUISystem* pSystem, undoStatus undoSettings)
{
    //! @todo maybe create a loadGUIObject function that takes appearance data instead of pLibrary (when special apperance are to be used)
    //Load the system the normal way (and add it)
    GUIObject* pSys = loadGUIObject(rData, pLibrary, pSystem, undoSettings);

    //Now read the external file to change appearance and populate the system
    pSys->loadFromHMF(rData.filepath);

    //Set the cqs type of the system
    pSys->setTypeCQS(rData.cqs_type);

    return pSys;
}



void loadConnector(const ConnectorLoadData &rData, GUISystem* pSystem, undoStatus undoSettings)
{
    qDebug() << "loadConnector: " << rData.startComponentName << " " << rData.endComponentName << " " << pSystem->mpCoreSystemAccess->getRootSystemName();
    bool success = pSystem->mpCoreSystemAccess->connect(rData.startComponentName, rData.startPortName, rData.endComponentName, rData.endPortName);
    if (success)
    {
        //Check if the component names are the same as the guiroot system name in such cases we should search for the actual systemport gui object instead
        //!< @todo this is extremely strange, some day we need to figure out a way that allways works the same way, this will likly mean MAJOR changes
        QString startGuiObjName, endGuiObjName;
        if (rData.startComponentName == pSystem->mpCoreSystemAccess->getRootSystemName())
        {
            startGuiObjName = rData.startPortName;
        }
        else
        {
            startGuiObjName = rData.startComponentName;
        }
        if (rData.endComponentName == pSystem->mpCoreSystemAccess->getRootSystemName())
        {
            endGuiObjName = rData.endPortName;
        }
        else
        {
            endGuiObjName = rData.endComponentName;
        }

        //! @todo all of this (above and bellow) should be inside some conventiant function like "connect"
        //! @todo Need some error handling here to avoid crash if components or ports do not exist
        GUIPort *startPort = pSystem->getGUIObject(startGuiObjName)->getPort(rData.startPortName);
        GUIPort *endPort = pSystem->getGUIObject(endGuiObjName)->getPort(rData.endPortName);

        GUIConnector *pTempConnector = new GUIConnector(startPort, endPort, rData.pointVector, pSystem);
        pSystem->mpScene->addItem(pTempConnector);

        //Hide connected ports
        startPort->hide();
        endPort->hide();

        QObject::connect(startPort->getGuiObject(),SIGNAL(componentDeleted()),pTempConnector,SLOT(deleteMeWithNoUndo()));
        QObject::connect(endPort->getGuiObject(),SIGNAL(componentDeleted()),pTempConnector,SLOT(deleteMeWithNoUndo()));

        pSystem->mSubConnectorList.append(pTempConnector);
    }
    else
    {
        qDebug() << "Unsuccessful connection atempt" << endl;
    }
}

//! @brief text version
void loadParameterValues(const ParameterLoadData &rData, GUISystem* pSystem, undoStatus undoSettings)
{
    //qDebug() << "Parameter: " << componentName << " " << parameterName << " " << parameterValue;
    //qDebug() << "count" << pSystem->mGUIObjectMap.count(rData.componentName);
    qDebug() << "load Parameter value for component: " << rData.componentName  << " in " << pSystem->getName();
    //qDebug() << "Parameter: " << rData.parameterName << " " << rData.parameterValue;
    GUIObject* ptr = pSystem->mGUIObjectMap.find(rData.componentName).value();
    qDebug() << ptr->getName();
    if (ptr != 0)
        ptr->setParameterValue(rData.parameterName, rData.parameterValue);
    else
        assert(false);

}

//! @brief xml version
void loadParameterValue(const ParameterLoadData &rData, GUIObject* pObject, undoStatus undoSettings)
{
    pObject->setParameterValue(rData.parameterName, rData.parameterValue);
}

//! @brief xml version
void loadParameterValue(QDomElement &rDomElement, GUIObject* pObject, undoStatus undoSettings)
{
    ParameterLoadData data;
    data.readDomElement(rDomElement);
    loadParameterValue(data, pObject, undoSettings);
}


//! @brief Conveniance function if you dont want to manipulate the loaded data
GUIObject* loadGUIObject(QTextStream &rStream, LibraryWidget* pLibrary, GUISystem* pSystem, undoStatus undoSettings)
{
    ObjectLoadData data;
    data.read(rStream);
    return loadGUIObject(data,pLibrary, pSystem, undoSettings);
}

//! @brief Conveniance function if you dont want to manipulate the loaded data
GUIObject* loadGUIObject(QDomElement &rDomElement, LibraryWidget* pLibrary, GUISystem* pSystem, undoStatus undoSettings)
{
    ObjectLoadData data;
    data.readDomElement(rDomElement);
    return loadGUIObject(data,pLibrary, pSystem, undoSettings);
}

GUIObject* loadSubsystemGUIObject(QTextStream &rStream, LibraryWidget* pLibrary, GUISystem* pSystem, undoStatus undoSettings)
{
    SubsystemLoadData data;
    data.read(rStream);
    return loadSubsystemGUIObject(data, pLibrary, pSystem, undoSettings);
}

GUIObject* loadSubsystemGUIObject(QDomElement &rDomElement, LibraryWidget* pLibrary, GUISystem* pSystem, undoStatus undoSettings)
{
    SubsystemLoadData data;
    data.readDomElement(rDomElement);
    return loadSubsystemGUIObject(data, pLibrary, pSystem, undoSettings);
}

//! @brief Conveniance function if you dont want to manipulate the loaded data
void loadConnector(QTextStream &rStream, GUISystem* pSystem, undoStatus undoSettings)
{
    ConnectorLoadData data;
    data.read(rStream);
    loadConnector(data, pSystem, undoSettings);
}

//! @brief Conveniance function if you dont want to manipulate the loaded data
void loadConnector(QDomElement &rDomElement, GUISystem* pSystem, undoStatus undoSettings)
{
    ConnectorLoadData data;
    data.readDomElement(rDomElement);
    loadConnector(data, pSystem, undoSettings);
}

//! @brief Conveniance function if you dont want to manipulate the loaded data
void loadParameterValues(QTextStream &rStream, GUISystem* pSystem, undoStatus undoSettings)
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

//! @todo thois should be in a save related file, or make this file both save and load
void writeHeader(QTextStream &rStream)
{
    //Make sure that the readHeader function is synced with changes here

    //Write Header to save file
    rStream << "--------------------------------------------------------------\n";
    rStream << "-------------------  HOPSAN NG MODEL FILE  -------------------\n";
    rStream << "--------------------------------------------------------------\n";
    rStream << "HOPSANGUIVERSION " << HOPSANGUIVERSION << "\n";
    rStream << "HMFVERSION " << HMFVERSION << "\n";
    rStream << "CAFVERSION " << CAFVERSION << "\n";
    rStream << "--------------------------------------------------------------\n";

    //! @todo wite more header data like time and viewport
}

void addHMFHeader(QDomElement &rDomElement)
{
    QDomElement xmlHeader = appendDomElement(rDomElement,"versionnumbers");
    appendDomTextNode(xmlHeader, "hopsanguiversion", HOPSANGUIVERSION);
    appendDomTextNode(xmlHeader, "hmfversion", HMFVERSION);
    appendDomTextNode(xmlHeader, "cafversion", CAFVERSION);
}

//! @brief Helper function for adding one initially empty Dom node
//! @todo maybe retunr reference (is there a difference)
//! @returns The new sub element dom node
QDomElement appendDomElement(QDomElement &rDomElement, const QString element_name)
{
    QDomElement subDomElement = rDomElement.ownerDocument().createElement(element_name);
    rDomElement.appendChild(subDomElement);
    return subDomElement;
}

//! @brief Helper function for adding Dom elements containing one text node
void appendDomTextNode(QDomElement &rDomElement, const QString element_name, const QString text)
{
    QDomDocument ownerDomDocument = rDomElement.ownerDocument();
    QDomElement subDomElement = ownerDomDocument.createElement(element_name);
    subDomElement.appendChild(ownerDomDocument.createTextNode(text));
    rDomElement.appendChild(subDomElement);
}

//! @brief Helper function for adding Dom elements containing one text node (based on a double value)
void appendDomValueNode(QDomElement &rDomElement, const QString element_name, const double val)
{
    QString tmp_string;
    tmp_string.setNum(val);
    appendDomTextNode(rDomElement, element_name, tmp_string);
}


void appendDomValueNode2(QDomElement &rDomElement, const QString element_name, const double a, const double b)
{
    QString num,str;
    num.setNum(a);
    str.append(num);
    str.append(" ");
    num.setNum(b);
    str.append(num);
    appendDomTextNode(rDomElement, element_name, str);
}


void appendDomValueNode3(QDomElement &rDomElement, const QString element_name, const double a, const double b, const double c)
{
    QString num,str;
    num.setNum(a);
    str.append(num);
    str.append(" ");
    num.setNum(b);
    str.append(num);
    str.append(" ");
    num.setNum(c);
    str.append(num);
    appendDomTextNode(rDomElement, element_name, str);
}


void appendDomValueNodeN(QDomElement &rDomElement, const QString element_name, const QVector<qreal> &rValues)
{
    QString num,str;
    for (unsigned int i=0; i<rValues.size(); ++i)
    {
        num.setNum(rValues[i]);
        str.append(num);
        str.append(" ");
    }
    str.chop(1); //Remove last space
    appendDomTextNode(rDomElement, element_name, str);
}



void parseDomValueNode3(QDomElement domElement, double &rA, double &rB, double &rC)
{
    QStringList poseList = domElement.text().split(" ");
    rA = poseList[0].toDouble();
    rB = poseList[1].toDouble();
    rC = poseList[2].toDouble();
}

void parseDomValueNode2(QDomElement domElement, double &rA, double &rB)
{
    QStringList poseList = domElement.text().split(" ");
    rA = poseList[0].toDouble();
    rB = poseList[1].toDouble();
}

qreal parseDomValueNode(QDomElement domElement)
{
    return domElement.text().toDouble();
}
