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
//! @file   PlotWidget.cpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-XX-XX
//!
//! @brief Contains the PlotWidget and otehr plot related classes
//!
//$Id$

//Qt includes
#include <QDebug>
#include <QSpinBox>
#include <QColorDialog>
#include <QLabel>
#include <QCursor>
#include <QAction>
#include <QTextStream>

//Hopsan includes
#include "Configuration.h"
#include "GraphicsView.h"
#include "GUIObjects/GUIContainerObject.h"
#include "GUIObjects/GUIModelObject.h"
#include "GUIObjects/GUISystem.h"
#include "GUIPort.h"
#include "LibraryWidget.h"
#include "loadFunctions.h"
#include "MainWindow.h"
#include "MessageWidget.h"
#include "PlotHandler.h"
#include "PlotWidget.h"
#include "PlotWindow.h"
#include "ProjectTabWidget.h"
#include "Utilities/GUIUtilities.h"


//! @brief Constructor for the variable items in the variable tree
//! @param componentName Name of the component where the variable is located
//! @param portName Name of the port where the variable is located
//! @param dataName Name of the variable
//! @param dataUnit Name of the unit of the variable
//! @param parent Pointer to a tree widget item, not used
PlotVariableTreeItem::PlotVariableTreeItem(SharedLogVariableDataPtrT pData, QTreeWidgetItem *parent)
        : QTreeWidgetItem(parent)
{
    mpData = pData;
    QString dataUnit = pData->getDataUnit();
    QString alias = pData->getAliasName();

    if (parent->text(0) == "__Alias__") //!< @todo Ok This is a really ugly hack /Peter
    {
        this->setText(0, alias + ", [" + dataUnit + "]");
    }
    else if (parent->text(0) == "__Imported__")
    {
        this->setText(0, pData->getFullVariableName() + ", [" + dataUnit + "]");
    }
    else
    {
        QString portName = pData->getPortName();
        QString dataName = pData->getDataName();
        if(!alias.isEmpty())
        {
            alias.prepend("<");
            alias.append("> ");
        }
        if (portName.isEmpty())
        {
            this->setText(0, alias + dataName + ", [" + dataUnit + "]");
        }
        else
        {
            this->setText(0, alias + portName + ", " + dataName + ", [" + dataUnit + "]");
        }
    }
}

SharedLogVariableDataPtrT PlotVariableTreeItem::getDataPtr()
{
    return mpData;
}

QString PlotVariableTreeItem::getFullName() const
{
    return mpData->getFullVariableName();
}


//! @brief Returns the name of the component where the variable is located
QString PlotVariableTreeItem::getComponentName()
{
    return mpData->getComponentName();
}


//! @brief Returns the name of the port where the variable is located
QString PlotVariableTreeItem::getPortName()
{
    return mpData->getPortName();
}


//! @brief Returns the name of the variable
QString PlotVariableTreeItem::getDataName()
{
    return mpData->getDataName();
}


//! @brief Returns the name of the unit of the variable
QString PlotVariableTreeItem::getDataUnit()
{
    return mpData->getDataUnit();
}

QString PlotVariableTreeItem::getAliasName()
{
    return mpData->getAliasName();
}


//! @brief Constructor for the variable tree widget
//! @param parent Pointer to the main window
PlotVariableTree::PlotVariableTree(QWidget *pParent)
        : QTreeWidget(pParent)
{
    mpLogDataHandler = 0;

    this->setDragEnabled(true);
    this->setAcceptDrops(false);
    this->setHeaderHidden(true);
    this->setColumnCount(1);
    this->setMouseTracking(true);

    connect(this, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(createPlotWindow(QTreeWidgetItem*)));
    this->updateList();
}

void PlotVariableTree::setLogDataHandler(QPointer<LogDataHandler> pLogDataHandler)
{
    if (mpLogDataHandler)
    {
        disconnect(mpLogDataHandler, 0, this, 0);
    }

    mpLogDataHandler = pLogDataHandler;
    connect(mpLogDataHandler, SIGNAL(newDataAvailable()), this, SLOT(updateList()));
    updateList();
}

LogDataHandler *PlotVariableTree::getLogDataHandler()
{
    return mpLogDataHandler;
}


//! @brief Updates the variable tree to the available components and variables in the current tab.
void PlotVariableTree::updateList()
{
    QStringList expandedItems;

    QTreeWidgetItemIterator it(this);
    while(*it)
    {
        if ((*it)->isExpanded())
        {
            expandedItems << (*it)->text(0);
        }
        ++it;
    }

//    for(int i=0; i<this->item; ++i)
//    {
//        if(items[i]->isExpanded())
//        {
//            expandedItems << items[i]->text(0);
//        }
//    }

    mAvailableVariables.clear();
    this->clear();

    if(mpLogDataHandler == 0)     //Check so that we have something to represent
    {
        return;
    }


    QTreeWidgetItem *pComponentLevelItem;             //Tree item for components
    QTreeWidgetItem *pAliasLevelItem=0;               //Tree item for aliases
    QTreeWidgetItem *pImportedLevelItem=0;               //Tree item for imports
    PlotVariableTreeItem *plotVariableLevelItem;      //Tree item for variables - reimplemented so they can store information about the variable

    QStringList aliasLevelItemList, importLevelItemList;
    QMap<QString, QTreeWidgetItem*> importedLevelItemMap;
    QMap<QString, QTreeWidgetItem*> componentLevelItemMap;
    QMap<QString, QTreeWidgetItem*>::iterator cilIt;
    QVector<SharedLogVariableDataPtrT> variables = mpLogDataHandler->getAllVariablesAtNewestGeneration();
    for(int i=0; i<variables.size(); ++i)
    {
        if (variables[i]->getVariableDescription()->mVariableSourceType == VariableDescription::TempVariableType)
        {
            continue;
        }

        // Check if this is an alias variable, if alias is set and not already in the aliasLevelItemMap map
        if ( !variables[i]->getAliasName().isEmpty() && (aliasLevelItemList.count(variables[i]->getAliasName()) < 1) )
        {
            // Create the __Alias__ top level item the fioirst time it is needed
            if (!pAliasLevelItem)
            {
                pAliasLevelItem = new QTreeWidgetItem();
                pAliasLevelItem->setText(0, "__Alias__");
                QFont tempFont = pAliasLevelItem->font(0);
                tempFont.setBold(true);
                pAliasLevelItem->setFont(0, tempFont);
                addTopLevelItem(pAliasLevelItem);
                pAliasLevelItem->setExpanded(true);
            }

            // Add a sub item with alias name
            new PlotVariableTreeItem(variables[i], pAliasLevelItem);
            aliasLevelItemList.append(variables[i]->getAliasName());
        }
        // Handle if variable is imported
        else if (variables[i]->getVariableDescription()->mVariableSourceType == VariableDescription::ImportedVariableType)
        {
            if (!pImportedLevelItem)
            {
                pImportedLevelItem = new QTreeWidgetItem();
                pImportedLevelItem->setText(0, "__Imported__");
                QFont tempFont = pImportedLevelItem->font(0);
                tempFont.setBold(true);
                pImportedLevelItem->setFont(0, tempFont);
                addTopLevelItem(pImportedLevelItem);
            }

            QString fName = variables[i]->getVariableDescription()->mImportFileName;
            QTreeWidgetItem *pImportFileLevelItem = importedLevelItemMap.value(variables[i]->getVariableDescription()->mImportFileName, 0);
            // If this file is not alrady added then create it
            if (!pImportFileLevelItem)
            {
                pImportFileLevelItem = new QTreeWidgetItem();
                pImportFileLevelItem->setText(0, fName);
                QFont tempFont = pComponentLevelItem->font(0);
                tempFont.setBold(true);
                pImportFileLevelItem->setFont(0, tempFont);
//                this->addTopLevelItem(pImportFileLevelItem);
                pComponentLevelItem->setExpanded(expandedItems.contains(pImportedLevelItem->text(0)));

                //Also remember that we created it
                importedLevelItemMap.insert(fName, pImportFileLevelItem);
                pImportedLevelItem->addChild(pImportFileLevelItem);
            }

            // Add a sub item with data name name
            new PlotVariableTreeItem(variables[i], pImportFileLevelItem);
            //importLevelItemList.append(variables[i]->getFullVariableName());
        }
        else
        // Ok, we do not need to add this item as an alias
        {
            // Did we have the top-level component item (in that case use it)
            QString cname = variables[i]->getComponentName();
            if (cname.isEmpty())
            {
                // Mostly for the timevariable
                cname = variables[i]->getFullVariableName();
            }
            cilIt = componentLevelItemMap.find(cname);
            if (cilIt != componentLevelItemMap.end())
            {
                pComponentLevelItem = cilIt.value();
            }
            else
            // Ok, we did not find it, then this is the first time the component is added, lets create and add that top-lvel item
            {
                pComponentLevelItem = new QTreeWidgetItem();
                pComponentLevelItem->setText(0, cname);
                QFont tempFont = pComponentLevelItem->font(0);
                tempFont.setBold(true);
                pComponentLevelItem->setFont(0, tempFont);
                this->addTopLevelItem(pComponentLevelItem);
                pComponentLevelItem->setExpanded(expandedItems.contains(pComponentLevelItem->text(0)));

                //Also remember that we created it
                componentLevelItemMap.insert(variables[i]->getComponentName(), pComponentLevelItem);
            }

            plotVariableLevelItem = new PlotVariableTreeItem(variables[i], pComponentLevelItem);
            pComponentLevelItem->addChild(plotVariableLevelItem);
        }

        // prepend icon if favourite variable
        //if(mpCurrentContainer->getPlotDataPtr()->getFavoriteVariableList().contains(variableDescription))
        {
            //tempPlotVariableTreeItem->setIcon(0, QIcon(QString(ICONPATH) + "Hopsan-Favorite.png"));
        }
        //! @todo FIXA /Peter
    }

//    QVector<double> time;
//    bool timeVectorRetained = false;
//    QStringList names = mpCurrentContainer->getModelObjectNames();
//    for(int i=0; i<names.size(); ++i)
//    {
//        ModelObject *pComponent = mpCurrentContainer->getModelObject(names[i]);
//        tempComponentItem = new QTreeWidgetItem();
//        tempComponentItem->setText(0, pComponent->getName());
//        QFont tempFont;
//        tempFont = tempComponentItem->font(0);
//        tempFont.setBold(true);
//        tempComponentItem->setFont(0, tempFont);
//        this->addTopLevelItem(tempComponentItem);

//        QList<Port*> portListPtrs = pComponent->getPortListPtrs();
//        QList<Port*>::iterator itp;
//        for(itp = portListPtrs.begin(); itp !=portListPtrs.end(); ++itp)
//        {
//            //If the port is not connected it has nothing to plot
//            if((*itp)->isConnected())
//            {
//                QVector<QString> variableNames;
//                QVector<QString> variableUnits;
//                gpModelHandler->getCurrentContainer()->getCoreSystemAccessPtr()->getPlotDataNamesAndUnits((*itp)->getGuiModelObjectName(), (*itp)->getName(), variableNames, variableUnits);
//                if(!timeVectorRetained)
//                {
//                    time = QVector<double>::fromStdVector(gpModelHandler->getCurrentContainer()->getCoreSystemAccessPtr()->getTimeVector((*itp)->getGuiModelObjectName(), (*itp)->getName()));
//                    timeVectorRetained = true;
//                }
//                if(time.size() > 0)     //If time vector is greater than zero we have something to plot!
//                {
//                    for(int i = 0; i!=variableNames.size(); ++i)
//                    {
//                        variableUnits[i] = gConfig.getDefaultUnit(variableNames[i]);
//                        tempPlotVariableTreeItem = new PlotVariableTreeItem(pComponent->getName(), (*itp)->getName(), variableNames[i], variableUnits[i], tempComponentItem);
//                        tempComponentItem->addChild(tempPlotVariableTreeItem);
//                        VariableDescription variableDescription;
//                        variableDescription.componentName = (*itp)->getGuiModelObjectName();
//                        variableDescription.portName = (*itp)->getName();
//                        variableDescription.dataName = variableNames[i];
//                        variableDescription.dataUnit = variableUnits[i];
//                        mAvailableVariables.append(variableDescription);
//                        if(gpModelHandler->getCurrentContainer()->getPlotDataPtr()->getFavoriteVariableList().contains(variableDescription))
//                        {
//                            tempPlotVariableTreeItem->setIcon(0, QIcon(QString(ICONPATH) + "Hopsan-Favorite.png"));
//                        }
//                    }
//                }
//            }
//        }
//    }

        //Append favorite plot variables to tree if they still exist
    for(int i=0; i<mpLogDataHandler->getFavoriteVariableList().size(); ++i)
    {
//        QString componentName = mpCurrentContainer->getPlotDataPtr()->getFavoriteVariableList().at(i).componentName;
//        QString portName = mpCurrentContainer->getPlotDataPtr()->getFavoriteVariableList().at(i).portName;
//        QString dataName = mpCurrentContainer->getPlotDataPtr()->getFavoriteVariableList().at(i).dataName;
//        QString dataUnit = mpCurrentContainer->getPlotDataPtr()->getFavoriteVariableList().at(i).dataUnit;
//        QString alias = mpCurrentContainer->getPlotDataPtr()->getPlotAlias(componentName, portName, dataName);

//        if(!componentName.isEmpty())
        {
//            tempPlotVariableTreeItem = new PlotVariableTreeItem(componentName, portName, dataName, dataUnit);
//            tempPlotVariableTreeItem->setText(0, " <"+alias+"> "+componentName+", "+portName+", "+dataName+", ["+dataUnit+"]");
//            tempPlotVariableTreeItem->setIcon(0, QIcon(QString(ICONPATH) + "Hopsan-Favorite.png"));
//            this->addTopLevelItem(tempPlotVariableTreeItem);
//            tempPlotVariableTreeItem->setDisabled(!mAvailableVariables.contains(gpModelHandler->getCurrentContainer()->getPlotDataPtr()->getFavoriteVariableList().at(i)));
        }
    }

    // Remove no longer existing favorite variables
    for(int i=0; i<mpLogDataHandler->getFavoriteVariableList().size(); ++i)
    {
        if(!mAvailableVariables.contains(mpLogDataHandler->getFavoriteVariableList().at(i)))
        {
           // gpModelHandler->getCurrentContainer()->getPlotDataPtr()->getFavoriteVariableList().removeAll(gpModelHandler->getCurrentTopLevelSystem()->getPlotDataPtr()->getFavoriteVariableList().at(i));
        }
    }

    //Sort the tree widget
    this->sortItems(0, Qt::AscendingOrder);

    // This connection makes sure that the plot list is connected to the new tab, so that it will update if the new tab is simulated.
    // It must first be disconnected in case it was already connected, to avoid duplication of connection.
//    disconnect(gpModelHandler->getCurrentModel(),    SIGNAL(simulationFinished()), this, SLOT(updateList()));
//    disconnect(gpCentralTabWidget,                     SIGNAL(simulationFinished()), this, SLOT(updateList()));
//    connect(gpModelHandler->getCurrentModel(),       SIGNAL(simulationFinished()), this, SLOT(updateList()));
//    connect(gpCentralTabWidget,                        SIGNAL(simulationFinished()), this, SLOT(updateList()));
}


//! @brief Helper function that creates a new plot window by using a QTreeWidgetItem in the plot variable tree.
//! @param *item Pointer to the tree widget item whos arrays will be looked up from the map and plotted
PlotWindow *PlotVariableTree::createPlotWindow(QTreeWidgetItem *item)
{
    // QTreeWidgetItem must be casted to a PlotVariableTreeItem. This is a necessary because double click event can not know which kind of tree item is clicked.
    // Top level items cannot be plotted (they represent the components)
    PlotVariableTreeItem *tempItem = dynamic_cast<PlotVariableTreeItem *>(item);
    if(tempItem)
    {
        return gpPlotHandler->plotDataToWindow(0, tempItem->getDataPtr(), QwtPlot::yLeft);
    }
    return 0; //! @todo Should this return 0?
}

//! @brief Defines what happens when clicking in the variable list. Used to initiate drag operations.
void PlotVariableTree::mousePressEvent(QMouseEvent *event)
{
    qDebug() << "mousePressEvent";

    gpMainWindow->showHelpPopupMessage("Double click on a variable to open a new plot window, or drag it to an existing one.");
    QTreeWidget::mousePressEvent(event);

    if (event->button() == Qt::LeftButton)
        dragStartPosition = event->pos();
}


//! @brief Defines what happens when mouse is moving in variable list. Used to handle drag operations.
void PlotVariableTree::mouseMoveEvent(QMouseEvent *event)
{
    //qDebug() << "mouseMoveEvent";

    this->setFrameShape(QFrame::Box);

    gpMainWindow->showHelpPopupMessage("Double click on a variable to open a new plot window, or drag it to an existing one.");
    if (!(event->buttons() & Qt::LeftButton))
    {
        return;
    }
    if ((event->pos() - dragStartPosition).manhattanLength() < QApplication::startDragDistance())
    {
        return;
    }

    PlotVariableTreeItem *item;
    item = dynamic_cast<PlotVariableTreeItem *>(itemAt(dragStartPosition.toPoint()));

    if(item != 0)
    {
        QString mimeText;
        mimeText = QString("HOPSANPLOTDATA:"+item->getFullName());
        QDrag *drag = new QDrag(this);
        QMimeData *mimeData = new QMimeData;
        mimeData->setText(mimeText);
        drag->setMimeData(mimeData);
        drag->exec();
    }

    QTreeWidget::mouseMoveEvent(event);
}


//! @brief Defines the right-click menu in the variable tree
void PlotVariableTree::contextMenuEvent(QContextMenuEvent */*event*/)
{
    qDebug() << "contextMenuEvent()";

    PlotVariableTreeItem *pItem = dynamic_cast<PlotVariableTreeItem *>(currentItem());
    if(pItem)
    {
        qDebug() << "currentItem() is ok!";

        QMenu menu;
        QAction *pDefineAliasAction = 0;
        QAction *pRemoveAliasAction = 0;
        QAction *pAddToFavoritesAction = 0;
        QAction *pRemoveFromFavoritesAction = 0;

        if(pItem->getAliasName().isEmpty())
        {
            pDefineAliasAction = menu.addAction(QString("Define Variable Alias"));
        }
        else
        {
            pDefineAliasAction = menu.addAction(QString("Change Variable Alias"));
            pRemoveAliasAction = menu.addAction(QString("Remove Variable Alias"));
        }

//! @todo FIXA /Peter
//        if(!gpModelHandler->getCurrentContainer()->getPlotDataPtr()->getFavoriteVariableList().contains(variableDescription))
//        {
//            addToFavoritesAction = menu.addAction(QString("Add Favorite Variable"));
//        }
//        else
//        {
//            removeFromFavoritesAction = menu.addAction(QString("Remove Favorite Variable"));
//        }

        //-- Action --//
        QCursor *cursor;
        QAction *selectedAction = menu.exec(cursor->pos());
        //------------//

        if(selectedAction == pRemoveAliasAction)
        {
           mpLogDataHandler->undefinePlotAlias(pItem->getAliasName());
           this->updateList();
        }

        if(selectedAction == pDefineAliasAction)
        {
            mpLogDataHandler->definePlotAlias(pItem->getFullName());
            this->updateList();
        }

        if(selectedAction == pAddToFavoritesAction)
        {
            //! @todo FIXA /Peter
            //gpModelHandler->getCurrentContainer()->getPlotDataPtr()->setFavoriteVariable(variableDescription.componentName, variableDescription.portName, variableDescription.dataName, variableDescription.dataUnit);
        }

        if(selectedAction == pRemoveFromFavoritesAction)
        {
           //! @todo FIXA /Peter
           //gpModelHandler->getCurrentContainer()->getPlotDataPtr()->removeFavoriteVariableByComponentName(pItem->getComponentName());
           //this->updateList();
        }
    }
}


//! @brief Constructor the main plot widget, which contains the tree with variables
//! @param parent Pointer to the main window
PlotTreeWidget::PlotTreeWidget(QWidget *pParent)
        : QWidget(pParent)
{
    mpPlotVariableTree = new PlotVariableTree(this);

    this->setMouseTracking(true);

    mpNewWindowButton = new QPushButton(tr("&Open New Plot Window"), this);
    mpNewWindowButton->setAutoDefault(false);
    mpNewWindowButton->setFixedHeight(30);
    QFont tempFont = mpNewWindowButton->font();
    tempFont.setBold(true);
    mpNewWindowButton->setFont(tempFont);

    mpLoadButton = new QPushButton(tr("&Load Plot Window from XML"), this);
    mpLoadButton->setAutoDefault(false);
    mpLoadButton->setFixedHeight(30);
    tempFont = mpLoadButton->font();
    tempFont.setBold(true);
    mpLoadButton->setFont(tempFont);

    mpLayout = new QGridLayout(this);
    mpLayout->addWidget(mpPlotVariableTree,0,0,1,1);
    mpLayout->addWidget(mpNewWindowButton, 1, 0, 1, 1);
    mpLayout->addWidget(mpLoadButton, 2, 0, 1, 1);
    mpLayout->setContentsMargins(4,4,4,4);

    connect(mpNewWindowButton, SIGNAL(clicked()), this, SLOT(openNewPlotWindow()));
    connect(mpLoadButton, SIGNAL(clicked()),this,SLOT(loadFromXml()));
    mpLoadButton->setHidden(true);      //!< @todo Fix /Peter
    //mpLoadButton->setDisabled(true);

    connect(gpLibraryWidget, SIGNAL(hovered()), this, SLOT(clearHoverEffects()));
}

void PlotTreeWidget::openNewPlotWindow()
{
    gpPlotHandler->createPlotWindow();
}


//! Loads a plot window from a specified .hpw file. Loads the actual plot data from a .xml file.
void PlotTreeWidget::loadFromXml()
{
//! @todo FIXA /Peter
//    QDir fileDialogOpenDir;
//    QString fileName = QFileDialog::getOpenFileName(this, tr("Load Plot Window Description From XML"),
//                                                         fileDialogOpenDir.currentPath(),
//                                                         tr("Plot Window Description File (*.xml)"));
//    if(fileName.isEmpty())                                                                      //User did not select a file
//    {
//        return;
//    }
//    QFile file(fileName);                                                                       //File is not readable
//    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
//    {
//        QMessageBox::information(gpMainWindow->window(), gpMainWindow->tr("Hopsan"),
//                                 "Unable to read XML file.\n");
//        return;
//    }

//    QDomDocument domDocument;
//    QString errorStr;
//    int errorLine, errorColumn;
//    if (!domDocument.setContent(&file, false, &errorStr, &errorLine, &errorColumn))             //Parse error in file
//    {
//        QMessageBox::information(gpMainWindow->window(), gpMainWindow->tr("Hopsan"),
//                                 QString(file.fileName() + "Parse error at line %1, column %2:\n%3")
//                                 .arg(errorLine)
//                                 .arg(errorColumn)
//                                 .arg(errorStr));
//        return;
//    }

//    QDomElement plotRoot = domDocument.documentElement();                                       //File has wrong root tag
//    if (plotRoot.tagName() != "hopsanplot")
//    {
//        QMessageBox::information(gpMainWindow->window(), gpMainWindow->tr("Hopsan"),
//                                 "The file is not a plot window description file. Incorrect hmf root tag name: "
//                                 + plotRoot.tagName() + " != hopsanplot");
//        return;
//    }

//        //Create new plot window
//    PlotWindow *pPlotWindow = new PlotWindow(mpPlotVariableTree, gpMainWindow);
//    pPlotWindow->show();

//        //Add plot tabs
//    QDomElement tabElement = plotRoot.firstChildElement("plottab");
//    bool first = true;
//    while(!tabElement.isNull())
//    {
//        if(first)      //First tab is created automatically, so don't make one extra
//        {
//            first=false;
//        }
//        else
//        {
//            pPlotWindow->addPlotTab();
//        }

//        double red, green, blue;
//        parseRgbString(tabElement.attribute("color"), red, green, blue);
//        pPlotWindow->getCurrentPlotTab()->getPlot()->setCanvasBackground(QColor(red, green, blue));
//        pPlotWindow->getCurrentPlotTab()->enableGrid(tabElement.attribute("grid") == "true");

//            //Add plot curve to tab
//        QDomElement curveElement = tabElement.firstChildElement("curve");
//        while(!curveElement.isNull())
//        {
//            QString modelName = curveElement.attribute("model");        //Find project tab with model file. Do nothing if not found.
//            bool foundModel = false;
//            int i;
//            for(i=0; i<gpModelHandler->count(); ++i)
//            {
//                if(gpCentralTabWidget->getSystem(i)->getModelFileInfo().filePath() == modelName)
//                {
//                    foundModel = true;
//                    break;
//                }
//            }

//            int generation = curveElement.attribute("generation").toInt();
//            QString componentName = curveElement.attribute("component");
//            QString portName = curveElement.attribute("port");
//            QString dataName = curveElement.attribute("data");
//            QString dataUnit = curveElement.attribute("unit");
//            int axisY = curveElement.attribute("axis").toInt();
//            if(foundModel &&
//               gpCentralTabWidget->getContainer(i)->getPlotDataPtr()->size() >= generation &&
//               gpCentralTabWidget->getContainer(i)->hasModelObject(componentName) &&
//               gpCentralTabWidget->getContainer(i)->getModelObject(componentName)->getPort(portName) != 0)

//            {
//                pPlotWindow->addPlotCurve(generation, componentName, portName, dataName, dataUnit, axisY, modelName);
//                double red, green, blue;
//                parseRgbString(curveElement.attribute("color"),red,green,blue);
//                pPlotWindow->getCurrentPlotTab()->getCurves().last()->setLineColor(QColor(red, green, blue));
//                pPlotWindow->getCurrentPlotTab()->getCurves().last()->setLineWidth(curveElement.attribute("width").toInt());
//            }
//            curveElement = curveElement.nextSiblingElement("curve");
//        }
//        tabElement = tabElement.nextSiblingElement("plottab");
//    }
//    file.close();
}


void PlotTreeWidget::clearHoverEffects()
{
    mpPlotVariableTree->setFrameShape(QFrame::StyledPanel);
}


void PlotTreeWidget::mouseMoveEvent(QMouseEvent *event)
{
    clearHoverEffects();

    QWidget::mouseMoveEvent(event);
}
