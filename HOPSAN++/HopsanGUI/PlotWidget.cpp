//$Id$

#include <QDebug>
#include <QSpinBox>
#include <QColorDialog>
#include <QLabel>
#include <QCursor>
#include <QAction>

#include "PlotWindow.h"
#include "PlotWidget.h"
#include "MainWindow.h"
#include "ProjectTabWidget.h"
#include "GUIObject.h"
#include "GUIPort.h"
#include "GraphicsView.h"
#include "GUISystem.h"
#include "GUIUtilities.h"
#include "GUISystem.h"


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
    this->setText(0, mPortName + ", " + mDataName + ", [" + mDataUnit + "]");
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
    mpParentMainWindow = parent;
    mpCurrentSystem = mpParentMainWindow->mpProjectTabs->getCurrentSystem();
    mFavoriteParameters.clear();

    this->setDragEnabled(true);
    this->setAcceptDrops(false);
    this->updateList();
    this->setHeaderHidden(true);
    this->setColumnCount(1);

    connect(mpParentMainWindow->mpProjectTabs, SIGNAL(currentChanged(int)), this, SLOT(updateList()));
    connect(mpParentMainWindow->mpProjectTabs, SIGNAL(tabCloseRequested(int)), this, SLOT(updateList()));
    connect(mpParentMainWindow->mpProjectTabs, SIGNAL(newTabAdded()), this, SLOT(updateList()));
    connect(mpParentMainWindow->mpProjectTabs->getCurrentSystem(), SIGNAL(componentChanged()), this, SLOT(updateList()));
    connect(mpParentMainWindow->mpProjectTabs->getCurrentTab(), SIGNAL(simulationFinished()), this, SLOT(updateList()));
    connect(this,SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)),this,SLOT(createPlotWindow(QTreeWidgetItem*)));
}


//! @brief Updates the parameter tree to the available components and parameters in the current tab.
void PlotParameterTree::updateList()
{
    mAvailableParameters.clear();
    this->clear();

    if(mpParentMainWindow->mpProjectTabs->count() == 0)     //Check so that at least one project tab exists
    {
        return;
    }

    mpCurrentSystem = mpParentMainWindow->mpProjectTabs->getCurrentTab()->mpSystem;
    QTreeWidgetItem *tempComponentItem;     //Tree item for components
    PlotParameterItem *tempPlotParameterItem;       //Tree item for parameters - reimplemented so they can store information about the parameter

    QHash<QString, GUIObject *>::iterator it;
    for(it = mpCurrentSystem->mGUIObjectMap.begin(); it!=mpCurrentSystem->mGUIObjectMap.end(); ++it)
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
            QVector<QString> parameterNames;
            QVector<QString> parameterUnits;
            mpParentMainWindow->mpProjectTabs->getCurrentTab()->mpSystem->mpCoreSystemAccess->getPlotDataNamesAndUnits((*itp)->getGUIComponentName(), (*itp)->getName(), parameterNames, parameterUnits);

            QVector<double> time = QVector<double>::fromStdVector(mpParentMainWindow->mpProjectTabs->getCurrentTab()->mpSystem->mpCoreSystemAccess->getTimeVector((*itp)->getGUIComponentName(), (*itp)->getName()));

            if(time.size() > 0)     //If time vector is greater than zero we have something to plot!
            {
                for(int i = 0; i!=parameterNames.size(); ++i)
                {
                    tempPlotParameterItem = new PlotParameterItem(it.value()->getName(), (*itp)->getName(), parameterNames[i], parameterUnits[i], tempComponentItem);
                    tempComponentItem->addChild(tempPlotParameterItem);
                    QStringList parameterDescription;
                    parameterDescription << (*itp)->getGUIComponentName() << (*itp)->getName() << parameterNames[i] << parameterUnits[i];
                    mAvailableParameters.append(parameterDescription);
                    if(mFavoriteParameters.contains(parameterDescription))
                    {
                        tempPlotParameterItem->setIcon(0, QIcon(QString(ICONPATH) + "Hopsan-Favorite.png"));
                    }
                }
            }
        }
    }

        //Append favorite plot variables to tree if they still exist
    for(size_t i=0; i<mFavoriteParameters.size(); ++i)
    {
        if(mAvailableParameters.contains(mFavoriteParameters.at(i)))
        {
            QString componentName = mFavoriteParameters.at(i).at(0);
            QString portName = mFavoriteParameters.at(i).at(1);
            QString dataName = mFavoriteParameters.at(i).at(2);
            QString dataUnit = mFavoriteParameters.at(i).at(3);

            tempPlotParameterItem = new PlotParameterItem(componentName, portName, dataName, dataUnit);
            tempPlotParameterItem->setText(0, tempPlotParameterItem->text(0).prepend(" " + componentName + ", "));
            tempPlotParameterItem->setIcon(0, QIcon(QString(ICONPATH) + "Hopsan-Favorite.png"));
            this->addTopLevelItem(tempPlotParameterItem);
        }
    }

        //Remove no longer existing favorite parameters
    for(size_t i=0; i<mFavoriteParameters.size(); ++i)
    {
        if(!mAvailableParameters.contains(mFavoriteParameters.at(i)))
        {
            mFavoriteParameters.removeAll(mFavoriteParameters.at(i));
        }
    }

        //Sort the tree widget
    this->sortItems(0, Qt::AscendingOrder);

        // This connection makes sure that the plot list is connected to the new tab, so that it will update if the new tab is simulated.
        // It must first be disconnected in case it was already connected, to avoid duplication of connection.
    disconnect(mpParentMainWindow->mpProjectTabs->getCurrentTab(), SIGNAL(simulationFinished()), this, SLOT(updateList()));
    connect(mpParentMainWindow->mpProjectTabs->getCurrentTab(), SIGNAL(simulationFinished()), this, SLOT(updateList()));
}


//! @brief Helper function that creates a new plot window by using a QTreeWidgetItem in the plot variable tree.
//! @param *item Pointer to the tree widget item whos arrays will be looked up from the map and plotted
void PlotParameterTree::createPlotWindow(QTreeWidgetItem *item)
{
    //! @todo This is a kind of dumb check; it assumes that component items have bold font and variables not.
    if(!item->font(0).bold())     //Top level items cannot be plotted (they represent the components)
    {
        //QTreeWidgetItem must be casted to a PlotParameterItem. This is a necessary because double click event can not know which kind of tree item is clicked.
        PlotParameterItem *tempItem = dynamic_cast<PlotParameterItem *>(item);
        createPlotWindow(tempItem->getComponentName(), tempItem->getPortName(), tempItem->getDataName(), tempItem->getDataUnit());
    }
}


//! @brief Creates a new plot window from specified component and parameter.
//! @param componentName Name of the component where the port with the parameter is located
//! @param portName Name of the port where the parameter is located
//! @param dataName Name of the parameter
//! @param dataUnit Name of the unit of the parameter
void PlotParameterTree::createPlotWindow(QString componentName, QString portName, QString dataName, QString dataUnit)
{
    QVector<double> xVector = QVector<double>::fromStdVector(mpParentMainWindow->mpProjectTabs->getCurrentTab()->mpSystem->mpCoreSystemAccess->getTimeVector(componentName, portName));
    QVector<double> yVector;
    mpParentMainWindow->mpProjectTabs->getCurrentTab()->mpSystem->mpCoreSystemAccess->getPlotData(componentName, portName, dataName, yVector);

    PlotWindow *plotWindow = new PlotWindow(this, mpParentMainWindow);
    plotWindow->show();
    plotWindow->addPlotCurve(xVector, yVector, componentName, portName, dataName, dataUnit, QwtPlot::yLeft);
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
        if(!mFavoriteParameters.contains(parameterDescription))
        {
            QAction *addToFavoritesAction;
            addToFavoritesAction = menu.addAction(QString("Add Favorite Parameter"));

            QCursor *cursor;
            QAction *selectedAction = menu.exec(cursor->pos());

            if(selectedAction == addToFavoritesAction)
            {
               mFavoriteParameters.append(parameterDescription);
               this->updateList();
            }
        }
        else
        {
            QAction *removeFromFavoritesAction;
            removeFromFavoritesAction = menu.addAction(QString("Remove Favorite Parameter"));

            QCursor *cursor;
            QAction *selectedAction = menu.exec(cursor->pos());

            if(selectedAction == removeFromFavoritesAction)
            {
               mFavoriteParameters.removeAll(parameterDescription);
               this->updateList();
            }
        }
    }
}


//! @brief Constructor the main plot widget, which contains the tree with variables
//! @param parent Pointer to the main window
PlotWidget::PlotWidget(MainWindow *parent)
        : QWidget(parent)
{
    mpParentMainWindow = parent;

    mpPlotParameterTree = new PlotParameterTree(mpParentMainWindow);
    mpLayout = new QGridLayout(this);
    mpLayout->addWidget(mpPlotParameterTree,0,0,3,1);
}
