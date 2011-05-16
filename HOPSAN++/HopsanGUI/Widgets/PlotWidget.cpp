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

#include <QDebug>
#include <QSpinBox>
#include <QColorDialog>
#include <QLabel>
#include <QCursor>
#include <QAction>
#include <QTextStream>

#include "../PlotWindow.h"
#include "PlotWidget.h"
#include "../MainWindow.h"
#include "ProjectTabWidget.h"
#include "../GUIObjects/GUIModelObject.h"
#include "../GUIPort.h"
#include "../GraphicsView.h"
#include "../GUIObjects/GUISystem.h"
#include "../Utilities/GUIUtilities.h"
#include "../loadObjects.h"
#include "MessageWidget.h"
#include "../Configuration.h"


//! @brief Constructor for the parameter items in the parameter tree
//! @param componentName Name of the component where the parameter is located
//! @param portName Name of the port where the parameter is located
//! @param dataName Name of the parameter
//! @param dataUnit Name of the unit of the parameter
//! @param parent Pointer to a tree widget item, not used
PlotParameterItem::PlotParameterItem(QString componentName, QString portName, QString dataName, QString dataUnit, QTreeWidgetItem *parent)
        : QTreeWidgetItem(parent)
{
    mComponentName = componentName;
    mPortName = portName;
    mDataName = dataName;
    mDataUnit = dataUnit;
    QString aliasPrepend = gpMainWindow->mpProjectTabs->getCurrentContainer()->getPlotAlias(componentName, portName, dataName);
    if(!aliasPrepend.isEmpty())
    {
        aliasPrepend.prepend("<");
        aliasPrepend.append("> ");
    }
    this->setText(0, aliasPrepend + mPortName + ", " + mDataName + ", [" + mDataUnit + "]");
}


//! @brief Returns the name of the component where the parameter is located
QString PlotParameterItem::getComponentName()
{
    return mComponentName;
}


//! @brief Returns the name of the port where the parameter is located
QString PlotParameterItem::getPortName()
{
    return mPortName;
}


//! @brief Returns the name of the parameter
QString PlotParameterItem::getDataName()
{
    return mDataName;
}


//! @brief Returns the name of the unit of the parameter
QString PlotParameterItem::getDataUnit()
{
    return mDataUnit;
}


//! @brief Constructor for the parameter tree widget
//! @param parent Pointer to the main window
PlotParameterTree::PlotParameterTree(MainWindow *parent)
        : QTreeWidget(parent)
{
    qDebug() << "Creating PlotParameterTree!";

    if(gpMainWindow->mpProjectTabs->count() > 0)
    {
        mpCurrentContainer = gpMainWindow->mpProjectTabs->getCurrentContainer();
        gpMainWindow->mpProjectTabs->getCurrentContainer()->mFavoriteVariables.clear();
        connect(gpMainWindow->mpProjectTabs->getCurrentContainer(), SIGNAL(componentChanged()), this, SLOT(updateList()));
        connect(gpMainWindow->mpProjectTabs->getCurrentTab(), SIGNAL(simulationFinished()), this, SLOT(updateList()));
    }

    this->setDragEnabled(true);
    this->setAcceptDrops(false);
    this->updateList();
    this->setHeaderHidden(true);
    this->setColumnCount(1);

    connect(gpMainWindow->mpProjectTabs, SIGNAL(currentChanged(int)), this, SLOT(updateList()));
    connect(gpMainWindow->mpProjectTabs, SIGNAL(tabCloseRequested(int)), this, SLOT(updateList()));
    connect(gpMainWindow->mpProjectTabs, SIGNAL(newTabAdded()), this, SLOT(updateList()));
    connect(this,SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)),this,SLOT(createPlotWindow(QTreeWidgetItem*)));
}


//! @brief Updates the parameter tree to the available components and parameters in the current tab.
void PlotParameterTree::updateList()
{
    mAvailableParameters.clear();
    this->clear();

    if(gpMainWindow->mpProjectTabs->count() == 0)     //Check so that at least one project tab exists
    {
        return;
    }

    mpCurrentContainer = gpMainWindow->mpProjectTabs->getCurrentContainer();
    QTreeWidgetItem *tempComponentItem;     //Tree item for components
    PlotParameterItem *tempPlotParameterItem;       //Tree item for parameters - reimplemented so they can store information about the parameter

    QVector<double> time;
    bool timeVectorRetained = false;
    GUIContainerObject::GUIModelObjectMapT::iterator it;
    for(it = mpCurrentContainer->mGUIModelObjectMap.begin(); it!=mpCurrentContainer->mGUIModelObjectMap.end(); ++it)
    {
        tempComponentItem = new QTreeWidgetItem();
        tempComponentItem->setText(0, it.value()->getName());
        QFont tempFont;
        tempFont = tempComponentItem->font(0);
        tempFont.setBold(true);
        tempComponentItem->setFont(0, tempFont);
        this->addTopLevelItem(tempComponentItem);

        QList<GUIPort*> portListPtrs = it.value()->getPortListPtrs();
        QList<GUIPort*>::iterator itp;
        for(itp = portListPtrs.begin(); itp !=portListPtrs.end(); ++itp)
        {
            //If the port is not connected it has nothing to plot
            if((*itp)->isConnected())
            {
                QVector<QString> parameterNames;
                QVector<QString> parameterUnits;
                gpMainWindow->mpProjectTabs->getCurrentContainer()->getCoreSystemAccessPtr()->getPlotDataNamesAndUnits((*itp)->getGuiModelObjectName(), (*itp)->getName(), parameterNames, parameterUnits);
                if(!timeVectorRetained)
                {
                    time = QVector<double>::fromStdVector(gpMainWindow->mpProjectTabs->getCurrentContainer()->getCoreSystemAccessPtr()->getTimeVector((*itp)->getGuiModelObjectName(), (*itp)->getName()));
                    timeVectorRetained = true;
                }
                if(time.size() > 0)     //If time vector is greater than zero we have something to plot!
                {
                    for(int i = 0; i!=parameterNames.size(); ++i)
                    {
                        parameterUnits[i] = gConfig.getDefaultUnit(parameterNames[i]);
                        tempPlotParameterItem = new PlotParameterItem(it.value()->getName(), (*itp)->getName(), parameterNames[i], parameterUnits[i], tempComponentItem);
                        tempComponentItem->addChild(tempPlotParameterItem);
                        QStringList parameterDescription;
                        parameterDescription << (*itp)->getGuiModelObjectName() << (*itp)->getName() << parameterNames[i] << parameterUnits[i];
                        mAvailableParameters.append(parameterDescription);
                        if(gpMainWindow->mpProjectTabs->getCurrentContainer()->mFavoriteVariables.contains(parameterDescription))
                        {
                            tempPlotParameterItem->setIcon(0, QIcon(QString(ICONPATH) + "Hopsan-Favorite.png"));
                        }
                    }
                }
            }
        }
    }

        //Append favorite plot variables to tree if they still exist
    for(int i=0; i<gpMainWindow->mpProjectTabs->getCurrentContainer()->mFavoriteVariables.size(); ++i)
    {
        QString componentName = gpMainWindow->mpProjectTabs->getCurrentContainer()->mFavoriteVariables.at(i).at(0);
        QString portName = gpMainWindow->mpProjectTabs->getCurrentContainer()->mFavoriteVariables.at(i).at(1);
        QString dataName = gpMainWindow->mpProjectTabs->getCurrentContainer()->mFavoriteVariables.at(i).at(2);
        QString dataUnit = gpMainWindow->mpProjectTabs->getCurrentContainer()->mFavoriteVariables.at(i).at(3);

        if(!componentName.isEmpty())
        {
            tempPlotParameterItem = new PlotParameterItem(componentName, portName, dataName, dataUnit);
            tempPlotParameterItem->setText(0, tempPlotParameterItem->text(0).prepend(" " + componentName + ", "));
            tempPlotParameterItem->setIcon(0, QIcon(QString(ICONPATH) + "Hopsan-Favorite.png"));
            this->addTopLevelItem(tempPlotParameterItem);
            tempPlotParameterItem->setDisabled(!mAvailableParameters.contains(gpMainWindow->mpProjectTabs->getCurrentContainer()->mFavoriteVariables.at(i)));
        }
    }

        //Remove no longer existing favorite variables
    for(int i=0; i<gpMainWindow->mpProjectTabs->getCurrentContainer()->mFavoriteVariables.size(); ++i)
    {
        if(!mAvailableParameters.contains(gpMainWindow->mpProjectTabs->getCurrentContainer()->mFavoriteVariables.at(i)))
        {
           // gpMainWindow->mpProjectTabs->getCurrentContainer()->mFavoriteVariables.removeAll(gpMainWindow->mpProjectTabs->getCurrentTopLevelSystem()->mFavoriteVariables.at(i));
        }
    }

        //Sort the tree widget
    this->sortItems(0, Qt::AscendingOrder);

        // This connection makes sure that the plot list is connected to the new tab, so that it will update if the new tab is simulated.
        // It must first be disconnected in case it was already connected, to avoid duplication of connection.
    disconnect(gpMainWindow->mpProjectTabs->getCurrentTab(), SIGNAL(simulationFinished()), this, SLOT(updateList()));
    connect(gpMainWindow->mpProjectTabs->getCurrentTab(), SIGNAL(simulationFinished()), this, SLOT(updateList()));
}


//! @brief Helper function that creates a new plot window by using a QTreeWidgetItem in the plot variable tree.
//! @param *item Pointer to the tree widget item whos arrays will be looked up from the map and plotted
PlotWindow *PlotParameterTree::createPlotWindow(QTreeWidgetItem *item)
{
    //! @todo This is a kind of dumb check; it assumes that component items have bold font and variables not.
    if(!item->font(0).bold() && !item->isDisabled())     //Top level items cannot be plotted (they represent the components)
    {
        //QTreeWidgetItem must be casted to a PlotParameterItem. This is a necessary because double click event can not know which kind of tree item is clicked.
        PlotParameterItem *tempItem = dynamic_cast<PlotParameterItem *>(item);
        return createPlotWindow(tempItem->getComponentName(), tempItem->getPortName(), tempItem->getDataName(), ""/*tempItem->getDataUnit()*/);
    }
    return 0; //! @todo Should this return 0?
}


PlotWindow *PlotParameterTree::getPlotWindow(int number)
{
    return mOpenPlotWindows.at(number);
}


void PlotParameterTree::reportClosedPlotWindow(PlotWindow *window)
{
    mOpenPlotWindows.removeAll(window);
}


//! @brief Creates a new plot window from specified component and parameter.
//! @param componentName Name of the component where the port with the parameter is located
//! @param portName Name of the port where the parameter is located
//! @param dataName Name of the parameter
//! @param dataUnit Name of the unit of the parameter
PlotWindow *PlotParameterTree::createPlotWindow(QString componentName, QString portName, QString dataName, QString dataUnit)
{
    QVector<double> xVector = QVector<double>::fromStdVector(gpMainWindow->mpProjectTabs->getCurrentContainer()->getCoreSystemAccessPtr()->getTimeVector(componentName, portName));
    QVector<double> yVector;
    gpMainWindow->mpProjectTabs->getCurrentContainer()->getCoreSystemAccessPtr()->getPlotData(componentName, portName, dataName, yVector);

    qDebug() << yVector;

    if((xVector.isEmpty()) || (yVector.isEmpty()))
        return 0;

    PlotWindow *plotWindow = new PlotWindow(this, gpMainWindow);
    plotWindow->show();
    plotWindow->addPlotCurve(mpCurrentContainer->getNumberOfPlotGenerations()-1, componentName, portName, dataName, dataUnit, QwtPlot::yLeft);

    mOpenPlotWindows.append(plotWindow);

    return plotWindow;
}


PlotWindow *PlotParameterTree::createPlotWindow(QVector<double> xVector, QVector<double> yVector, int axis, QString componentName, QString portName, QString dataName, QString dataUnit)
{
    PlotWindow *plotWindow = new PlotWindow(this, gpMainWindow);
    plotWindow->show();
    plotWindow->addPlotCurve(0, componentName, portName, dataName, dataUnit, axis);

    return plotWindow;
}


//! @brief Defines what happens when clicking in the variable list. Used to initiate drag operations.
void PlotParameterTree::mousePressEvent(QMouseEvent *event)
{
    QTreeWidget::mousePressEvent(event);

    if (event->button() == Qt::LeftButton)
        dragStartPosition = event->pos();
}


//! @brief Defines what happens when mouse is moving in variable list. Used to handle drag operations.
void PlotParameterTree::mouseMoveEvent(QMouseEvent *event)
{

    if (!(event->buttons() & Qt::LeftButton))
    {
        return;
    }
    if ((event->pos() - dragStartPosition).manhattanLength() < QApplication::startDragDistance())
    {
        return;
    }

    PlotParameterItem *item;
    item = dynamic_cast<PlotParameterItem *>(currentItem());

    if(item != 0)
    {
        QString mimeText;
        mimeText = QString("HOPSANPLOTDATA " + addQuotes(item->getComponentName()) + " " + addQuotes(item->getPortName()) + " " + addQuotes(item->getDataName()) + " " + addQuotes(item->getDataUnit()));
        QDrag *drag = new QDrag(this);
        QMimeData *mimeData = new QMimeData;
        mimeData->setText(mimeText);
        drag->setMimeData(mimeData);
        drag->exec();
    }
}


//! @brief Defines the right-click menu in the parameter tree
void PlotParameterTree::contextMenuEvent(QContextMenuEvent *event)
{
    qDebug() << "contextMenuEvent()";

    PlotParameterItem *item;

    //! @todo Dumb check that assumes component tree items to be bold and parameter items to be not bold
    if(currentItem() != 0 && !currentItem()->font(0).bold())
    {
        qDebug() << "currentItem() is ok!";

        item = dynamic_cast<PlotParameterItem *>(currentItem());
        QStringList parameterDescription;
        parameterDescription << item->getComponentName() << item->getPortName() << item->getDataName() << item->getDataUnit();
        QMenu menu;

        QAction *defineAliasAction = 0;
        QAction *removeAliasAction = 0;
        QAction *addToFavoritesAction = 0;
        QAction *removeFromFavoritesAction = 0;

        if(gpMainWindow->mpProjectTabs->getCurrentContainer()->getPlotAlias(item->getComponentName(), item->getPortName(), item->getDataName()).isEmpty())
        {
            defineAliasAction = menu.addAction(QString("Define Variable Alias"));
        }
        else
        {
            removeAliasAction = menu.addAction(QString("Remove Variable Alias"));
        }


        if(!gpMainWindow->mpProjectTabs->getCurrentContainer()->mFavoriteVariables.contains(parameterDescription))
        {
            addToFavoritesAction = menu.addAction(QString("Add Favorite Variable"));
        }
        else
        {
            removeFromFavoritesAction = menu.addAction(QString("Remove Favorite Variable"));
        }

        //-- Action --//
        QCursor *cursor;
        QAction *selectedAction = menu.exec(cursor->pos());
        //------------//

        if(selectedAction == removeAliasAction)
        {
           gpMainWindow->mpProjectTabs->getCurrentContainer()->undefinePlotAlias(gpMainWindow->mpProjectTabs->getCurrentContainer()->getPlotAlias(item->getComponentName(), item->getPortName(), item->getDataName()));
           this->updateList();
        }

        if(selectedAction == defineAliasAction)
        {
            gpMainWindow->mpProjectTabs->getCurrentContainer()->definePlotAlias(item->getComponentName(), item->getPortName(), item->getDataName());
            this->updateList();
        }

        if(selectedAction == addToFavoritesAction)
        {
            gpMainWindow->mpProjectTabs->getCurrentContainer()->setFavoriteVariable(parameterDescription.at(0), parameterDescription.at(1), parameterDescription.at(2), parameterDescription.at(3));
        }

        if(selectedAction == removeFromFavoritesAction)
        {
           gpMainWindow->mpProjectTabs->getCurrentContainer()->mFavoriteVariables.removeAll(parameterDescription);
           this->updateList();
        }
    }
}


//! @brief Constructor the main plot widget, which contains the tree with variables
//! @param parent Pointer to the main window
PlotWidget::PlotWidget(MainWindow *parent)
        : QWidget(parent)
{
    qDebug() << "Creating PlotWidget!";

    //mpParentMainWindow = parent;

    mpPlotParameterTree = new PlotParameterTree(gpMainWindow);

    mpLoadButton = new QPushButton(tr("&Load Plot Window from XML"), this);
    mpLoadButton->setAutoDefault(false);
    mpLoadButton->setFixedHeight(30);
    QFont tempFont = mpLoadButton->font();
    tempFont.setBold(true);
    mpLoadButton->setFont(tempFont);

    mpLayout = new QGridLayout(this);
    mpLayout->addWidget(mpPlotParameterTree,0,0,1,1);
    mpLayout->addWidget(mpLoadButton, 1, 0, 1, 1);
    mpLayout->setContentsMargins(4,4,4,4);

    connect(mpLoadButton, SIGNAL(clicked()),this,SLOT(loadFromXml()));
}


//! Loads a plot window from a specified .hpw file. Loads the actual plot data from a .xml file.
void PlotWidget::loadFromXml()
{
    QDir fileDialogOpenDir;
    QString fileName = QFileDialog::getOpenFileName(this, tr("Load Plot Window Description From XML"),
                                                         fileDialogOpenDir.currentPath(),
                                                         tr("Plot Window Description File (*.xml)"));
    if(fileName.isEmpty())                                                                      //User did not select a file
    {
        return;
    }
    QFile file(fileName);                                                                       //File is not readable
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::information(gpMainWindow->window(), gpMainWindow->tr("Hopsan GUI"),
                                 "Unable to read XML file.\n");
        return;
    }

    QDomDocument domDocument;
    QString errorStr;
    int errorLine, errorColumn;
    if (!domDocument.setContent(&file, false, &errorStr, &errorLine, &errorColumn))             //Parse error in file
    {
        QMessageBox::information(gpMainWindow->window(), gpMainWindow->tr("Hopsan GUI"),
                                 gpMainWindow->tr("Parse error at line %1, column %2:\n%3")
                                 .arg(errorLine)
                                 .arg(errorColumn)
                                 .arg(errorStr));
        return;
    }

    QDomElement plotRoot = domDocument.documentElement();                                       //File has wrong root tag
    if (plotRoot.tagName() != "hopsanplot")
    {
        QMessageBox::information(gpMainWindow->window(), gpMainWindow->tr("Hopsan GUI"),
                                 "The file is not a plot window description file. Incorrect hmf root tag name: "
                                 + plotRoot.tagName() + " != hopsanplot");
        return;
    }

        //Create new plot window
    PlotWindow *pPlotWindow = new PlotWindow(mpPlotParameterTree, gpMainWindow);
    pPlotWindow->show();

        //Add plot tabs
    QDomElement tabElement = plotRoot.firstChildElement("plottab");
    bool first = true;
    while(!tabElement.isNull())
    {
        if(first)      //First tab is created automatically, so don't make one extra
        {
            first=false;
        }
        else
        {
            pPlotWindow->addPlotTab();
        }

        double red, green, blue;
        parseRgbString(tabElement.attribute("color"), red, green, blue);
        pPlotWindow->getCurrentPlotTab()->getPlot()->setCanvasBackground(QColor(red, green, blue));
        pPlotWindow->getCurrentPlotTab()->enableGrid(tabElement.attribute("grid") == "true");

            //Add plot curve to tab
        QDomElement curveElement = tabElement.firstChildElement("curve");
        while(!curveElement.isNull())
        {
            QString modelName = curveElement.attribute("model");        //Find project tab with model file. Do nothing if not found.
            bool foundModel = false;
            int i;
            for(i=0; i<gpMainWindow->mpProjectTabs->count(); ++i)
            {
                if(gpMainWindow->mpProjectTabs->getTab(i)->mpSystem->mModelFileInfo.filePath() == modelName)
                {
                    foundModel = true;
                    break;
                }
            }

            int generation = curveElement.attribute("generation").toInt();
            QString componentName = curveElement.attribute("component");
            QString portName = curveElement.attribute("port");
            QString dataName = curveElement.attribute("data");
            QString dataUnit = curveElement.attribute("unit");
            int axisY = curveElement.attribute("axis").toInt();
            if(foundModel &&
               gpMainWindow->mpProjectTabs->getContainer(i)->getNumberOfPlotGenerations() >= generation &&
               gpMainWindow->mpProjectTabs->getContainer(i)->hasGUIModelObject(componentName) &&
               gpMainWindow->mpProjectTabs->getContainer(i)->getGUIModelObject(componentName)->getPort(portName) != 0)

            {
                pPlotWindow->addPlotCurve(generation, componentName, portName, dataName, dataUnit, axisY, modelName);
                double red, green, blue;
                parseRgbString(curveElement.attribute("color"),red,green,blue);
                pPlotWindow->getCurrentPlotTab()->getCurves().last()->setLineColor(QColor(red, green, blue));
                pPlotWindow->getCurrentPlotTab()->getCurves().last()->setLineWidth(curveElement.attribute("width").toInt());
            }
            curveElement = curveElement.nextSiblingElement("curve");
        }
        tabElement = tabElement.nextSiblingElement("plottab");
    }
    file.close();
}



