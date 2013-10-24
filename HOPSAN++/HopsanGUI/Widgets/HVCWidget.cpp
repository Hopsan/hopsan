/*-----------------------------------------------------------------------------
 This source file is part of Hopsan NG

 Copyright (c) 2011
    Mikael Axin, Robert Braun, Alessandro Dell'Amico, Björn Eriksson,
    Peter Nordin, Karl Pettersson, Petter Krus, Ingo Staack

 This file is provided "as is", with no guarantee or warranty for the
 functionality or reliability of the contents. All contents in this file is
 the original work of the copyright holders at the Division of Fluid and
 Mechatronic Systems (Flumes) at Linköping University. Modifying, using or
 redistributing any part of this file is prohibited without explicit
 permission from the copyright holders.
-----------------------------------------------------------------------------*/

//!
//! @file   HVCWidget.cpp
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2012
//!
//! @brief Contains class for Hopsan validation widget
//!
//$Id: LibraryWidget.cpp 5499 2013-06-04 10:52:33Z robbr48 $

//Qt includes
#include <QCheckBox>
#include <QLabel>
#include <QFileDialog>
#include <QToolButton>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPushButton>

//Hopsan includes
#include "common.h"
#include "GUIObjects/GUISystem.h"
#include "HVCWidget.h"
#include "LogDataHandler.h"
#include "ModelHandler.h"
#include "PlotHandler.h"
#include "Utilities/XMLUtilities.h"
#include "Widgets/ModelWidget.h"


HVCWidget::HVCWidget(QWidget *parent) :
    QDialog(parent)
{
    mpHvcOpenPathEdit = new QLineEdit();
    QPushButton *pBrowseButton = new QPushButton();
    pBrowseButton->setIcon(QIcon(QString(ICONPATH)+"Hopsan-Open.png"));

    QHBoxLayout *pOpenFileHLayout = new QHBoxLayout();
    pOpenFileHLayout->addWidget(new QLabel("HVC: "));
    pOpenFileHLayout->addWidget(mpHvcOpenPathEdit);
    pOpenFileHLayout->addWidget(pBrowseButton);

    QCheckBox *pFoundModelCheckBox = new QCheckBox();
    QCheckBox *pFoundDataCheckBox = new QCheckBox();
    QHBoxLayout *pOpenStatusHLayout = new QHBoxLayout();
    pOpenStatusHLayout->addWidget(new QLabel("Found model: "));
    pOpenStatusHLayout->addWidget(pFoundModelCheckBox);
    pOpenStatusHLayout->addWidget(new QLabel("Found data: "));
    pOpenStatusHLayout->addWidget(pFoundDataCheckBox);


    mpAllVariablesTree = new FullNameVariableTreeWidget();
    mpSelectedVariablesTree = new FullNameVariableTreeWidget();
    QHBoxLayout *pVariablesHLayout = new QHBoxLayout();

    pVariablesHLayout->addWidget(mpAllVariablesTree);
    pVariablesHLayout->addSpacing(10);
    pVariablesHLayout->addWidget(mpSelectedVariablesTree);

    QHBoxLayout *pButtonLayout = new QHBoxLayout();
    QPushButton *pSaveButton = new QPushButton("Save hvc");
    pSaveButton->setDisabled(true);
    QPushButton *pCloseButton = new QPushButton("Close");
    QPushButton *pCompareButton = new QPushButton("Compare");
    pButtonLayout->addWidget(pCompareButton);
    pButtonLayout->addWidget(pSaveButton);
    pButtonLayout->addWidget(pCloseButton);

    QVBoxLayout *pMainLayout = new QVBoxLayout();
    pMainLayout->addWidget(new QLabel("Not yet finished, but you can use the compare function"));
    pMainLayout->addWidget(new QLabel("You can generate new HVC files from the plotwindow"));
    pMainLayout->addLayout(pOpenFileHLayout);
    pMainLayout->addLayout(pOpenStatusHLayout);
    pMainLayout->addLayout(pVariablesHLayout);
    pMainLayout->addLayout(pButtonLayout);

    this->setLayout(pMainLayout);

    connect(pCloseButton, SIGNAL(clicked()), this, SLOT(close()));
    connect(pBrowseButton, SIGNAL(clicked()), this, SLOT(openHvcFile()));
    connect(pCompareButton, SIGNAL(clicked()), this, SLOT(runHvcTest()));

    this->resize(800, 600);
    this->setWindowTitle("Hopsan Model Validation");

}

void HVCWidget::openHvcFile()
{
    QFile hvcFile;
    hvcFile.setFileName(QFileDialog::getOpenFileName(this, "Select a HVC file"));
    if (hvcFile.exists())
    {
        // Clear
        clearContents();

        QFileInfo hvcFileInfo = QFileInfo(hvcFile);
        QDomDocument xmlDocument;
        QDomElement rootElement = loadXMLDomDocument(hvcFile, xmlDocument, "hopsanvalidationconfiguration");
        if (rootElement.isNull())
        {
            //! @todo some error message
            return;
        }

        QFile hmfFile, dataFile;
        if (rootElement.attribute("hvcversion") == "0.1")
        {
            // Load old format
            QDomElement xmlValidation = rootElement.firstChildElement("validation");
            QString modelFilePath = xmlValidation.firstChildElement("modelfile").text();
            if (modelFilePath.isEmpty())
            {
                // Use same name as hvc file instead
                modelFilePath = hvcFileInfo.absolutePath()+"/"+hvcFileInfo.baseName()+".hmf";
            }
            else
            {
                // Make sure absoulte, the loaded modelpath should be relative to the hvc file
                modelFilePath = hvcFileInfo.absolutePath()+"/"+modelFilePath;
            }
            hmfFile.setFileName(modelFilePath);

            if (hmfFile.exists())
            {
                mModelFilePath = modelFilePath;
                QDomElement xmlComponent = xmlValidation.firstChildElement("component");
                while (!xmlComponent.isNull())
                {
                    QDomElement xmlPort = xmlComponent.firstChildElement("port");
                    while (!xmlPort.isNull())
                    {
                        QString dataFilePath = xmlComponent.firstChildElement("csvfile").text();
                        if (dataFilePath.isEmpty())
                        {
                            // Use same name as hvc file instead
                            dataFilePath = hvcFileInfo.absolutePath()+"/"+hvcFileInfo.baseName()+".csv";
                        }
                        dataFile.setFileName(dataFilePath);

                        QDomElement xmlVariable = xmlPort.firstChildElement("variable");
                        while (!xmlVariable.isNull())
                        {
                            // check if we should override data file
                            if (!xmlComponent.firstChildElement("csvfile").isNull())
                            {
                                QString dataFilePath = xmlComponent.firstChildElement("csvfile").text();
                                if (dataFilePath.isEmpty())
                                {
                                    // Use same name as hvc file instead
                                    dataFilePath = hvcFileInfo.absolutePath()+"/"+hvcFileInfo.baseName()+".csv";
                                }
                                dataFile.setFileName(dataFilePath);
                            }

                            HvcConfig conf;
                            conf.mDataColumn = parseDomIntegerNode(xmlVariable.firstChildElement("column"), 0);
                            conf.mTolerance = parseDomValueNode(xmlVariable.firstChildElement("tolerance"), conf.mTolerance);
                            conf.mFullVarName = xmlComponent.attribute("name")+"#"+xmlPort.attribute("name")+"#"+xmlVariable.attribute("name");
                            conf.mDataFile = dataFile.fileName();
                            mDataConfigs.append(conf);

                            // Populate the tree
                            mpAllVariablesTree->addFullNameVariable(conf.mFullVarName);
                            mpSelectedVariablesTree->addFullNameVariable(conf.mFullVarName);

                            // Next variable to check
                            xmlVariable = xmlVariable.nextSiblingElement("variable");
                        }
                        xmlPort = xmlPort.nextSiblingElement("port");
                    }
                    xmlComponent = xmlComponent.nextSiblingElement("component");
                }
            }
            else
            {
                //! @todo some error
            }
        }
        else
        {
            // Load new format
            //! @todo

        }


        // Show file in line edit
        mpHvcOpenPathEdit->setText(hvcFile.fileName());

        // Find model and any data files, update status

        // Populate tree

    }
}

void HVCWidget::clearContents()
{
    mpHvcOpenPathEdit->clear();
    mpAllVariablesTree->clear();
    mpSelectedVariablesTree->clear();
    mModelFilePath.clear();
    mDataConfigs.clear();
}

void HVCWidget::runHvcTest()
{
    // First load the model
    gpModelHandler->loadModel(mModelFilePath, true);
    // Switch to that tab

    // Simulate the system
    gpModelHandler->getCurrentModel()->simulate_blocking();

    // Run each test
    for (int t=0; t<mDataConfigs.size(); ++t)
    {
        LogDataHandler *pLogDataHandler = gpModelHandler->getCurrentTopLevelSystem()->getLogDataHandler();

        QVector<int> columns;
        QStringList names;
        columns.append(mDataConfigs[t].mDataColumn);
        names.append(mDataConfigs[t].mFullVarName+"_valid");

        pLogDataHandler->importTimeVariablesFromCSVColumns(mDataConfigs[t].mDataFile, columns, names, 0);

        QString windowName = QString("Validation Plot %1").arg(t);
        gpPlotHandler->createPlotWindow(windowName);
        gpPlotHandler->plotDataToWindow(windowName, pLogDataHandler->getLogVariableDataPtr(mDataConfigs[t].mFullVarName,-1), 0);
        gpPlotHandler->plotDataToWindow(windowName, pLogDataHandler->getLogVariableDataPtr(mDataConfigs[t].mFullVarName+"_valid",-1), 0);
    }
}


void FullNameVariableTreeWidget::addFullNameVariable(const QString &rFullName, const QString &rRemaningName, QTreeWidgetItem *pParentItem)
{
    if (!rFullName.isEmpty())
    {
        bool isLeaf=false;
        QStringList systems = rRemaningName.split("$");
        QString name;

        // If we have a system part
        if (systems.size() > 1)
        {
            name = systems.first();
        }
        // Else we have a comp::port::var
        else
        {
            QStringList compportvar = rRemaningName.split("#");
            if (compportvar.size()==1)
            {
                isLeaf = true;
            }
            name = compportvar.first();
        }


        QTreeWidgetItem* pLevelItem=0;
        // Add a new level in the tree
        if (pParentItem)
        {
            QTreeWidgetItem* pFound=0;
            for (int c=0; c<pParentItem->childCount(); ++c)
            {
                if (pParentItem->child(c)->text(0) == name)
                {
                    // We assume there will never be duplicates due to uniqe name requirements
                    pFound = pParentItem->child(c);
                    break;
                }
            }

            if (pFound)
            {
                pLevelItem = pFound;
            }
            else
            {
                // Add new item
                pLevelItem = new QTreeWidgetItem(QStringList(name));
                pParentItem->addChild(pLevelItem);
            }
        }
        else
        {
            QList<QTreeWidgetItem*> found = this->findItems(name, 0);
            if (found.isEmpty())
            {
                // Add new top level item
                pLevelItem = new QTreeWidgetItem(QStringList(name));
                addTopLevelItem(pLevelItem);
            }
            else
            {
                // We assume there will never be duplicates due to uniqe name requirements
                pLevelItem = found.first();
            }

        }

        if (isLeaf)
        {
            pLevelItem->setData(0,Qt::UserRole,rFullName);
        }
        else
        {
            // Recurse
            QString shorterRemaningName = rRemaningName;
            shorterRemaningName.remove(0, name.size()+1);
            addFullNameVariable(rFullName, shorterRemaningName, pLevelItem);
        }
    }
}


FullNameVariableTreeWidget::FullNameVariableTreeWidget(QWidget *pParent) : QTreeWidget(pParent)
{
    //Nothing
}

void FullNameVariableTreeWidget::addFullNameVariable(const QString &rFullName)
{
    addFullNameVariable(rFullName, rFullName, 0);
}

//void FullNameVariableTreeWidget::mousePressEvent(QMouseEvent *event)
//{
//    QTreeWidget::mousePressEvent(event);

//    //! @todo dragstartposition
//    QTreeWidgetItem *pItem = this->itemAt(event->pos());

//    if (pItem && (pItem->childCount() == 0))
//    {
//        QDrag *pDrag = new QDrag(this);
//        QMimeData *pMime = new QMimeData;

//        pMime->setText(pItem->data(0, Qt::UserRole).toString());
//        pDrag->setMimeData(pMime);

//        Qt::DropAction dropAction = pDrag->exec(Qt::CopyAction | Qt::MoveAction);
//    }
//}

//void FullNameVariableTreeWidget::dropEvent(QDropEvent *event)
//{
//    if (!event->mimeData()->text().isEmpty())
//    {
//        addFullNameVariable(event->mimeData()->text());
//        event->acceptProposedAction();
//    }
//}

//void FullNameVariableTreeWidget::dragEnterEvent(QDragEnterEvent *event)
//{
//    if (!event->mimeData()->text().isEmpty())
//    {
//        event->acceptProposedAction();
//    }
//}
