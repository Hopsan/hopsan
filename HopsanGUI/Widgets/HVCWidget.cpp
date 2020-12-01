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
//! @file   HVCWidget.cpp
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2012
//!
//! @brief Contains class for Hopsan validation widget
//!
//$Id$

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
#include "global.h"
#include "GUIObjects/GUIContainerObject.h"
#include "HVCWidget.h"
#include "ModelHandler.h"
#include "PlotHandler.h"
#include "Utilities/XMLUtilities.h"
#include "Widgets/ModelWidget.h"
#include "MessageHandler.h"

// Helpfunction to strip top-level system name from full name
QString stripTopLevelSystemName(const QString &fullName)
{
    int p = fullName.indexOf("$");
    if (p >= 0)
    {
        return fullName.right(fullName.size()-p-1);
    }
    return fullName;
}


HVCWidget::HVCWidget(QWidget *parent) :
    QDialog(parent)
{
    mpHvcOpenPathEdit = new QLineEdit();
    QPushButton *pBrowseButton = new QPushButton();
    pBrowseButton->setIcon(QIcon(QString(ICONPATH)+"svg/Hopsan-Open.svg"));

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
                // Make sure absolute, the loaded modelpath should be relative to the hvc file
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
                            conf.mTimeColumn = 0;
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
        else if (rootElement.attribute("hvcversion") == "0.2")
        {
            // Get model to load
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

                QString dataFilePath = xmlValidation.firstChildElement("hvdfile").text();
                if (dataFilePath.isEmpty())
                {
                    // Use same name as hvc file instead
                    dataFilePath = hvcFileInfo.absolutePath()+"/"+hvcFileInfo.baseName()+".hvd";
                }
                else
                {
                    // Make sure absolute, the loaded datafilepath should be relative to the hvc file
                    dataFilePath = hvcFileInfo.absolutePath()+"/"+dataFilePath;
                }
                dataFile.setFileName(dataFilePath);

                QDomElement xmlVariable = xmlValidation.firstChildElement("variable");
                while (!xmlVariable.isNull())
                {
                    HvcConfig conf;
                    conf.mDataColumn = parseDomIntegerNode(xmlVariable.firstChildElement("column"), 0);
                    conf.mTimeColumn = parseDomIntegerNode(xmlVariable.firstChildElement("timecolumn"), 0);
                    conf.mTolerance = parseDomValueNode(xmlVariable.firstChildElement("tolerance"), conf.mTolerance);
                    conf.mFullVarName = xmlVariable.attribute("name");
                    conf.mDataFile = dataFile.fileName();
                    mDataConfigs.append(conf);

                    // Populate the tree
                    mpAllVariablesTree->addFullNameVariable(conf.mFullVarName);
                    mpSelectedVariablesTree->addFullNameVariable(conf.mFullVarName);

                    // Next variable to check
                    xmlVariable = xmlVariable.nextSiblingElement("variable");
                }
            }
            else
            {
                //! @todo some error
            }

        }
        else
        {
            // Unsupported format
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
    gpModelHandler->loadModel(mModelFilePath, ModelHandler::IgnoreAlreadyOpen);
    // Switch to that tab

    // Get Log data handler for model
    int simuGen=-1;
    LogDataHandler2 *pLogDataHandler = gpModelHandler->getCurrentLogDataHandler().data();

    // Simulate the system
    if (gpModelHandler->getCurrentModel())
    {
        gpModelHandler->getCurrentModel()->simulate_blocking();
        simuGen = pLogDataHandler->getCurrentGenerationNumber();
    }


    // Compare each variable
    gpPlotHandler->closeAllOpenWindows(); //Close all plot windows to avoid confusion if we run several tests after each other
    for (int t=0; t<mDataConfigs.size(); ++t)
    {
//        QString variableName = mDataConfigs[t].mFullVarName;

//        // Handle subsystem variables
//        if (variableName.contains('$'))
//        {
//            QStringList fields = variableName.split('$');
//            fields.erase(fields.begin()); // Remove the first "top-level system" name
//            ContainerObject *pContainer=gpModelHandler->getCurrentTopLevelSystem();
//            while (!fields.front().contains('#'))
//            {
//                ModelObject *pObj = pContainer->getModelObject(fields.front());
//                if (pObj && (pObj->type() == SystemContainerType))
//                {
//                    pContainer = qobject_cast<ContainerObject*>(pObj);
//                }
//                else
//                {
//                    gpMessageHandler->addErrorMessage(QString("Could not find system: %1").arg(fields.front()));
//                    return;
//                }
//                fields.erase(fields.begin());
//            }
//            variableName = fields.front();
//        }

        QVector<int> columns, timecolumns;
        QStringList names;
        columns.append(mDataConfigs[t].mDataColumn);
        timecolumns.append(mDataConfigs[t].mTimeColumn);
        names.append(mDataConfigs[t].mFullVarName+"_valid");

        pLogDataHandler->importTimeVariablesFromCSVColumns(mDataConfigs[t].mDataFile, columns, names, timecolumns);
        int importGen = pLogDataHandler->getCurrentGenerationNumber();

        QString windowName = QString("Validation Plot %1").arg(t);
        gpPlotHandler->createNewOrReplacePlotwindow(windowName);

        SharedVectorVariableT pSimVar = pLogDataHandler->getVectorVariable(mDataConfigs[t].mFullVarName, simuGen);
        // In previous versions of the HopsanCLI full names included the root system name in the begining.
        // The LogdataHandler does not do that, so if we did not find anything we need to retry with the first part removed
        if (!pSimVar)
        {
            pSimVar = pLogDataHandler->getVectorVariable(stripTopLevelSystemName(mDataConfigs[t].mFullVarName), simuGen);
        }

        gpPlotHandler->plotDataToWindow(windowName, pSimVar, 0);
        gpPlotHandler->plotDataToWindow(windowName, pLogDataHandler->getVectorVariable(mDataConfigs[t].mFullVarName+"_valid", importGen), 0);
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
                    // We assume there will never be duplicates due to unique name requirements
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
                // We assume there will never be duplicates due to unique name requirements
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
