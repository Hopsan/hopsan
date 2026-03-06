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
//! @file   SSPWidget.cpp
//! @brief A widget for exploring SSPs
//! @author Robert Braun <robert.braun@liu.se>
//!
//$Id$

#include "SSPWidget.h"
#include "GUIObjects/GUIContainerObject.h"
#include "GUIObjects/GUIModelObject.h"
#include "GUIPort.h"
#include "ModelHandler.h"
#include "common.h"
#include <QVBoxLayout>
#include <QApplication>
#include <QDebug>

#include "global.h"
#include "ssp4c.h"
#include "ssp4c_ssd.h"
#include "ssp4c_ssd_component.h"
#include "ssp4c_ssd_connection.h"
#include "ssp4c_ssd_connector.h"

SSPWidget::SSPWidget(QWidget *pParent)
{
    this->setObjectName("SSPWidget");
    this->setWindowTitle("SSP Explorer");

    mpTree = new QTreeWidget(this);
    mpTree->setMouseTracking(true);
    mpTree->setHeaderHidden(true);
    mpTree->setColumnCount(1);


    QVBoxLayout *pLayout = new QVBoxLayout(this);;
    pLayout->addWidget(mpTree);
}

void SSPWidget::addSSP(QFileInfo path)
{
    sspHandle *ssp = ssp4c_loadSsp(path.absoluteFilePath().toStdString().c_str());

    QFont boldFont = qApp->font();
    boldFont.setBold(true);

    QTreeWidgetItem *pSspItem = new QTreeWidgetItem();
    pSspItem->setFont(0,boldFont);
    pSspItem->setIcon(0, QIcon(QString(ICONPATH) + "svg/Hopsan-SSP.svg"));
    pSspItem->setText(0,path.fileName());
    mpTree->addTopLevelItem(pSspItem);

    connect(mpTree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), this, SLOT(openSSDModel(QTreeWidgetItem*,int)));

    qDebug() << "SSP unzipped path: " << ssp4c_getUnzippedLocation(ssp);
    int ssdCount = ssp4c_getNumberOfSsds(ssp);
    for(int i=0; i<ssdCount; ++i) {
        ssdHandle *ssd = ssp4c_getSsdByIndex(ssp,i);
        qDebug() << "SSD filename: " << ssp4c_ssd_getFileName(ssd);
        qDebug() << "SSD name: " << ssp4c_ssd_getName(ssd);
        qDebug() << "SSD version: " << ssp4c_ssd_getVersion(ssd);
        qDebug() << "SSD id: " << ssp4c_ssd_getId(ssd);
        qDebug() << "SSD description: " << ssp4c_ssd_getDescription(ssd);
        qDebug() << "SSD author: " << ssp4c_ssd_getAuthor(ssd);
        qDebug() << "SSD fileversion: " << ssp4c_ssd_getFileversion(ssd);
        qDebug() << "SSD copyright: " << ssp4c_ssd_getCopyright(ssd);
        qDebug() << "SSD license: " << ssp4c_ssd_getLicense(ssd);
        qDebug() << "SSD generationTool :" << ssp4c_ssd_getGenerationTool(ssd);
        qDebug() << "SSD generationDateAndTime: " << ssp4c_ssd_getGenerationDateAndTime(ssd);

        QTreeWidgetItem *pSsdItem = new QTreeWidgetItem();
        //pSsdItem->setIcon(0, QIcon(QString(ICONPATH) + "svg/Hopsan-SSD.svg"));
        pSsdItem->setText(0,ssp4c_ssd_getName(ssd));
        pSspItem->addChild(pSsdItem);

        itemToSspMap.insert(pSsdItem, ssp);
        itemToSsdMap.insert(pSsdItem, ssd);
    }
}

void SSPWidget::openSSDModel(QTreeWidgetItem *item, int)
{
    sspHandle *ssp = itemToSspMap[item];
    ssdHandle* ssd = itemToSsdMap[item];

    //! @todo Check so that SSD is not already open somehow

    gpModelHandler->addNewModel(ssp4c_ssd_getName(ssd));

    int componentCount = ssp4c_ssd_getNumberOfComponents(ssd);
    for(int i=0; i<componentCount; ++i) {
        ssdComponentHandle *comp = ssp4c_ssd_getComponentByIndex(ssd,i);
        qDebug() << "  component name: " << ssp4c_ssd_component_getName(comp);
        qDebug() << "  component source: " << ssp4c_ssd_component_getSource(comp);

        int connectorsCount = ssp4c_ssd_component_getNumberOfConnectors(comp);
        for(int i=0; i<connectorsCount; ++i) {
            ssdConnectorHandle *con = ssp4c_ssd_component_getConnectorByIndex(comp, i);
            qDebug() << "    connector: " << ssp4c_ssd_connector_getName(con);
        }
        SystemObject *pSystem = gpModelHandler->getCurrentTopLevelSystem();
        if(pSystem) {
            QPointF pos = pSystem->getGraphicsViewport().mCenter;
            ModelObject *pFmuComponent = pSystem->addModelObject("FMIWrapper", pos);
            if(pFmuComponent) {
                QString unzippedLocation = ssp4c_getUnzippedLocation(ssp);
                QString source = ssp4c_ssd_component_getSource(comp);
                pFmuComponent->setParameterValue("path", unzippedLocation+"/"+source);
                pSystem->renameModelObject(pFmuComponent->getName(), ssp4c_ssd_component_getName(comp));
            }
        }
    }

    int connetionCount = ssp4c_ssd_getNumberOfConnections(ssd);
    for(int i=0; i<connetionCount; ++i) {
        ssdConnectionHandle *con = ssp4c_ssd_getConnectionByIndex(ssd,i);
        qDebug() << "  start element: " << ssp4c_ssd_connection_getStartElement(con);
        qDebug() << "  start connector: " << ssp4c_ssd_connection_getStartConnector(con);
        qDebug() << "  end element: " << ssp4c_ssd_connection_getEndElement(con);
        qDebug() << "  end connector: " << ssp4c_ssd_connection_getEndConnector(con);

        SystemObject *pSystem = gpModelHandler->getCurrentTopLevelSystem();
        if(pSystem) {
            ModelObject *pStartComponent = pSystem->getModelObject(ssp4c_ssd_connection_getStartElement(con));
            ModelObject *pEndComponent = pSystem->getModelObject(ssp4c_ssd_connection_getEndElement(con));
            if(pStartComponent != nullptr && pEndComponent != nullptr) {
                qDebug() << "Found components!";
                QString startPortName = ssp4c_ssd_connection_getStartConnector(con);
                startPortName.replace(".", "_");
                QString endPortName = ssp4c_ssd_connection_getEndConnector(con);
                endPortName.replace(".", "_");
                Port *pStartPort = pStartComponent->getPort(startPortName);
                Port *pEndPort = pEndComponent->getPort(endPortName);
                if(pStartPort != nullptr && pEndComponent != nullptr) {
                    qDebug() << "Found ports!";
                    SystemObject *sysObj = gpModelHandler->getCurrentViewContainerObject();
                    Connector *pCon = sysObj->createConnector(pStartPort, pEndPort, NoUndo);
                    if(pCon != nullptr) {
                        qDebug() << "Creatd connetor!";
                        QVector<QPointF> pointVector;
                        QStringList geometryList;
                        pointVector.push_back(pCon->getStartPort()->boundingRect().center());
                        pointVector.push_back(pCon->getEndPort()->boundingRect().center());
                        geometryList.clear();
                        geometryList.append(hmf::connector::diagonal);
                        pCon->setPointsAndGeometries(pointVector, geometryList);
                    }
                }
            }
        }
    }

}

