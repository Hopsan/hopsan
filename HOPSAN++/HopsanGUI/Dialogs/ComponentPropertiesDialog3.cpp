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
//! @file   ComponentPropertiesDialog3.cpp
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2010-03-01
//!
//! @brief Contains a dialog class for changing component properties
//!
//$Id: ComponentPropertiesDialog3.cpp 4807 2012-11-28 14:07:11Z petno25 $

#include <QtGui>
#include <QDebug>

#include "ComponentPropertiesDialog3.h"

#include "MainWindow.h"
#include "Configuration.h"

#include "UndoStack.h"
#include "GUIPort.h"

#include "Widgets/MessageWidget.h"

#include "Widgets/SystemParametersWidget.h"
#include "Widgets/LibraryWidget.h"

#include "GUIObjects/GUIComponent.h"
#include "GUIObjects/GUIContainerObject.h"

#include "Utilities/GUIUtilities.h"
#include "Dialogs/MovePortsDialog.h"
#include "Dialogs/ParameterSettingsLayout.h"




//! @class ComponentPropertiesDialog3
//! @brief The ComponentPropertiesDialog3 class is a Widget used to interact with component parameters.
//!
//! It reads and writes parameters to the core components.
//!


//! @brief Constructor for the parameter dialog for components
//! @param pGUIComponent Pointer to the component
//! @param parent Pointer to the parent widget
ComponentPropertiesDialog3::ComponentPropertiesDialog3(Component *pComponent, QWidget *pParent)
    : ModelObjectPropertiesDialog(pComponent, pParent)
{
    mpComponent = pComponent;
    this->setPalette(gConfig.getPalette());

    setWindowTitle(tr("Component Properties"));
    mpMainLayout = new QGridLayout(this);

    // Parents to new objects bellow should be set automatically when adding layout or widget to other layout or widget

    // Add help picture and text
    //------------------------------------------------------------------------------------------------------------------------------
    if(!mpComponent->getHelpText().isNull() || !mpComponent->getHelpPicture().isNull())
    {
        QGroupBox *pHelpGroupBox = new QGroupBox();
        QVBoxLayout *pHelpLayout = new QVBoxLayout();

        QLabel *pHelpHeading = new QLabel(gpMainWindow->mpLibrary->getAppearanceData(mpComponent->getTypeName())->getDisplayName());
        pHelpHeading->setAlignment(Qt::AlignCenter);
        QFont tempFont = pHelpHeading->font();
        tempFont.setPixelSize(16);
        tempFont.setBold(true);
        pHelpHeading->setFont(tempFont);
        pHelpLayout->addWidget(pHelpHeading);

        if(!mpComponent->getHelpPicture().isNull())
        {
            QLabel *pHelpPicture = new QLabel();
            QPixmap helpPixMap(mpComponent->getAppearanceData()->getBasePath() + mpComponent->getHelpPicture());
            pHelpPicture->setPixmap(helpPixMap);
            pHelpPicture->setAlignment(Qt::AlignCenter);
            pHelpLayout->addWidget(pHelpPicture);
        }

        if(!mpComponent->getHelpText().isNull())
        {
            QLabel *pHelpText = new QLabel(mpComponent->getHelpText(), this);
            pHelpText->setWordWrap(true);
            pHelpLayout->addWidget(pHelpText);
        }

        pHelpGroupBox->setStyleSheet(QString::fromUtf8("QGroupBox {background-color: white; border: 2px solid gray; border-radius: 5px; margin-top: 1ex;}"));
        pHelpGroupBox->setLayout(pHelpLayout);

        mpMainLayout->addWidget(pHelpGroupBox, 0, 0, 1, 2);
    }
    //------------------------------------------------------------------------------------------------------------------------------

    // Add name edit and type information
    //------------------------------------------------------------------------------------------------------------------------------
    QGridLayout *pNameTypeLayout = new QGridLayout();
    QLabel *pNameLabel = new QLabel("Name: ", this);
    mpNameEdit = new QLineEdit(mpComponent->getName(), this);
    QLabel *pTypeNameLabel = new QLabel("Type Name: \"" + mpComponent->getTypeName() + "\"", this);
    pNameTypeLayout->addWidget(pNameLabel,0,0);
    pNameTypeLayout->addWidget(mpNameEdit,0,1);
    pNameTypeLayout->addWidget(pTypeNameLabel,1,0,1,2);
    if (!mpComponent->getSubTypeName().isEmpty())
    {
        QLabel *pSubTypeNameLabel = new QLabel("SubType Name: \"" + mpComponent->getSubTypeName() + "\"", this);
        pNameTypeLayout->addWidget(pSubTypeNameLabel,2,0,1,2);
    }
    mpMainLayout->addLayout(pNameTypeLayout, mpMainLayout->rowCount(), 0);
    //------------------------------------------------------------------------------------------------------------------------------

    // Add button box with buttons
    //------------------------------------------------------------------------------------------------------------------------------
    QPushButton *pEditPortPos = new QPushButton(tr("&Move ports"), this);
    QPushButton *pCancelButton = new QPushButton(tr("&Cancel"), this);
    QPushButton *pOkButton = new QPushButton(tr("&Ok"), this);
    pOkButton->setDefault(true);
    QDialogButtonBox *pButtonBox = new QDialogButtonBox(Qt::Vertical, this);
    pButtonBox->addButton(pOkButton, QDialogButtonBox::ActionRole);
    pButtonBox->addButton(pCancelButton, QDialogButtonBox::ActionRole);
    pButtonBox->addButton(pEditPortPos, QDialogButtonBox::ActionRole);
    connect(pOkButton, SIGNAL(clicked()), this, SLOT(okPressed()));
    connect(pCancelButton, SIGNAL(clicked()), this, SLOT(close()));
    connect(pEditPortPos, SIGNAL(clicked()), this, SLOT(editPortPos()));
    mpMainLayout->addWidget(pButtonBox, mpMainLayout->rowCount()-1, 1);
    //------------------------------------------------------------------------------------------------------------------------------

    // Add Parameter settings table
    //------------------------------------------------------------------------------------------------------------------------------
    mpVariableTableWidget = new VariableTableWidget(mpComponent,this);
    mpMainLayout->addWidget(mpVariableTableWidget);

    //------------------------------------------------------------------------------------------------------------------------------

    this->setLayout(mpMainLayout);
}


//! @brief Check if the parameter is a start value
//! @param [in,out] parameterDescription The description of the parameter/startvalue
//! @returns true if it is a startvalue, otherwise false
//!
//! This method is used to determine whether or not a parameter should be interpretted
//! as a start value by the GUI. In HOPSANcore there is no difference between parameters
//! and start values. The start values are registred and stored in the same container.
//! But, a start value is taged by "startvalue:" in the description.
bool ComponentPropertiesDialog3::interpretedAsStartValue(QString &parameterDescription)
{
    QString startValueString = "startvalue:";
    bool res=false;
    if(parameterDescription.contains(startValueString, Qt::CaseInsensitive))
    {
        parameterDescription.remove(startValueString, Qt::CaseInsensitive);
        res = true;
    }
    return res;
}


//! @brief Creates the contents in the parameter dialog
void ComponentPropertiesDialog3::createEditStuff()
{
    QFont fontH1;
    fontH1.setBold(true);
}


//! @brief Reads the values from the dialog and writes them into the core component
void ComponentPropertiesDialog3::okPressed()
{
    setName();
    setParametersAndStartValues();
    close();
}


void ComponentPropertiesDialog3::editPortPos()
{
    //! @todo who owns the dialog, is it ever removed?
    MovePortsDialog *dialog = new MovePortsDialog(mpComponent->getAppearanceData(), mpComponent->getParentContainerObject()->getGfxType());
    connect(dialog, SIGNAL(finished()), mpComponent, SLOT(refreshExternalPortsAppearanceAndPosition()), Qt::UniqueConnection);
}


//! @brief Sets the parameters and start values in the core component. Read the values from the dialog and write them into the core component.
//! @see setParametersAndStartValues(QVector<ParameterLayout *> vParLayout)
void ComponentPropertiesDialog3::setParametersAndStartValues()
{
//    if(setParameterValues(mvParameterLayout) && setParameterValues(mvStartValueLayout))
//    {
//        qDebug() << "Parameters and start values updated.";
//        this->close();
//    }
}

void ComponentPropertiesDialog3::setName()
{
    mpComponent->getParentContainerObject()->renameModelObject(mpComponent->getName(), mpNameEdit->text());
}


VariableTableWidget::VariableTableWidget(Component *pComponent, QWidget *pParent) :
    QTableWidget(pParent)
{
    mpComponent = pComponent;

    this->setColumnCount(NumCols);
    QVector<CoreParameterData> parameters;
    mpComponent->getParameters(parameters);

    // Decide which parameters should be shown as Constants and what should be shown as variables
    QVector<int> constantsIds, variablesIds;
    constantsIds.reserve(parameters.size());
    variablesIds.reserve(parameters.size());
    for (int i=0; i<parameters.size(); ++i)
    {
        if (parameters[i].mName.contains("::"))
        {
            variablesIds.push_back(i);
        }
        else
        {
            constantsIds.push_back(i);
        }
    }

    QStringList columnHeaders;
    columnHeaders.append("Name");
    columnHeaders.append("Alias");
    columnHeaders.append("Unit");
    columnHeaders.append("Description");
    columnHeaders.append("Type");
    columnHeaders.append("Value");
    columnHeaders.append("PlotScale");
    columnHeaders.append("Buttons");
    this->setHorizontalHeaderLabels(columnHeaders);

    int r=0;
    for (int i=0; i<constantsIds.size(); ++i)
    {
        createTableRow(r, parameters[constantsIds[i]]);
        ++r;
    }

    QString currPortName;
    for (int i=0; i<variablesIds.size(); ++i)
    {
        // Extract current port name to see if we should make a separator
        QString portName = parameters[variablesIds[i]].mName.split("::").at(0);
        if (portName != currPortName)
        {
            createSeparatorRow(r,"Port: "+portName);
            currPortName = portName;
            ++r;
        }
        createTableRow(r, parameters[variablesIds[i]]);
        ++r;
    }

    resizeColumnToContents(Name);
    resizeColumnToContents(Unit);
    resizeColumnToContents(Type);


}

void VariableTableWidget::resetDefaultValueAtRow(int row)
{
    QString name = item(row,Name)->text();
    if(mpComponent)
    {
        QString defaultText = mpComponent->getDefaultParameterValue(name);
        if(defaultText != QString())
            item(row,Value)->setText(defaultText);
        //pickValueTextColor();
        //! @todo color
    }
}

void VariableTableWidget::selectSystemParameterAtRow(int row)
{
    QMenu menu;
    QMap<QAction*, QString> actionParamMap;

    QVector<CoreParameterData> paramDataVector;
    mpComponent->getParentContainerObject()->getParameters(paramDataVector);

    for (int i=0; i<paramDataVector.size(); ++i)
    {
        QAction *tempAction = menu.addAction(paramDataVector[i].mName+" = "+paramDataVector[i].mValue);
        tempAction->setIconVisibleInMenu(false);
        actionParamMap.insert(tempAction, paramDataVector[i].mName);
    }

    QCursor cursor;
    QAction *selectedAction = menu.exec(cursor.pos());
    QString parNameString = actionParamMap.value(selectedAction);
    if(!parNameString.isEmpty())
    {
        item(row,Value)->setText(parNameString);
    }
}

void VariableTableWidget::makePortAtRow(int row, bool isPort)
{

//! @todo Do this,
//! @todo hmm it does not make sense to have startvalues as ports (or maybe it does, but startvalues are run before init and simulate), but then you could have startvalue to startvalue to startvalue ...

//    if (isPort)
//    {
//        Port * pPort = mpModelObject->createRefreshExternalDynamicParameterPort(mName);
//        if (pPort)
//        {
//            // Make sure that our new port has the "correct" angle
//            pPort->setRotation(180);
//        }
//    }
//    else
//    {
//        mpModelObject->removeExternalPort(mName);
//    }

}

void VariableTableWidget::createTableRow(const int row, const CoreParameterData &rData)
{
    QTableWidgetItem *pItem;
    insertRow(row);

    pItem = new QTableWidgetItem(rData.mName);
    pItem->setTextAlignment(Qt::AlignCenter);
    pItem->setFlags(Qt::NoItemFlags | Qt::ItemIsEnabled);
    setItem(row,Name,pItem);

    pItem = new QTableWidgetItem(rData.mAlias);
    pItem->setTextAlignment(Qt::AlignCenter);
    pItem->setFlags(Qt::NoItemFlags);
    pItem->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled);
    setItem(row,Alias,pItem);

    pItem = new QTableWidgetItem(parseVariableUnit(rData.mUnit));
    pItem->setTextAlignment(Qt::AlignCenter);
    pItem->setFlags(Qt::NoItemFlags | Qt::ItemIsEnabled);
    setItem(row,Unit,pItem);

    pItem = new QTableWidgetItem(rData.mDescription);
    pItem->setTextAlignment(Qt::AlignCenter);
    pItem->setFlags(Qt::NoItemFlags | Qt::ItemIsEnabled);
    setItem(row,Description,pItem);

    pItem = new QTableWidgetItem(rData.mType);
    pItem->setTextAlignment(Qt::AlignCenter);
    pItem->setFlags(Qt::NoItemFlags | Qt::ItemIsEnabled);
    setItem(row,Type,pItem);

    pItem = new QTableWidgetItem(rData.mValue);
    pItem->setTextAlignment(Qt::AlignCenter);
    pItem->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled);
    setItem(row,Value,pItem);

    pItem = new QTableWidgetItem("-1");
    pItem->setTextAlignment(Qt::AlignCenter);
    pItem->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled);
    setItem(row,Scale,pItem);

    // Set tool buttons
    QToolButton *pResetDefaultToolButton = new RowAwareToolButton(row);
    pResetDefaultToolButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-ResetDefault.png"));
    pResetDefaultToolButton->setToolTip("Reset Default Value");
    this->setIndexWidget(model()->index(row,ResetButton), pResetDefaultToolButton);
    connect(pResetDefaultToolButton, SIGNAL(triggeredAtRow(int)), this, SLOT(resetDefaultValueAtRow(int)));

    QToolButton *pSystemParameterToolButton = new RowAwareToolButton(row);
    pSystemParameterToolButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-SystemParameter.png"));
    pSystemParameterToolButton->setToolTip("Map To System Parameter");
    this->setIndexWidget(model()->index(row,SysparButton), pSystemParameterToolButton);
    connect(pSystemParameterToolButton, SIGNAL(triggeredAtRow(int)), this, SLOT(selectSystemParameterAtRow(int)));

    //! @todo buttons
}

void VariableTableWidget::createSeparatorRow(const int row, const QString name)
{
    QTableWidgetItem *pItem;
    pItem = new QTableWidgetItem(name);
    pItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    pItem->setFlags(Qt::NoItemFlags | Qt::ItemIsEnabled);
    pItem->setBackgroundColor(Qt::lightGray);
    insertRow(row);
    setItem(row,Name,pItem);
    int c=1;
    if (name.isEmpty()) {c=0;}
    for (;c<columnCount(); ++c)
    {
        pItem = new QTableWidgetItem();
        pItem->setFlags(Qt::NoItemFlags);
        pItem->setBackgroundColor(Qt::lightGray);
        setItem(row,c,pItem);
    }
    resizeRowToContents(row);
}
