#include "loadObjects.h"

#include <QObject> //!< @todo maybe not this one for connect()

#include "LibraryWidget.h"
#include "GraphicsView.h"
#include "GUIObject.h"
#include "GUIRootSystem.h"
#include "GUIConnector.h"
#include "GUIPort.h"

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



GUIObject* loadGUIObject(const ObjectLoadData &rData, LibraryWidget* pLibrary, GraphicsView* pGraphicsView)
{
    AppearanceData *pAppearanceData = pLibrary->getAppearanceData(rData.type);
    if (pAppearanceData != 0)
    {
        AppearanceData appearanceData = *pAppearanceData; //Make a copy
        appearanceData.setName(rData.name);

        GUIObject* pObj = pGraphicsView->addGUIObject(appearanceData, QPoint(rData.posX, rData.posY), 0);
        pObj->setNameTextPos(rData.nameTextPos);
        while(pObj->rotation() != rData.rotation)
        {
            pObj->rotate();
        }
        return pObj;
    }
    else
    {
        //! @todo Some error message
        return 0;
    }
}



void loadConnector(const ConnectorLoadData &rData, GraphicsView* pGraphicsView, GUIRootSystem* pRootSystem)
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


void loadParameterValues(const ParameterLoadData &rData, GraphicsView* pGraphicsView)
{
    //qDebug() << "Parameter: " << componentName << " " << parameterName << " " << parameterValue;
    pGraphicsView->mGUIObjectMap.find(rData.componentName).value()->setParameterValue(rData.parameterName, rData.parameterValue);
}

//! @brief Conveniance function if you dont want to manipulate the loaded data
GUIObject* loadGUIObject(QTextStream &rStream, LibraryWidget* pLibrary, GraphicsView* pGraphicsView)
{
    ObjectLoadData data;
    data.read(rStream);
    loadGUIObject(data,pLibrary,pGraphicsView);
}

//! @brief Conveniance function if you dont want to manipulate the loaded data
void loadConnector(QTextStream &rStream, GraphicsView* pGraphicsView, GUIRootSystem* pRootSystem)
{
    ConnectorLoadData data;
    data.read(rStream);
    loadConnector(data,pGraphicsView,pRootSystem);
}

//! @brief Conveniance function if you dont want to manipulate the loaded data
void loadParameterValues(QTextStream &rStream, GraphicsView* pGraphicsView)
{
    ParameterLoadData data;
    data.read(rStream);
    loadParameterValues(data,pGraphicsView);
}
