#include "loadObjects.h"

#include <QObject> //!< @todo maybe not this one for connect()

#include "LibraryWidget.h"
#include "GraphicsView.h"
#include "GUIObject.h"
#include "GUIRootSystem.h"
#include "GUIConnector.h"
#include "GUIPort.h"
#include "MessageWidget.h"
#include "version.h"

#include "GUIUtilities.h"

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

//! @brief Reads system appearnce data from stream
//! Assumes that this data ends when commandword has - as first sign
//! Header must have been removed (read) first
void SystemAppearanceLoadData::read(QTextStream &rStream)
{
    QString commandword;

    while ( !commandword.startsWith("-") )
    {
        rStream >> commandword;
        qDebug() << commandword;

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


void ParameterLoadData::read(QTextStream &rStream)
{
    componentName = readName(rStream);
    parameterName = readName(rStream);
    rStream >> parameterValue;
}



GUIObject* loadGUIObject(const ObjectLoadData &rData, LibraryWidget* pLibrary, GraphicsView* pGraphicsView, undoStatus undoSettings)
{
    AppearanceData *pAppearanceData = pLibrary->getAppearanceData(rData.type);
    if (pAppearanceData != 0)
    {
        AppearanceData appearanceData = *pAppearanceData; //Make a copy
        appearanceData.setName(rData.name);

        GUIObject* pObj = pGraphicsView->addGUIObject(appearanceData, QPoint(rData.posX, rData.posY), 0, DESELECTED, undoSettings);
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

GUIObject* loadSubsystemGUIObject(const SubsystemLoadData &rData, LibraryWidget* pLibrary, GraphicsView* pGraphicsView, undoStatus undoSettings)
{
    //! @todo maybe create a loadGUIObject function that takes appearance data instead of pLibrary (when special apperance are to be used)
    //Load the system the normal way (and add it)
    GUIObject* pSys = loadGUIObject(rData, pLibrary, pGraphicsView, undoSettings);

    //Now read the external file to change appearance and populate the system
    pSys->loadFromFile(rData.filepath);

    //Set the cqs type of the system
    pSys->setTypeCQS(rData.cqs_type);

    return pSys;
}



void loadConnector(const ConnectorLoadData &rData, GraphicsView* pGraphicsView, GUIRootSystem* pRootSystem, undoStatus undoSettings)
{
    qDebug() << rData.startComponentName << " " << rData.endComponentName << " " << pRootSystem->getName();
    bool success = pRootSystem->connect(rData.startComponentName, rData.startPortName, rData.endComponentName, rData.endPortName);
    if (success)
    {
        //Check if the component names are the same as the guiroot system name in such cases we should search for the actual systemport gui object instead
        //!< @todo this is extremely strange, some day we need to figure out a way that allways works the same way, this will likly mean MAJOR changes
        QString startGuiObjName, endGuiObjName;
        if (rData.startComponentName == pRootSystem->getName())
        {
            startGuiObjName = rData.startPortName;
        }
        else
        {
            startGuiObjName = rData.startComponentName;
        }
        if (rData.endComponentName == pRootSystem->getName())
        {
            endGuiObjName = rData.endPortName;
        }
        else
        {
            endGuiObjName = rData.endComponentName;
        }

        //! @todo Need some error handling here to avoid crash if components or ports do not exist
        GUIPort *startPort = pGraphicsView->getGUIObject(startGuiObjName)->getPort(rData.startPortName);
        GUIPort *endPort = pGraphicsView->getGUIObject(endGuiObjName)->getPort(rData.endPortName);

        GUIConnector *pTempConnector = new GUIConnector(startPort, endPort, rData.pointVector, pGraphicsView);
        pGraphicsView->scene()->addItem(pTempConnector);

        //Hide connected ports
        startPort->hide();
        endPort->hide();

        QObject::connect(startPort->getGuiObject(),SIGNAL(componentDeleted()),pTempConnector,SLOT(deleteMeWithNoUndo()));
        QObject::connect(endPort->getGuiObject(),SIGNAL(componentDeleted()),pTempConnector,SLOT(deleteMeWithNoUndo()));

        pGraphicsView->mConnectorVector.append(pTempConnector);

    }
    else
    {
        qDebug() << "Unsuccessful connection try" << endl;
    }
}


void loadParameterValues(const ParameterLoadData &rData, GraphicsView* pGraphicsView, undoStatus undoSettings)
{
    //qDebug() << "Parameter: " << componentName << " " << parameterName << " " << parameterValue;
    pGraphicsView->mGUIObjectMap.find(rData.componentName).value()->setParameterValue(rData.parameterName, rData.parameterValue);
}

//! @brief Conveniance function if you dont want to manipulate the loaded data
GUIObject* loadGUIObject(QTextStream &rStream, LibraryWidget* pLibrary, GraphicsView* pGraphicsView, undoStatus undoSettings)
{
    ObjectLoadData data;
    data.read(rStream);
    return loadGUIObject(data,pLibrary, pGraphicsView, undoSettings);
}

//! @brief Conveniance function if you dont want to manipulate the loaded data
void loadConnector(QTextStream &rStream, GraphicsView* pGraphicsView, GUIRootSystem* pRootSystem, undoStatus undoSettings)
{
    ConnectorLoadData data;
    data.read(rStream);
    loadConnector(data,pGraphicsView, pRootSystem, undoSettings);
}

//! @brief Conveniance function if you dont want to manipulate the loaded data
void loadParameterValues(QTextStream &rStream, GraphicsView* pGraphicsView, undoStatus undoSettings)
{
    ParameterLoadData data;
    data.read(rStream);
    loadParameterValues(data, pGraphicsView, undoSettings);
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
