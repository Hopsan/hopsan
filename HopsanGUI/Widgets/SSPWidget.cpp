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
#include <QMenu>
#include <QMessageBox>
#include <QContextMenuEvent>

#include "global.h"
#include "ssp4c.h"
#include "ssp4c_ssd.h"
#include "ssp4c_ssd_system.h"
#include "ssp4c_ssd_component.h"
#include "ssp4c_ssd_connection.h"
#include "ssp4c_ssd_connector.h"
#include "ssp4c_ssv_parameter_set.h"
#include "ssp4c_ssv_parameter.h"
#include "ssp4c_ssm_parameter_mapping.h"
#include "ssp4c_ssm_mapping_entry.h"

SSPWidget::SSPWidget(QWidget *pParent)
{
    this->setObjectName("SSPWidget");
    this->setWindowTitle("SSP Explorer");

    mpTree = new SSPTreeWidget(this);

    QVBoxLayout *pLayout = new QVBoxLayout(this);;
    pLayout->addWidget(mpTree);
}

void SSPWidget::addSSP(QFileInfo path)
{
    sspHandle *ssp = ssp4c_loadSsp(path.absoluteFilePath().toStdString().c_str());

    QFont boldFont = qApp->font();
    boldFont.setBold(true);

    QTreeWidgetItem *pSspItem = new QTreeWidgetItem(SSPTreeWidget::SSPItem);
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

        QTreeWidgetItem *pSsdItem = new QTreeWidgetItem(SSPTreeWidget::SSDItem);
        pSsdItem->setIcon(0, QIcon(QString(ICONPATH) + "svg/Hopsan-SSD.svg"));
        QString itemName = ssp4c_ssd_getFileName(ssd);
        QString name = ssp4c_ssd_getName(ssd);
        if(!name.isEmpty()) {
            itemName.append(" (\"");
            itemName.append(name);
            itemName.append("\")");
        }
        pSsdItem->setText(0,itemName);
        pSspItem->addChild(pSsdItem);

        itemToSspMap.insert(pSsdItem, ssp);
        itemToSsdMap.insert(pSsdItem, ssd);

        auto rootSystem = ssp4c_ssd_getRootSystem(ssd);
        if(rootSystem != nullptr) {
            QTreeWidgetItem *pSystemItem = new QTreeWidgetItem(SSPTreeWidget::SystemItem);
            pSystemItem->setText(0, QString(ssp4c_ssd_system_getName(rootSystem)));
            pSsdItem->addChild(pSystemItem);
            itemToSspMap.insert(pSystemItem, ssp);
            itemToSsdMap.insert(pSystemItem, ssd);
            itemToSystemMap.insert(pSystemItem, rootSystem);
            int componentCount = ssp4c_ssd_system_getNumberOfComponents(rootSystem);
            for(int i=0; i<componentCount; ++i) {
                ssdComponentHandle *comp = ssp4c_ssd_system_getComponentByIndex(rootSystem, i);
                
                QTreeWidgetItem *pCompItem = new QTreeWidgetItem(SSPTreeWidget::FMUItem);
                pCompItem->setText(0, QString(ssp4c_ssd_component_getName(comp)));
                pCompItem->setIcon(0, QIcon(QString(ICONPATH) + "svg/Hopsan-FMI.svg"));
                pSystemItem->addChild(pCompItem);
            }
        }


    }

    QTreeWidgetItem *pResourcesItem = new QTreeWidgetItem();
    pResourcesItem->setText(0, "Resources");
    pSspItem->addChild(pResourcesItem);

    int ssvCount = ssp4c_getNumberOfSsvs(ssp);
    qDebug() << "Found" << ssvCount << "SSVs";
    for(int i=0; i<ssvCount; ++i) {
        ssvParameterSetHandle *ssv = ssp4c_getSsvByIndex(ssp, i);
        int parCount = ssp4c_ssv_parameterSet_getNumberOfParameters(ssv);
        qDebug() << "Found" << parCount << "SSV parameters";
        for(int p=0; p<parCount; ++p) {
            ssvParameterHandle *par = ssp4c_ssv_parameterSet_getParameterByIndex(ssv, p);
            //qDebug() << "Parameter";
            //qDebug() << "  Name:  "+QString(ssp4c_ssv_parameter_getName(par));
            sspDataType type = ssp4c_ssv_parameter_getDatatype(par);
            if(type == sspDataTypeString) {
                //qDebug() << "  Value: "+QString(ssp4c_ssv_parameter_getStringValue(par));
            }
            else if(type == sspDataTypeReal) {
                //qDebug() << "  Value: "+QString::number(ssp4c_ssv_parameter_getRealValue(par));
            }
        }

        QTreeWidgetItem *pSsvItem = new QTreeWidgetItem(SSPTreeWidget::SSVItem);
        pSsvItem->setIcon(0, QIcon(QString(ICONPATH) + "svg/Hopsan-SSV.svg"));
        QString itemName = ssp4c_ssv_parameterSet_getFileName(ssv);
        QString name = ssp4c_ssv_parameterSet_getName(ssv);
        if(!name.isEmpty()) {
            itemName.append(" (\"");
            itemName.append(name);
            itemName.append("\")");
        }
        pSsvItem->setText(0,itemName);
        pResourcesItem->addChild(pSsvItem);

        itemToSspMap.insert(pSsvItem, ssp);
        itemToSsvMap.insert(pSsvItem, ssv);
    }

    int ssmCount = ssp4c_getNumberOfSsms(ssp);
    qDebug() << "Found" << ssmCount << "SSMs";
    for(int i=0; i<ssmCount; ++i) {
        ssmParameterMappingHandle *ssm = ssp4c_getSsmByIndex(ssp, i);
        int entryCount = ssp4c_ssm_parameterMapping_getNumberOfMappingEntries(ssm);
        qDebug() << "Found" << entryCount << "mapping entries";
        for(int e=0; e<entryCount; ++e) {
            ssp4c_ssm_parameterMapping_getMappingEntryByIndex(ssm,e);
            ssmParameterMappingEntryHandle *entry = ssp4c_ssm_parameterMapping_getMappingEntryByIndex(ssm, e);
            //qDebug() << "Source:" << QString(ssp4c_ssm_mappingEntry_getSource(entry));
            //qDebug() << "Target:" << QString(ssp4c_ssm_mappingEntry_getTarget(entry));
        }

        QTreeWidgetItem *pSsmItem = new QTreeWidgetItem(SSPTreeWidget::SSMItem);
        pSsmItem->setIcon(0, QIcon(QString(ICONPATH) + "svg/Hopsan-SSM.svg"));
        pSsmItem->setText(0,QString(ssp4c_ssm_parameterMapping_getFilename(ssm)));
        pResourcesItem->addChild(pSsmItem);

        itemToSspMap.insert(pSsmItem, ssp);
        itemToSsmMap.insert(pSsmItem, ssm);
    }
}

SSPTreeWidget::SSPTreeWidget(QWidget *parent) : QTreeWidget(parent)
{
    setMouseTracking(true);
    setHeaderHidden(true);
    setColumnCount(1);

    setDragEnabled(true);
    setAcceptDrops(true);
    setDropIndicatorShown(true);

    setDragDropMode(QAbstractItemView::DragDrop);
    setDefaultDropAction(Qt::CopyAction);
}

void SSPTreeWidget::startDrag(Qt::DropActions supportedActions)
{
    QTreeWidgetItem *item = currentItem();
    if (!item)
        return;

    // Allow drag of SSV and SSM items
    if (item->type() != SSVItem && item->type() != SSMItem)
        return;

    QTreeWidget::startDrag(Qt::CopyAction);
}

void SSPTreeWidget::dragMoveEvent(QDragMoveEvent *event)
{
    QTreeWidgetItem *target = itemAt(event->pos());

    static QTreeWidgetItem *lastItem = nullptr;
    if (lastItem) {
        lastItem->setBackground(0, Qt::NoBrush);
    }

    if (!target || (target->type() != FMUItem && target->type() != SystemItem))
    {
        lastItem = nullptr;
        event->ignore();
        return;
    }

    target->setBackground(0, QBrush(Qt::lightGray));
    lastItem = target;
    event->setDropAction(Qt::CopyAction);
    event->accept();
}

void SSPTreeWidget::dropEvent(QDropEvent *event)
{
    QTreeWidgetItem *target = itemAt(event->pos());

    if (!target || (target->type() != FMUItem && target->type() != SystemItem))
    {
        event->ignore();
        return;
    }

    QTreeWidgetItem *source = currentItem();

    if (!source || (source->type() != SSVItem && source->type() != SSMItem))
    {
        event->ignore();
        return;
    }

    // Create a copy of item and add it to the target
    QTreeWidgetItem *copy = new QTreeWidgetItem(source->type());
    copy->setText(0, source->text(0));
    copy->setTextColor(0, source->textColor(0));  // Preserve text color
    //! @todo We must also copy the mapping between SSV/SSM item and SSV/SSM file

    target->addChild(copy);
    target->setExpanded(true);

    event->acceptProposedAction();
}

void SSPTreeWidget::contextMenuEvent(QContextMenuEvent *event)
{
    QTreeWidgetItem *item = itemAt(event->pos());
    if (!item) {
        return;
    }

    QMenu menu(this);
    
    if((item->type() == SSVItem || item->type() == SSMItem) && item->parent()->type() != SSPItem) {
        menu.addAction("Remove from SSD", [this, item]() {
            QTreeWidgetItem *parent = item->parent();
            if(parent) {
                parent->removeChild(item);
                // @todo Also remove the mapping between SSV/SSM item and SSV/SSM file
            }
        });
    }

    menu.exec(event->globalPos());
}

void SSPWidget::openSSDModel(QTreeWidgetItem *item, int)
{
    sspHandle *ssp;
    ssdHandle *ssd;
    ssdSystemHandle *system;
    if(itemToSsdMap.contains(item)) {
        ssp = itemToSspMap[item];
        ssd = itemToSsdMap[item];
    }
    else {
        return;
    }

    system = ssp4c_ssd_getRootSystem(ssd);

    //! @todo Check so that SSD is not already open somehow

    auto pModel = gpModelHandler->addNewModel(ssp4c_ssd_getName(ssd));
    pModel->setModelType(ModelWidget::SsdModel);
    pModel->setSsdHandle(ssd);

    int componentCount = ssp4c_ssd_system_getNumberOfComponents(system);
    for(int i=0; i<componentCount; ++i) {
        ssdComponentHandle *comp = ssp4c_ssd_system_getComponentByIndex(system, i);
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

    int connetionCount = ssp4c_ssd_system_getNumberOfConnections(system);
    for(int i=0; i<connetionCount; ++i) {
        ssdConnectionHandle *con = ssp4c_ssd_system_getConnectionByIndex(system, i);
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
                if(pStartPort != nullptr && pEndPort != nullptr) {
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

