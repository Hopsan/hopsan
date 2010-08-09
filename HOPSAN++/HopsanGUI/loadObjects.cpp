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

void ObjectLoadData::read(QTextStream &rStream)
{
    type = readName(rStream);
    name = readName(rStream);  //Now read the name, assume that the name is contained within quotes signs, "name"
    rStream >> posX;
    rStream >> posY;

    //! @todo if not end of stream do this, to allow incomplete load_data
    rStream >> rotation;
    rStream >> nameTextPos;
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
    loadGUIObject(data,pLibrary, pGraphicsView, noUnDo);
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
void readHeader(QTextStream &rInputStream, MessageWidget *pMessageWidget)
{
    QString inputWord, ver;

    //Read and discard title block
    rInputStream.readLine();
    rInputStream.readLine();
    rInputStream.readLine();

    //Read the three version numbers
    for (int i=0; i<3; ++i)
    {
        rInputStream >> inputWord >> ver;

        if ( inputWord == "HOPSANGUIVERSION")
        {
            if(ver > QString(HOPSANGUIVERSION))
            {
                pMessageWidget->printGUIWarningMessage(QString("Warning: File was saved in newer version of Hopsan"));
            }
            else if(ver < QString(HOPSANGUIVERSION))
            {
                pMessageWidget->printGUIWarningMessage(QString("Warning: File was saved in older version of Hopsan"));
            }
        }
        else if ( inputWord == "HMFVERSION")
        {
            if(ver > QString(HMFVERSION))
            {
                pMessageWidget->printGUIWarningMessage(QString("Warning: File was saved in newer version of Hopsan"));
            }
            else if(ver < QString(HMFVERSION))
            {
                pMessageWidget->printGUIWarningMessage(QString("Warning: File was saved in older version of Hopsan"));
            }
        }
        else if ( inputWord == "CAFVERSION") //CAF = Component Appearance File
        {
            if(ver > QString(CAFVERSION))
            {
                pMessageWidget->printGUIWarningMessage(QString("Warning: File was saved in newer version of Hopsan"));
            }
            else if(ver < QString(CAFVERSION))
            {
                pMessageWidget->printGUIWarningMessage(QString("Warning: File was saved in older version of Hopsan"));
            }
        }
        else
        {
            pMessageWidget->printGUIErrorMessage(QString("Error: unknown HEADER command: " + inputWord));
        }
    }

    //Remove dashed end line
    rInputStream.readLine();
}

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
