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
//! @brief Contains the PlotWidget and related classes
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
#include "global.h"
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

// Plot Widget help classes declarations
// ----------------------------------------------------------------------------

class BaseVariableTreeItem : public QTreeWidgetItem
{
public:
    BaseVariableTreeItem(SharedLogVariableDataPtrT pData, QTreeWidgetItem *pParent);
    SharedLogVariableDataPtrT getDataPtr();
    QString getFullName() const;
    QString getComponentName();
    QString getPortName();
    QString getDataName();
    QString getDataUnit();
    QString getAliasName();
    int getGeneration() const;

protected:
    SharedLogVariableDataPtrT mpData;
};

class FullVariableTreeItem : public BaseVariableTreeItem
{
public:
    FullVariableTreeItem(SharedLogVariableDataPtrT pData, QTreeWidgetItem *pParent);
};

class ImportedVariableTreeItem : public BaseVariableTreeItem
{
public:
    ImportedVariableTreeItem(SharedLogVariableDataPtrT pData, QTreeWidgetItem *pParent);
};

class AliasVariableTreeItem : public BaseVariableTreeItem
{
public:
    AliasVariableTreeItem(SharedLogVariableDataPtrT pData, QTreeWidgetItem *pParent);
};

class ImportedFileTreeItem : public QTreeWidgetItem
{
public:
    ImportedFileTreeItem(const QString &rFileName, QTreeWidgetItem *pParent);
};

// ----------------------------------------------------------------------------



//! @brief Constructor for the variable items in the variable tree
//! @param pData Shared pointer to variable to represent
//! @param pParent Pointer to the parent tree widget item
BaseVariableTreeItem::BaseVariableTreeItem(SharedLogVariableDataPtrT pData, QTreeWidgetItem *pParent)
        : QTreeWidgetItem(pParent)
{
    mpData = pData;
}

SharedLogVariableDataPtrT BaseVariableTreeItem::getDataPtr()
{
    return mpData;
}

QString BaseVariableTreeItem::getFullName() const
{
    return mpData->getFullVariableName();
}


//! @brief Returns the name of the component where the variable is located
QString BaseVariableTreeItem::getComponentName()
{
    return mpData->getComponentName();
}


//! @brief Returns the name of the port where the variable is located
QString BaseVariableTreeItem::getPortName()
{
    return mpData->getPortName();
}


//! @brief Returns the name of the variable
QString BaseVariableTreeItem::getDataName()
{
    return mpData->getDataName();
}


//! @brief Returns the name of the unit of the variable
QString BaseVariableTreeItem::getDataUnit()
{
    return mpData->getDataUnit();
}

QString BaseVariableTreeItem::getAliasName()
{
    return mpData->getAliasName();
}

int BaseVariableTreeItem::getGeneration() const
{
    return mpData->getGeneration();
}

//! @brief Constructor for the variable tree widget
//! @param parent Pointer to the main window
VariableTree::VariableTree(QWidget *pParent)
        : QTreeWidget(pParent)
{
    mpLogDataHandler = 0;
    mpImportedItemParent=0;
    mpAliasItemParent=0;

    this->setDragEnabled(true);
    this->setAcceptDrops(false);
    this->setHeaderHidden(true);
    this->setColumnCount(1);
    this->setMouseTracking(true);

    resetAliasItemParent();
    resetImportedItemParent();

    connect(this, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(plotToPreferedPlotWindow(QTreeWidgetItem*)));
}

void VariableTree::setLogDataHandler(QPointer<LogDataHandler> pLogDataHandler)
{
    mpLogDataHandler = pLogDataHandler;
}

LogDataHandler *VariableTree::getLogDataHandler()
{
    return mpLogDataHandler;
}

void VariableTree::setPreferedPlotWindow(QPointer<PlotWindow> pPreferedPlotWindow)
{
    mpPreferedPlotWindow = pPreferedPlotWindow;
}

void VariableTree::addFullVariable(SharedLogVariableDataPtrT pData)
{
    QTreeWidgetItem *pComponentItem=0;
    // Did we have the top-level component item (in that case use it)
    QString cname = pData->getComponentName();
    if (cname.isEmpty())
    {
        // Mostly for the timevariable
        cname = pData->getFullVariableName();
    }
    // Try to find the component if it has already been added
    QMap<QString, QTreeWidgetItem*>::iterator cit = mFullVariableItemMap.find(cname);
    if (cit != mFullVariableItemMap.end())
    {
        pComponentItem = cit.value();
    }
    else
    // Ok, we did not find it, then this is the first time the component is added, lets create and add that top-lvel item
    {
        pComponentItem = new QTreeWidgetItem();
        pComponentItem->setText(0, cname);
        QFont tempFont = pComponentItem->font(0);
        tempFont.setBold(true);
        pComponentItem->setFont(0, tempFont);
        this->addTopLevelItem(pComponentItem);

        //Also remember that we created it
        mFullVariableItemMap.insert(pData->getComponentName(), pComponentItem);
    }

    // Add the actual variable item
    pComponentItem->addChild(new FullVariableTreeItem(pData, pComponentItem));
    this->show();
}

void VariableTree::addAliasVariable(SharedLogVariableDataPtrT pData)
{
    // Check if this is an alias variable, if alias is set and not already in the aliasLevelItemMap map
    if ( !mAliasVariableItemMap.contains(pData->getAliasName()) )
    {
        // Add a sub item with alias name and remember it in the map
        AliasVariableTreeItem *pItem = new AliasVariableTreeItem(pData, 0);
        mpAliasItemParent->addChild(pItem);
        mAliasVariableItemMap.insert(pData->getAliasName(), pItem);
        mpAliasItemParent->setHidden(false);
        mpAliasItemParent->setExpanded(true);
    }
}

void VariableTree::addImportedVariable(SharedLogVariableDataPtrT pData)
{
    QString fName = pData->getImportedFromFileName();
    QTreeWidgetItem *pFileItem = mImportedFileItemMap.value(fName, 0);
    // If this file was not alrady added then create it
    if (!pFileItem)
    {
        pFileItem = new ImportedFileTreeItem(fName,0);

        // Also remember that we created it
        mImportedFileItemMap.insert(fName, pFileItem);
        mpImportedItemParent->addChild(pFileItem);
    }

    // Add a sub item with data name name
    new ImportedVariableTreeItem(pData, pFileItem);
    mpImportedItemParent->setHidden(false);
    mpImportedItemParent->setExpanded(true);
}

void VariableTree::refreshImportedVariables()
{
    resetImportedItemParent();
    QList<QString> importedFileNames = mpLogDataHandler->getImportedVariablesFileNames();
    for (int f=0; f<importedFileNames.size(); ++f)
    {
        QList<SharedLogVariableDataPtrT> vars = mpLogDataHandler->getImportedVariablesForFile(importedFileNames[f]);
        if (!vars.isEmpty())
        {
            for (int v=0; v<vars.size(); ++v)
            {
                addImportedVariable(vars[v]);
            }
        }
    }
}

void VariableTree::updateList()
{
    QStringList expandedImportFiles, expandedFullVariableComponents;
    getExpandedFullVariables(expandedFullVariableComponents);
    getExpandedImportFiles(expandedImportFiles);

    // Clear all contents
    clear();

    // Check so that we have something to represent
    if(mpLogDataHandler == 0)
    {
        return;
    }

    // First refresh the import variable tree
    refreshImportedVariables();

    // Now add variables to the Alis and Variable tree
    QVector<SharedLogVariableDataPtrT> variables = getLogDataHandler()->getAllVariablesAtNewestGeneration();
    for(int i=0; i<variables.size(); ++i)
    {
        if ( (variables[i]->getVariableSource() == TempVariableType) || (variables[i]->getVariableSource() == ImportedVariableType) )
        {
            continue;
        }

        // Handle alias variables
        if ( !variables[i]->getAliasName().isEmpty() )
        {
            addAliasVariable(variables[i]);
        }

        // Handle all non-temp non-imported variables
        addFullVariable(variables[i]);
    }

    // Sort the tree widget
    sortItems(0, Qt::AscendingOrder);

    // Now restore the expanded variables tree items
    expandFullVariableItems(expandedFullVariableComponents);
    expandImportFileItems(expandedImportFiles);
}

void VariableTree::clear()
{
    QTreeWidget::clear();
    mpImportedItemParent = 0;
    mpAliasItemParent = 0;

    mImportedFileItemMap.clear();
    mAliasVariableItemMap.clear();
    mFullVariableItemMap.clear();

    resetAliasItemParent();
    resetImportedItemParent();
}


//! @brief Updates the variable tree to the available components and variables in the current tab.
void PlotWidget::updateList()
{
    // There is not point in updating if we are not visible
    if (isVisible())
    {
        mpVariableTree->updateList();
        mHasPendingUpdate = false;
    }
    else
    {
        // Remember that we want to update, until the next time we show
        mHasPendingUpdate = true;
    }
}

void PlotWidget::clearList()
{
    mpVariableTree->clear();
}


//! @brief Helper function that plots item to prefered plotwindow or creates a new one
//! @param[in] *item Pointer to the tree widget item whos should be plotted
//! @returns A pointer to the plotwindow or 0 pointer if item could not be plotted
PlotWindow *VariableTree::plotToPreferedPlotWindow(QTreeWidgetItem *item)
{
    // QTreeWidgetItem must be casted to a BaseVariableTreeItem. This is a necessary because double click event can not know which kind of tree item is clicked.
    // Top level items cannot be plotted (they represent the components)
    BaseVariableTreeItem *pVariableItem = dynamic_cast<BaseVariableTreeItem *>(item);
    if(pVariableItem)
    {
        if (mpPreferedPlotWindow)
        {
            return gpPlotHandler->plotDataToWindow(mpPreferedPlotWindow, pVariableItem->getDataPtr(), QwtPlot::yLeft);
        }
        else
        {
            return gpPlotHandler->plotDataToWindow(0, pVariableItem->getDataPtr(), QwtPlot::yLeft);
        }
    }
    return 0;
}

//! @brief Defines what happens when clicking in the variable list. Used to initiate drag operations.
void VariableTree::mousePressEvent(QMouseEvent *event)
{
    gpMainWindow->showHelpPopupMessage("Double click on a variable to open a new plot window, or drag it to an existing one.");
    QTreeWidget::mousePressEvent(event);

    if (event->button() == Qt::LeftButton)
        mDragStartPosition = event->pos();
}


//! @brief Defines what happens when mouse is moving in variable list. Used to handle drag operations.
void VariableTree::mouseMoveEvent(QMouseEvent *event)
{
    gpMainWindow->showHelpPopupMessage("Double click on a variable to plot it, or drag it to an existing plot window.");
    if (!(event->buttons() & Qt::LeftButton))
    {
        return;
    }
    if ((event->pos() - mDragStartPosition).manhattanLength() < QApplication::startDragDistance())
    {
        return;
    }

    BaseVariableTreeItem *item;
    item = dynamic_cast<BaseVariableTreeItem *>(itemAt(mDragStartPosition.toPoint()));

    if(item != 0)
    {
        QString mimeText;
        mimeText = QString("HOPSANPLOTDATA:"+item->getFullName()+QString(":%1").arg(item->getGeneration()));
        QDrag *drag = new QDrag(this);
        QMimeData *mimeData = new QMimeData;
        mimeData->setText(mimeText);
        drag->setMimeData(mimeData);
        drag->exec();
    }

    QTreeWidget::mouseMoveEvent(event);
}


//! @brief Defines the right-click menu in the variable tree
void VariableTree::contextMenuEvent(QContextMenuEvent *event)
{
    Q_UNUSED(event);

    // Build context menue for variable items
    BaseVariableTreeItem *pItem = dynamic_cast<BaseVariableTreeItem *>(currentItem());
    if(pItem)
    {
        QMenu menu;
        QAction *pDefineAliasAction = 0;
        QAction *pRemoveAliasAction = 0;

        if(pItem->getAliasName().isEmpty())
        {
            pDefineAliasAction = menu.addAction(QString("Define Variable Alias"));
        }
        else
        {
            pDefineAliasAction = menu.addAction(QString("Change Variable Alias"));
            pRemoveAliasAction = menu.addAction(QString("Remove Variable Alias"));
        }

        //-- Action --//
        QCursor *cursor;
        QAction *selectedAction = menu.exec(cursor->pos());
        //------------//

        if(selectedAction == pRemoveAliasAction)
        {
           mpLogDataHandler->undefinePlotAlias(pItem->getAliasName());
        }

        if(selectedAction == pDefineAliasAction)
        {
            mpLogDataHandler->definePlotAlias(pItem->getFullName());
        }
    }

    // Build context menu for imported file items
    ImportedFileTreeItem *pFileItem = dynamic_cast<ImportedFileTreeItem *>(currentItem());
    if (pFileItem)
    {
        QMenu menu;
        QAction *pRemovefileAction = menu.addAction("Unload File");

        QCursor *cursor;
        QAction *selectedAction = menu.exec(cursor->pos());


        if(selectedAction == pRemovefileAction)
        {
           mpLogDataHandler->removeImportedFileGeneration(pFileItem->text(0));
        }
    }
}

void PlotWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    if (mHasPendingUpdate)
    {
        updateList();
        mHasPendingUpdate = false;
    }
}

void VariableTree::resetImportedItemParent()
{
    if (mpImportedItemParent)
    {
        removeItemWidget(mpImportedItemParent, 0);
    }
    mpImportedItemParent = new QTreeWidgetItem();
    mpImportedItemParent->setText(0,"__Imported__");
    QFont font = mpImportedItemParent->font(0);
    font.setBold(true);
    mpImportedItemParent->setFont(0,font);
    addTopLevelItem(mpImportedItemParent);
    mpImportedItemParent->setHidden(true);
}

void VariableTree::resetAliasItemParent()
{
    if (mpAliasItemParent)
    {
        removeItemWidget(mpAliasItemParent, 0);
    }
    mpAliasItemParent = new QTreeWidgetItem();
    mpAliasItemParent->setText(0,"__Alias__");
    QFont font = mpAliasItemParent->font(0);
    font.setBold(true);
    mpAliasItemParent->setFont(0,font);
    addTopLevelItem(mpAliasItemParent);
    mpAliasItemParent->setHidden(true);
}

void VariableTree::getExpandedFullVariables(QStringList &rList)
{
    rList.clear();
    QTreeWidgetItemIterator it(this);
    while(*it)
    {
        if ( (*it != mpAliasItemParent) && (*it != mpImportedItemParent) )
        {
            if ((*it)->isExpanded())
            {
                rList << (*it)->text(0);
            }
        }
        ++it;
    }
}

void VariableTree::getExpandedImportFiles(QStringList &rList)
{
    rList.clear();
    if (mpImportedItemParent)
    {
        QTreeWidgetItemIterator it(mpImportedItemParent);
        while(*it)
        {
            if ((*it)->isExpanded())
            {
                rList << (*it)->text(0);
            }
            ++it;
        }
    }
}

void VariableTree::expandImportFileItems(const QStringList &rList)
{
    QString fName;
    Q_FOREACH(fName, rList)
    {
        QTreeWidgetItem* pItem = mImportedFileItemMap.value(fName,0);
        if (pItem)
        {
            pItem->setExpanded(true);
        }
    }
}

void VariableTree::expandFullVariableItems(const QStringList &rList)
{
    QString cName;
    Q_FOREACH(cName, rList)
    {
        QTreeWidgetItem* pItem = mFullVariableItemMap.value(cName,0);
        if (pItem)
        {
            pItem->setExpanded(true);
        }
    }
}


//! @brief Constructor the main plot widget, which contains the tree with variables
//! @param parent Pointer to the main window
PlotWidget::PlotWidget(QWidget *pParent)
        : QWidget(pParent)
{
    mHasPendingUpdate = false;
    mpVariableTree = new VariableTree(this);

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

    QVBoxLayout *pLayout = new QVBoxLayout(this);
    pLayout->addWidget(mpVariableTree, 1);
    pLayout->addWidget(mpNewWindowButton);
    pLayout->addWidget(mpLoadButton);
    pLayout->setSpacing(1);

    connect(mpNewWindowButton, SIGNAL(clicked()), this, SLOT(openNewPlotWindow()));
    connect(mpLoadButton, SIGNAL(clicked()),this,SLOT(loadFromXml()));
    mpLoadButton->setHidden(true);      //!< @todo Fix /Peter
    //mpLoadButton->setDisabled(true);

    this->setMouseTracking(true);
}

void PlotWidget::setLogDataHandler(QPointer<LogDataHandler> pLogDataHandler)
{
    if (getLogDataHandler())
    {
        disconnect(getLogDataHandler(), 0, this, 0);
    }

    mpVariableTree->setLogDataHandler(pLogDataHandler);

    // Connect signals if the pLogdataHndler is not a null pointer
    if (pLogDataHandler)
    {
        connect(pLogDataHandler, SIGNAL(newDataAvailable()), this, SLOT(updateList()));
        connect(pLogDataHandler, SIGNAL(dataRemoved()), this, SLOT(updateList()));
    }
    updateList();
}

LogDataHandler *PlotWidget::getLogDataHandler()
{
    return mpVariableTree->getLogDataHandler();
}

void PlotWidget::setPreferedPlotWindow(QPointer<PlotWindow> pPreferedPlotWindow)
{
    mpVariableTree->setPreferedPlotWindow(pPreferedPlotWindow);
}

void PlotWidget::openNewPlotWindow()
{
    gpPlotHandler->createNewPlotWindowIfItNotAlreadyExists();
}


//! Loads a plot window from a specified .hpw file. Loads the actual plot data from a .xml file.
void PlotWidget::loadFromXml()
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

ImportedVariableTreeItem::ImportedVariableTreeItem(SharedLogVariableDataPtrT pData, QTreeWidgetItem *pParent)
    : BaseVariableTreeItem(pData, pParent)
{
    setText(0, mpData->getFullVariableName() + ", [" + mpData->getDataUnit() + "]");
}


AliasVariableTreeItem::AliasVariableTreeItem(SharedLogVariableDataPtrT pData, QTreeWidgetItem *pParent)
    : BaseVariableTreeItem(pData, pParent)
{
    setText(0, mpData->getAliasName() + ", [" + mpData->getDataUnit() + "]");
}

FullVariableTreeItem::FullVariableTreeItem(SharedLogVariableDataPtrT pData, QTreeWidgetItem *pParent)
    : BaseVariableTreeItem(pData, pParent)
{
    QString alias = mpData->getAliasName();
    const QString &portName = mpData->getPortName();
    const QString &dataName = mpData->getDataName();
    if(!alias.isEmpty())
    {
        alias.prepend("<");
        alias.append("> ");
    }
    if (portName.isEmpty())
    {
        this->setText(0, alias + dataName + ", [" +  mpData->getDataUnit() + "]");
    }
    else
    {
        this->setText(0, alias + portName + ", " + dataName + ", [" +  mpData->getDataUnit() + "]");
    }
}


ImportedFileTreeItem::ImportedFileTreeItem(const QString &rFileName, QTreeWidgetItem *pParent)
    : QTreeWidgetItem(pParent)
{
    setText(0, rFileName);
    QFont boldfont = font(0);
    boldfont.setBold(true);
    setFont(0, boldfont);
}
