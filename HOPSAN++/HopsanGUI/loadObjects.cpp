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
    rStream >> loadtype;
    name = readName(rStream);
    cqs_type = readName(rStream);

    if (loadtype == "external")
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
    else if (loadtype == "embeded")
    {
        //not implemented yet
        //! @todo handle error
        assert(false);
    }
    else
    {
        //incorrect type
        //! @todo handle error
        assert(false);
    }
}

void SystemAppearanceLoadData::read(QTextStream &rStream)
{
    QString commandword;
    rStream >> commandword;

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
    rStream >> parameterName;
    rStream >> parameterValue;
}



GUIObject* loadGUIObject(const ObjectLoadData &rData, LibraryWidget* pLibrary, GraphicsView* pGraphicsView, bool noUnDo)
{
    AppearanceData *pAppearanceData = pLibrary->getAppearanceData(rData.type);
    if (pAppearanceData != 0)
    {
        AppearanceData appearanceData = *pAppearanceData; //Make a copy
        appearanceData.setName(rData.name);

        GUIObject* pObj = pGraphicsView->addGUIObject(appearanceData, QPoint(rData.posX, rData.posY), 0, true, noUnDo);
        pObj->setNameTextPos(rData.nameTextPos);
        if(!rData.textVisible)
        {
            pObj->hideName();
        }
        while(pObj->rotation() != rData.rotation)
        {
            pObj->rotate(noUnDo);
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

GUIObject* loadSubsystemGUIObject(const SubsystemLoadData &rData, LibraryWidget* pLibrary, GraphicsView* pGraphicsView, bool noUnDo)
{
    //! @todo maybe create a loadGUIObject function that takes appearance data instead of pLibrary (when special apperance are to be used)
    //Load the system the normal way (and add it)
    GUIObject* pSys = loadGUIObject(rData, pLibrary, pGraphicsView, noUnDo);

    //Now read the external file to change appearance and populate the system
     QFile file(rData.filepath);
     if (!file.open(QIODevice::ReadOnly | QIODevice::Text))  //open file
     {
         //! @todo Should signal an error message on screen
         qDebug() << "Failed to open file or not a text file: " + rData.filepath;
         return 0;
     }
     QTextStream inputStream(&file);  //Create a QTextStream object to stream the content of file

     //Read the entire file for the appearance specific data (a slight waste of time but its ok for now)
     //! @todo maybe should try to read more efficiently somehow, in this case only read the appearance specific data, not go through the entire file
     SystemAppearanceLoadData appdata;
     while (!inputStream.atEnd())
     {
        appdata.read(inputStream);
     }

     pSys->getAppearanceData()->setIconPathUser(appdata.usericon_path);
     pSys->getAppearanceData()->setIconPathISO(appdata.isoicon_path);

     PortAppearanceMapT* portappmap = &(pSys->getAppearanceData()->getPortAppearanceMap());
     for (int i=0; i<appdata.portnames.size(); ++i)
     {
         PortAppearance portapp;
         portapp.x = appdata.port_xpos[i];
         portapp.y = appdata.port_ypos[i];
         portapp.rot = appdata.port_angle[i];
         portapp.selectPortIcon("","",""); //!< @todo fix this

         portappmap->insert(appdata.portnames[i], portapp);
     }

     //Load the contents of the subsystem from the external file
     //! @todo do this

     pSys->refreshAppearance();

     return pSys;
}



void loadConnector(const ConnectorLoadData &rData, GraphicsView* pGraphicsView, GUIRootSystem* pRootSystem, bool noUnDo)
{
    //! @todo Need some error handling here to avoid crash if components or ports do not exist
    GUIPort *startPort = pGraphicsView->getGUIObject(rData.startComponentName)->getPort(rData.startPortName);
    GUIPort *endPort = pGraphicsView->getGUIObject(rData.endComponentName)->getPort(rData.endPortName);

    bool success = pRootSystem->connect(rData.startComponentName, rData.startPortName, rData.endComponentName, rData.endPortName);
    if (success)
    {
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


void loadParameterValues(const ParameterLoadData &rData, GraphicsView* pGraphicsView, bool noUnDo)
{
    //qDebug() << "Parameter: " << componentName << " " << parameterName << " " << parameterValue;
    pGraphicsView->mGUIObjectMap.find(rData.componentName).value()->setParameterValue(rData.parameterName, rData.parameterValue);
}

//! @brief Conveniance function if you dont want to manipulate the loaded data
GUIObject* loadGUIObject(QTextStream &rStream, LibraryWidget* pLibrary, GraphicsView* pGraphicsView, bool noUnDo)
{
    ObjectLoadData data;
    data.read(rStream);
    return loadGUIObject(data,pLibrary, pGraphicsView, noUnDo);
}

//! @brief Conveniance function if you dont want to manipulate the loaded data
void loadConnector(QTextStream &rStream, GraphicsView* pGraphicsView, GUIRootSystem* pRootSystem, bool noUnDo)
{
    ConnectorLoadData data;
    data.read(rStream);
    loadConnector(data,pGraphicsView, pRootSystem, noUnDo);
}

//! @brief Conveniance function if you dont want to manipulate the loaded data
void loadParameterValues(QTextStream &rStream, GraphicsView* pGraphicsView, bool noUnDo)
{
    ParameterLoadData data;
    data.read(rStream);
    loadParameterValues(data, pGraphicsView, noUnDo);
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
