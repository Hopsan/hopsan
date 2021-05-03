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
//! @file   PlotWidget2.cpp
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
#include <QApplication>
#include <QDrag>
#include <QPushButton>
#include <QTreeWidget>
#include <QInputDialog>
#include <QDialogButtonBox>

//Hopsan includes
#include "PlotWidget2.h"
#include "global.h"
#include "Configuration.h"
#include "GraphicsView.h"
#include "GUIObjects/GUIContainerObject.h"
#include "GUIObjects/GUIModelObject.h"
#include "GUIPort.h"
#include "LibraryWidget.h"
#include "loadFunctions.h"
#include "MessageWidget.h"
#include "PlotHandler.h"
#include "PlotWidget2.h"
#include "PlotWindow.h"
#include "ProjectTabWidget.h"
#include "Utilities/GUIUtilities.h"
#include "Utilities/HelpPopUpWidget.h"
#include "Widgets/FindWidget.h"
#include "LogDataHandler2.h"
#include "Widgets/TimeOffsetWidget.h"
#include "ModelHandler.h"

// Plot Widget help classes declarations
// ----------------------------------------------------------------------------

class VariableTree : public QTreeWidget
{
    Q_OBJECT
public:
    VariableTree(QWidget *pParent=0);

    void setLogDataHandler(QPointer<LogDataHandler2> pLogDataHandler);
    LogDataHandler2 *getLogDataHandler();
    void setPreferedPlotWindow(QPointer<PlotWindow> pPreferedPlotWindow);

    void addFullVariable(SharedVectorVariableT data);
    bool addAliasVariable(SharedVectorVariableT data);
    void addImportedVariable(SharedVectorVariableT data);
    void refreshImportedVariables();

    void clear();

    void getExpandedFullVariables(QStringList &rList);
    void getExpandedImportFiles(QStringList &rList);

    void expandImportFileItems(const QStringList &rList);
    void expandFullVariableItems(const QStringList &rList);

public slots:
    void updateList(const int gen);

protected slots:
    PlotWindow *plotToPreferedPlotWindow(QTreeWidgetItem *item);

protected:
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void contextMenuEvent(QContextMenuEvent *event);

    void resetImportedItemParent();
    void resetAliasItemParent();

    QTreeWidgetItem *getFullVariableComponentItem(const QString &rName)
    {
        auto cit = mFullVariableItemMap.find(rName);
        if (cit != mFullVariableItemMap.end())
        {
            return cit.value();
        }
        return 0;
    }

    int mGen=-1;

    QPointF mDragStartPosition;
    QMap<QString, QTreeWidgetItem*> mFullVariableItemMap;
    QMap<QString, QTreeWidgetItem*> mAliasVariableItemMap;
    QMap<QString, QTreeWidgetItem*> mImportedFileItemMap;

    QTreeWidgetItem* mpImportedItemParent;
    QTreeWidgetItem* mpAliasItemParent;

    //QList<VariableCommonDescription> mAvailableVariables;
    QPointer<LogDataHandler2> mpLogDataHandler;
    QPointer<PlotWindow> mpPreferedPlotWindow;
};

class BaseVariableTreeItem : public QTreeWidgetItem
{
public:
    BaseVariableTreeItem(SharedVectorVariableT data, QTreeWidgetItem *pParent);
    SharedVectorVariableT getData();
    QString getName() const;
    QString getFullName() const;
    const QString &getComponentName() const;
    const QString &getPortName() const;
    const QString &getDataName() const;
    const QString &getDataUnit() const;
    const QString &getDataQuantity() const;
    const QString &getPlotDataUnit() const;
    const QString &getAliasName() const;
    const QString &getModelName() const;

    int getGeneration() const;

protected:
    SharedVectorVariableT mData;
};

class FullVariableTreeItem : public BaseVariableTreeItem
{
public:
    FullVariableTreeItem(SharedVectorVariableT data, QTreeWidgetItem *pParent);
};

class ImportedVariableTreeItem : public BaseVariableTreeItem
{
public:
    ImportedVariableTreeItem(SharedVectorVariableT data, QTreeWidgetItem *pParent);
};

class AliasVariableTreeItem : public BaseVariableTreeItem
{
public:
    AliasVariableTreeItem(SharedVectorVariableT data, QTreeWidgetItem *pParent);
};

class ImportedFileTreeItem : public QTreeWidgetItem
{
public:
    ImportedFileTreeItem(const QString &rName, const int gen, QTreeWidgetItem *pParent);
    int getGeneration() const;
    QString getName() const;
private:
    int mGen;
};

class ComponentHeaderTreeItem : public QTreeWidgetItem
{
public:
    ComponentHeaderTreeItem(const QString &rCompName, QTreeWidgetItem *pParent);
};

class GenerationSelector : public QWidget
{
    Q_OBJECT
public:
    GenerationSelector()
    {
        mAllGensCheckBox.setChecked(false);
        mAllGensCheckBox.setToolTip("Show variables from all generations");
        mCurrentGenCheckBox.setChecked(true);
        mCurrentGenCheckBox.setToolTip("Show variables from current (newest) generations");
        mSelectGenSpinBox.setDisabled(true);
        mSelectGenSpinBox.setToolTip("Show variables from manualy selected generation");

        connect(&mAllGensCheckBox, SIGNAL(clicked(bool)), this, SLOT(selectAllToggled()));
        connect(&mCurrentGenCheckBox, SIGNAL(clicked(bool)), this, SLOT(selectCurrentToggled()));
        connect(&mSelectGenSpinBox, SIGNAL(valueChanged(int)), this, SLOT(selectManual(int)));

        QHBoxLayout *pLayout = new QHBoxLayout(this);
        pLayout->addWidget(new QLabel("Gen:"));
        pLayout->addWidget(new QLabel("All"),0,Qt::AlignRight);
        pLayout->addWidget(&mAllGensCheckBox);
        pLayout->addWidget(new QLabel("Curr"),0,Qt::AlignRight);
        pLayout->addWidget(&mCurrentGenCheckBox);
        pLayout->addWidget(new QLabel("Select:"),0,Qt::AlignRight);
        pLayout->addWidget(&mSelectGenSpinBox);
    }

    void setLogDataHandler(LogDataHandler2 *pLogDataHandler)
    {
        mpLogdataHandler = pLogDataHandler;
    }

    void refreshDisplayGen()
    {
        if(mpLogdataHandler)
        {
            int low = mpLogdataHandler->getLowestGenerationNumber();
            int high = mpLogdataHandler->getHighestGenerationNumber();
            mSelectGenSpinBox.setRange(low+1, high+1);
            if (mCurrentGenCheckBox.isChecked())
            {
                mSelectGenSpinBox.setValue(mpLogdataHandler->getCurrentGenerationNumber()+1);
            }
        }
    }

    int getGen() const
    {
        if (mCurrentGenCheckBox.isChecked())
        {
            return -1;
        }
        else if (mAllGensCheckBox.isChecked())
        {
            return -2;
        }
        else
        {
            return mSelectGenSpinBox.value()-1;
        }
    }

signals:
    void setGen(int gen);

public slots:

    void selectAllToggled()
    {
        if (mAllGensCheckBox.isChecked())
        {
            mCurrentGenCheckBox.setChecked(false);
            mSelectGenSpinBox.setDisabled(true);
            emit setGen(-2);
        }
        else
        {
            mSelectGenSpinBox.setDisabled(false);
            emit setGen(mSelectGenSpinBox.value()-1);
        }
    }

    void selectCurrentToggled()
    {
        if (mCurrentGenCheckBox.isChecked())
        {
            mAllGensCheckBox.setChecked(false);
            mSelectGenSpinBox.setDisabled(true);
            refreshDisplayGen();
            emit setGen(-1);
        }
        else
        {
            mSelectGenSpinBox.setDisabled(false);
            emit setGen(mSelectGenSpinBox.value()-1);
        }
    }

    void selectManual(int gen)
    {
        setGen(gen-1);
    }


private:
    QCheckBox mAllGensCheckBox;
    QCheckBox mCurrentGenCheckBox;
    QSpinBox mSelectGenSpinBox;
    QPointer<LogDataHandler2> mpLogdataHandler;
};

// ----------------------------------------------------------------------------



//! @brief Constructor for the variable items in the variable tree
//! @param pData Shared pointer to variable to represent
//! @param pParent Pointer to the parent tree widget item
BaseVariableTreeItem::BaseVariableTreeItem(SharedVectorVariableT data, QTreeWidgetItem *pParent)
        : QTreeWidgetItem(pParent)
{
    mData = data;
}

SharedVectorVariableT BaseVariableTreeItem::getData()
{
    return mData;
}

QString BaseVariableTreeItem::getName() const
{
    return mData->getSmartName();

}

QString BaseVariableTreeItem::getFullName() const
{
    return mData->getFullVariableName();
}


//! @brief Returns the name of the component where the variable is located
const QString &BaseVariableTreeItem::getComponentName() const
{
    return mData->getComponentName();
}


//! @brief Returns the name of the port where the variable is located
const QString &BaseVariableTreeItem::getPortName() const
{
    return mData->getPortName();
}


//! @brief Returns the name of the variable
const QString &BaseVariableTreeItem::getDataName() const
{
    return mData->getDataName();
}


//! @brief Returns the name of the unit of the variable
const QString &BaseVariableTreeItem::getDataUnit() const
{
    return mData->getDataUnit();
}

const QString &BaseVariableTreeItem::getDataQuantity() const
{
    return mData->getDataQuantity();
}

const QString &BaseVariableTreeItem::getPlotDataUnit() const
{
    //return mData->getActualPlotDataUnit();
    return mData->getDataUnit();
}

const QString &BaseVariableTreeItem::getAliasName() const
{
    return mData->getAliasName();
}

const QString &BaseVariableTreeItem::getModelName() const
{
    return mData->getModelPath();
}

int BaseVariableTreeItem::getGeneration() const
{
    return mData->getGeneration();
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

void VariableTree::setLogDataHandler(QPointer<LogDataHandler2> pLogDataHandler)
{
    mpLogDataHandler = pLogDataHandler;
}

LogDataHandler2 *VariableTree::getLogDataHandler()
{
    return mpLogDataHandler;
}

void VariableTree::setPreferedPlotWindow(QPointer<PlotWindow> pPreferedPlotWindow)
{
    mpPreferedPlotWindow = pPreferedPlotWindow;
}

void VariableTree::addFullVariable(SharedVectorVariableT data)
{
    QTreeWidgetItem *pComponentItem=0, *pSystemItem=0;

    SharedSystemHierarchyT systemHiearchy =  data->getSystemHierarchy();
    QString fullSystemName;
    if (systemHiearchy && !systemHiearchy->isEmpty())
    {
        for (QString &systemName : *systemHiearchy.data())
        {
           //! @todo should try to get system names in reverse maybe, to reduce searching time every time
           QString prevFullSystemName = fullSystemName;
           fullSystemName += systemName+"$";

           // Try to find the component if it has already been added
           pSystemItem = getFullVariableComponentItem(fullSystemName);

           // If we did not find it, then this is the first time the component is added, lets create and add that top-level item
           if (!pSystemItem)
           {
               QTreeWidgetItem *pParentSystemItem = getFullVariableComponentItem(prevFullSystemName);
               pSystemItem = new ComponentHeaderTreeItem(systemName, pParentSystemItem);
               if (pParentSystemItem)
               {
                   pParentSystemItem->addChild(pSystemItem);
               }
               else
               {
                   addTopLevelItem(pSystemItem);
               }

               //Also remember that we created it
               mFullVariableItemMap.insert(fullSystemName, pSystemItem);
           }
        }
    }

    // Did we have the top-level component item (in that case use it)
    QString cname = data->getComponentName();
    if (cname.isEmpty())
    {
        // Mostly for the time variable
        cname = data->getDataName();
    }

    // Determine full Component name, we append $ to make name lookup same as for systems (for interface ports)
    QString fullComponentName = fullSystemName+cname+"$";

    // Try to find the component if it has already been added
    pComponentItem = getFullVariableComponentItem(fullComponentName);

    // If we did not find it, then this is the first time the component is added, lets create and add that top-level item
    if (!pComponentItem)
    {
        pComponentItem = new ComponentHeaderTreeItem(cname, pSystemItem);
        if (pSystemItem)
        {
            pSystemItem->addChild(pComponentItem);
        }
        else
        {
            addTopLevelItem(pComponentItem);
        }

        //Also remember that we created it
        mFullVariableItemMap.insert(fullComponentName, pComponentItem);
    }

    // Add the actual variable item
    pComponentItem->addChild(new FullVariableTreeItem(data, pComponentItem));
    this->show();
}

bool VariableTree::addAliasVariable(SharedVectorVariableT data)
{
    // Prevent adding the same alias again
    if (!mAliasVariableItemMap.contains(data->getAliasName()))
    {
        AliasVariableTreeItem *pAliasItem = new AliasVariableTreeItem(data, 0);
        mAliasVariableItemMap.insert(data->getAliasName(), pAliasItem);
        mpAliasItemParent->addChild(pAliasItem);
        mpAliasItemParent->setHidden(false);
        mpAliasItemParent->setExpanded(true);
        return true;
    }
    else
    {
        return false;
    }
}

void VariableTree::addImportedVariable(SharedVectorVariableT data)
{
    QString fName = QString("%1 (%2)").arg(data->getImportedFileName()).arg(data->getGeneration()+1);
    QTreeWidgetItem *pFileItem = mImportedFileItemMap.value(fName, 0);
    // If this file was not already added then create it
    if (!pFileItem)
    {
        pFileItem = new ImportedFileTreeItem(fName,data->getGeneration(),0);

        // Also remember that we created it
        mImportedFileItemMap.insert(fName, pFileItem);
        mpImportedItemParent->addChild(pFileItem);
    }

    // Add a sub item with data name name
    new ImportedVariableTreeItem(data, pFileItem);
    mpImportedItemParent->setHidden(false);
    mpImportedItemParent->setExpanded(true);
}

void VariableTree::refreshImportedVariables()
{
    resetImportedItemParent();
    QList<int> importedGens = mpLogDataHandler->getImportedGenerations();
    for (int g : importedGens)
    {
        QList<SharedVectorVariableT> vars = mpLogDataHandler->getAllVariablesAtGeneration(g);
        for (auto &var : vars)
        {
            addImportedVariable(var);
        }
    }
}


//! @brief Refresh the list for given generation
//! @param gen The generation to show, (0 index based) -2 == ALL -1 == Current
void VariableTree::updateList(const int gen)
{
    mGen = gen;

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

    // Now add variables to the Alias and Variable tree depending on selected generation to display
    QList<SharedVectorVariableT> variables;

    if (gen == -1)
    {
        variables = getLogDataHandler()->getAllVariablesAtCurrentGeneration();
    }
    else if (gen >= 0)
    {
        variables = getLogDataHandler()->getAllVariablesAtGeneration(gen);
    }
    else
    {
        variables = getLogDataHandler()->getAllVariablesAtRespectiveNewestGeneration();
    }

    for(auto &var : variables)
    {
        // Skip data that is only imported, (they are handled above)
        if (var->isImported())
        {
            continue;
        }

        // Handle adding as alias variable
        bool addedAlias=false;
        if ( var->hasAliasName() )
        {
            addedAlias = addAliasVariable(var);
        }

        // If it was not added as an alias then add as ordinary variable
        if (!addedAlias)
        {
            // Handle all non-temp non-imported non-alias variables
            addFullVariable(var);
        }
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
void PlotWidget2::updateList()
{
    // There is not point in updating if we are not visible
    if (isVisible())
    {
        mpGenerationSelector->refreshDisplayGen();
        mpVariableTree->updateList(mpGenerationSelector->getGen());

        mHasPendingUpdate = false;
    }
    else
    {
        // Remember that we want to update, until the next time we show
        mHasPendingUpdate = true;
    }
}

void PlotWidget2::clearList()
{
    mpVariableTree->clear();
    mpGenerationSelector->setLogDataHandler(0);
}


//! @brief Helper function that plots item to preferred plotwindow or creates a new one
//! @param[in] *item Pointer to the tree widget item who should be plotted
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
            return gpPlotHandler->plotDataToWindow(mpPreferedPlotWindow, pVariableItem->getData(), QwtPlot::yLeft);
        }
        else
        {
            return gpPlotHandler->plotDataToWindow(0, pVariableItem->getData(), QwtPlot::yLeft);
        }
    }
    return 0;
}

//! @brief Defines what happens when clicking in the variable list. Used to initiate drag operations.
void VariableTree::mousePressEvent(QMouseEvent *event)
{
    gpHelpPopupWidget->showHelpPopupMessage("Double click on a variable to open a new plot window, or drag it to an existing one.");
    QTreeWidget::mousePressEvent(event);

    if (event->button() == Qt::LeftButton)
        mDragStartPosition = event->pos();
}


//! @brief Defines what happens when mouse is moving in variable list. Used to handle drag operations.
void VariableTree::mouseMoveEvent(QMouseEvent *event)
{
    //! @todo maybe should try to be smart with a local function that selects plotwindow if one is set (for message)
    gpHelpPopupWidget->showHelpPopupMessage("Double click on a variable to plot it, or drag it to an existing plot window.");
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
        mimeText = QString("HOPSANPLOTDATA:%1:%2:%3").arg(item->getName()).arg(item->getGeneration()).arg(item->getModelName());
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

    // Build context menu for variable items
    BaseVariableTreeItem *pItem = dynamic_cast<BaseVariableTreeItem *>(currentItem());
    if(pItem)
    {
        bool isImportVariabel = (dynamic_cast<ImportedVariableTreeItem*>(currentItem()) != 0);
        bool isAliasVariabel = (dynamic_cast<AliasVariableTreeItem*>(currentItem()) != 0);

        QMenu menu;
        QAction *pDefineAliasAction = 0;
        QAction *pUndefineAliasAction = 0;
        QAction *pFindAliasAction = 0;
        QAction *pDeleteVariableThisGenAction = 0;
        QAction *pDeleteVariableAllGenAction = 0;
        QAction *pSetQuantityAction = 0;
        QAction *pInvertAction = 0;
        QAction *pSetTimePlotOffsetAction = 0;

        // Add actions
        if (!isImportVariabel)
        {
            if (!isAliasVariabel)
            {
                // Only show alias buttons for non imported variables
                if(pItem->getAliasName().isEmpty())
                {
                    pDefineAliasAction = menu.addAction(QString("Define Variable Alias"));
                }
                else
                {
                    pDefineAliasAction = menu.addAction(QString("Change Alias"));
                    pUndefineAliasAction = menu.addAction(QString("Undefine Alias"));
                    pFindAliasAction = menu.addAction("Find Alias");
                }
            }
            else
            {
                // Only show alias commands if the alias represent the actual current alias in the model (Avoids problems with aliases lingering in old generations after rename or similar)
                QString actualAlias = mpLogDataHandler->getParentModel()->getVariableAlias(pItem->getFullName());
                if (actualAlias == pItem->getAliasName())
                {
                    pDefineAliasAction = menu.addAction(QString("Change Alias"));
                    pUndefineAliasAction = menu.addAction(QString("Undefine Alias"));
                    pFindAliasAction = menu.addAction("Find Alias");
                }
            }
            menu.addSeparator();
        }
        pDeleteVariableThisGenAction = menu.addAction(QString("Remove Variable @%1").arg(pItem->getGeneration()+1));
        pDeleteVariableAllGenAction = menu.addAction(QString("Remove Variable @*"));

        menu.addSeparator();
        pSetQuantityAction = menu.addAction("Set plot quantity");
        pInvertAction = menu.addAction("Invert plot");

        if (pItem->getDataName() == TIMEVARIABLENAME && pItem->getComponentName().isEmpty())
        {
            pSetTimePlotOffsetAction = menu.addAction("Set Time Offset");
        }

        // Execute menu and wait for selected action
        QAction *pSelectedAction = menu.exec(QCursor::pos());
        if (pSelectedAction != 0)
        {
            // Execute selected action
            if(pSelectedAction == pUndefineAliasAction)
            {
                mpLogDataHandler->getParentModel()->undefineVariableAlias(pItem->getFullName());
            }
            else if(pSelectedAction == pDefineAliasAction)
            {
                mpLogDataHandler->getParentModel()->defineVariableAlias(pItem->getFullName());
            }
            else if(pSelectedAction == pFindAliasAction)
            {
                gpFindWidget->findAlias(pItem->getAliasName());
            }
            else if (pSelectedAction == pDeleteVariableThisGenAction)
            {
                if (isAliasVariabel)
                {
                    mpLogDataHandler->unregisterAlias(pItem->getAliasName(), pItem->getGeneration());
                }
                else
                {
                    mpLogDataHandler->removeVariable(pItem->getFullName(), pItem->getGeneration());
                }
            }
            else if (pSelectedAction == pDeleteVariableAllGenAction)
            {
                if (isAliasVariabel)
                {
                    //! @todo -2 (all) is not supported right now (will only remove current) /Peter
                    mpLogDataHandler->unregisterAlias(pItem->getAliasName(), -2);
                }
                else
                {
                    mpLogDataHandler->removeVariable(pItem->getFullName(), -2);
                }
            }
            else if (pSelectedAction == pSetQuantityAction)
            {
                bool ok;
                QString q = QInputDialog::getText(gpMainWindowWidget, gpMainWindowWidget->tr("Set Quantity"),
                                                  QString("Quantity for: %1").arg(pItem->getFullName()), QLineEdit::Normal, pItem->getDataQuantity(), &ok);
                if(ok)
                {
                    ok = mpLogDataHandler->registerQuantity(pItem->getFullName(), q, pItem->getGeneration());
                    if (ok)
                    {
                        updateList(mGen);
                    }
                }
            }
            else if (pSelectedAction == pInvertAction)
            {
                SharedVectorVariableT pVar = mpLogDataHandler->getVectorVariable(pItem->getFullName(), pItem->getGeneration());
                if (pVar)
                {
                    pVar->togglePlotInverted();
                }
            }
            else if (pSelectedAction == pSetTimePlotOffsetAction)
            {
                QDialog dialog;
                QVBoxLayout *pLayout = new QVBoxLayout(&dialog);
                pLayout->addWidget(new TimeOffsetWidget(pItem->getGeneration(), mpLogDataHandler, &dialog));
                QDialogButtonBox *pButtonbox = new QDialogButtonBox(Qt::Horizontal, &dialog);
                pButtonbox->addButton(QDialogButtonBox::Ok);
                pLayout->addWidget(pButtonbox,0,Qt::AlignRight);
                connect(pButtonbox, SIGNAL(accepted()), &dialog, SLOT(close()));
                dialog.exec();
            }
        }
    }

    // Build context menu for imported file items
    ImportedFileTreeItem *pFileItem = dynamic_cast<ImportedFileTreeItem *>(currentItem());
    if (pFileItem)
    {
        QMenu menu;
        QAction *pRemovefileAction = menu.addAction("Unload File");
        QAction *selectedAction = menu.exec(QCursor::pos());
        if(selectedAction == pRemovefileAction)
        {
           mpLogDataHandler->removeGeneration(pFileItem-> getGeneration(), true);
        }
    }

    // Build context menu for component header items
    ComponentHeaderTreeItem *pComponentItem = dynamic_cast<ComponentHeaderTreeItem *>(currentItem());
    if (pComponentItem)
    {
        QMenu menu;
        QAction *pFindComponentAction = menu.addAction("Find Component");
        QAction *selectedAction = menu.exec(QCursor::pos());
        if (selectedAction == pFindComponentAction)
        {
            gpFindWidget->findComponent(pComponentItem->text(0));
        }
    }
}

void PlotWidget2::showEvent(QShowEvent *event)
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
                rList << mFullVariableItemMap.key(*it);;
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
                rList << mImportedFileItemMap.key(*it);
            }
            ++it;
        }
    }
}

void VariableTree::expandImportFileItems(const QStringList &rList)
{
    for(const QString &fName : rList) {
        QTreeWidgetItem* pItem = mImportedFileItemMap.value(fName, nullptr);
        if (pItem) {
            pItem->setExpanded(true);
        }
    }
}

void VariableTree::expandFullVariableItems(const QStringList &rList)
{
    for(const QString &cName : rList) {
        QTreeWidgetItem* pItem = mFullVariableItemMap.value(cName, nullptr);
        if (pItem)
        {
            pItem->setExpanded(true);
        }
    }
}


//! @brief Constructor the main plot widget, which contains the tree with variables
//! @param parent Pointer to the main window
PlotWidget2::PlotWidget2(QWidget *pParent)
        : QWidget(pParent)
{
    mHasPendingUpdate = false;
    mpVariableTree = new VariableTree(this);

    QPushButton *pNewWindowButton = new QPushButton(tr("&Open New Plot Window"), this);
    pNewWindowButton->setAutoDefault(false);
    pNewWindowButton->setFixedHeight(30);
    QFont tempFont = pNewWindowButton->font();
    tempFont.setBold(true);
    pNewWindowButton->setFont(tempFont);

    mpGenerationSelector = new GenerationSelector();
    connect(mpGenerationSelector, SIGNAL(setGen(int)), mpVariableTree,  SLOT(updateList(int)));

    QVBoxLayout *pLayout = new QVBoxLayout(this);
    pLayout->addWidget(mpVariableTree, 1);
    pLayout->addWidget(mpGenerationSelector);
    pLayout->addWidget(pNewWindowButton);
    pLayout->setSpacing(1);

    connect(pNewWindowButton, SIGNAL(clicked()), this, SLOT(openNewPlotWindow()));

    this->setMouseTracking(true);
}

void PlotWidget2::setLogDataHandler(QPointer<LogDataHandler2> pLogDataHandler)
{
    if (getLogDataHandler())
    {
        disconnect(getLogDataHandler(), 0, this, 0);
    }

    // Remember expanded items
    if (getLogDataHandler())
    {
        QStringList expandedFullVars, expandedImportedFiles;
        mpVariableTree->getExpandedFullVariables(expandedFullVars);
        mpVariableTree->getExpandedImportFiles(expandedImportedFiles);
        mExpandedItems.insert(getLogDataHandler()->getParentModel()->getTopLevelSystemContainer()->getModelFilePath(), QPair<QStringList, QStringList>(expandedFullVars, expandedImportedFiles));
    }

    mpGenerationSelector->setLogDataHandler(pLogDataHandler);
    mpVariableTree->setLogDataHandler(pLogDataHandler);

    // Connect signals if the pLogDataHandler is not a null pointer
    if (pLogDataHandler)
    {
        connect(pLogDataHandler, SIGNAL(dataAdded()), this, SLOT(updateList()));
        connect(pLogDataHandler, SIGNAL(dataRemoved()), this, SLOT(updateList()));
        connect(pLogDataHandler, SIGNAL(aliasChanged()), this, SLOT(updateList()));
        connect(pLogDataHandler, SIGNAL(quantityChanged()), this, SLOT(updateList()));
    }
    updateList();

    // Restore expanded items, also clear any obsolete log data handlers
    if (pLogDataHandler && mExpandedItems.contains(pLogDataHandler->getParentModel()->getTopLevelSystemContainer()->getModelFilePath()))
    {
        QPair<QStringList, QStringList> lists = mExpandedItems.value(pLogDataHandler->getParentModel()->getTopLevelSystemContainer()->getModelFilePath());
        mpVariableTree->expandFullVariableItems(lists.first);
        mpVariableTree->expandImportFileItems(lists.second);

        // Clear obsolete
        for (auto it=mExpandedItems.begin(); it!=mExpandedItems.end(); ++it)
        {
            if (!gpModelHandler->getModel(it.key()))
            {
                mExpandedItems.erase(it);
                it=mExpandedItems.begin();
            }
        }
    }
}

LogDataHandler2 *PlotWidget2::getLogDataHandler()
{
    return mpVariableTree->getLogDataHandler();
}

void PlotWidget2::setPreferedPlotWindow(QPointer<PlotWindow> pPreferedPlotWindow)
{
    mpVariableTree->setPreferedPlotWindow(pPreferedPlotWindow);
}

void PlotWidget2::openNewPlotWindow()
{
    gpPlotHandler->createNewPlotWindowIfItNotAlreadyExists();
}


ImportedVariableTreeItem::ImportedVariableTreeItem(SharedVectorVariableT data, QTreeWidgetItem *pParent)
    : BaseVariableTreeItem(data, pParent)
{
    QString uq = getDataQuantity();
    if (uq.isEmpty())
    {
        uq = getPlotDataUnit();
    }
    setText(0, getFullName() + ", [" + uq + "]");
}


AliasVariableTreeItem::AliasVariableTreeItem(SharedVectorVariableT data, QTreeWidgetItem *pParent)
    : BaseVariableTreeItem(data, pParent)
{
    QString uq = getDataQuantity();
    if (uq.isEmpty())
    {
        uq = getPlotDataUnit();
    }
    setText(0, getAliasName() + ", [" + uq + "]");
}

FullVariableTreeItem::FullVariableTreeItem(SharedVectorVariableT data, QTreeWidgetItem *pParent)
    : BaseVariableTreeItem(data, pParent)
{
    QString alias = getAliasName();
    const QString &portName = getPortName();
    const QString &dataName = getDataName();
    if(!alias.isEmpty())
    {
        alias.prepend("<");
        alias.append("> ");
    }
    if (portName.isEmpty())
    {
        this->setText(0, alias + dataName + ", [" +  getPlotDataUnit() + "]");
    }
    else
    {
        this->setText(0, alias + portName + ", " + dataName + ", [" +  getPlotDataUnit() + "]");
    }
}


ImportedFileTreeItem::ImportedFileTreeItem(const QString &rName, const int gen, QTreeWidgetItem *pParent)
    : QTreeWidgetItem(pParent)
{
    mGen = gen;
    QFileInfo fileInfo(rName);
    setText(0, fileInfo.fileName());
    setToolTip(0, rName);
    QFont boldfont = font(0);
    boldfont.setBold(true);
    setFont(0, boldfont);
}

int ImportedFileTreeItem::getGeneration() const
{
    return mGen;
}

QString ImportedFileTreeItem::getName() const
{
    return toolTip(0);
}


ComponentHeaderTreeItem::ComponentHeaderTreeItem(const QString &rCompName, QTreeWidgetItem *pParent)
    : QTreeWidgetItem(pParent)
{
    setText(0, rCompName);
    QFont tempFont = font(0);
    tempFont.setBold(true);
    setFont(0, tempFont);
}

#include "PlotWidget2.moc"

