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
//! @file   ComponentPropertiesDialog2.cpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-03-01
//!
//! @brief Contains a dialog class for changing component properties
//!
//$Id$

#include <QtGui>
#include <QDebug>

#include "ComponentPropertiesDialog2.h"

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


//! @class ComponentPropertiesDialog2
//! @brief The ComponentPropertiesDialog2 class is a Widget used to interact with component parameters.
//!
//! It reads and writes parameters to the core components.
//!


//! @brief Constructor for the parameter dialog for components
//! @param pGUIComponent Pointer to the component
//! @param parent Pointer to the parent widget
ComponentPropertiesDialog2::ComponentPropertiesDialog2(Component *pComponent, MainWindow *pParent)
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
    mpParameterTableModel = new ParametersTableModel(mpComponent, this);
    mpParameterTableView = new QTableView(this);
    mpParameterTableView->setModel(mpParameterTableModel);
    mpParameterTableView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    SysParSelectButtonDelegate *pPushButtonDelegate = new SysParSelectButtonDelegate(mpComponent, mpParameterTableView);
    mpParameterTableView->setItemDelegateForColumn(ParametersTableModel::SetSysPar, pPushButtonDelegate);
    mpParameterTableView->setEditTriggers(QAbstractItemView::AllEditTriggers);
    mpMainLayout->addWidget(mpParameterTableView, mpMainLayout->rowCount(), 0, 1, 2);
    //------------------------------------------------------------------------------------------------------------------------------

    // Add PortVariable settings table
    //------------------------------------------------------------------------------------------------------------------------------
    mpPortVariablesTableModel = new PortVariablesTableModel(mpComponent, this);
    mpPortVariableTableView = new QTableView(this);
    mpPortVariableTableView->setModel(mpPortVariablesTableModel);
    mpPortVariableTableView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    SysParSelectButtonDelegate *pPushButtonDelegate2 = new SysParSelectButtonDelegate(mpComponent, mpPortVariableTableView);
    mpPortVariableTableView->setItemDelegateForColumn(ParametersTableModel::SetSysPar, pPushButtonDelegate2);
    mpPortVariableTableView->setEditTriggers(QAbstractItemView::AllEditTriggers);
    mpMainLayout->addWidget(mpPortVariableTableView, mpMainLayout->rowCount(), 0, 1, 2);

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
bool ComponentPropertiesDialog2::interpretedAsStartValue(QString &parameterDescription)
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
void ComponentPropertiesDialog2::createEditStuff()
{


    //mpNameEdit = new QLineEdit(mpComponent->getName(), this);

    QFont fontH1;
    fontH1.setBold(true);

//    QFont fontH2;
//    fontH2.setBold(true);
//    fontH2.setItalic(true);


//    QLabel *pParameterLabel = new QLabel("Parameters", this);
//    pParameterLabel->setFont(fontH1);
//    QLabel *pStartValueLabel = new QLabel("Start Values", this);
//    pStartValueLabel->setFont(fontH1);

//    QGridLayout *parameterLayout = new QGridLayout();
//    QGridLayout *startValueLayout = new QGridLayout();

//    //QVector<QString> qParameterNames, qParameterValues, qDescriptions, qUnits, qTypes;
//    //mpGUIComponent->getParameters(qParameterNames, qParameterValues, qDescriptions, qUnits, qTypes);

//    QVector<CoreParameterData> paramDataVector;
//    mpComponent->getParameters(paramDataVector);

//    size_t nParam=0;
//    size_t nStV=0;
//    for(int i=0; i<paramDataVector.size(); ++i)
//    {
//        if(interpretedAsStartValue(paramDataVector[i].mDescription))
//        {
//            //QString unit = gConfig.getDefaultUnit(qParameterNames[i].section("::", 1, 1));
//            paramDataVector[i].mUnit.prepend("[").append("]");
//            mvStartValueLayout.push_back(new ParameterSettingsLayout(paramDataVector[i],
//                                                                     mpComponent));
//            startValueLayout->addLayout(mvStartValueLayout.back(), nParam, 0);
//            ++nParam;
//        }
//        else
//        {
//            mvParameterLayout.push_back(new ParameterSettingsLayout(paramDataVector[i],
//                                                                    mpComponent));
//            parameterLayout->addLayout(mvParameterLayout.back(), nStV, 0);
//            ++nStV;
//        }
//    }

//    //Adjust sizes of labels, to make sure that all text is visible and that the spacing is not too big between them
//    int descriptionSize=30;
//    int nameSize = 10;
//    //Paramters
//    for(int i=0; i<mvParameterLayout.size(); ++i)
//    {
//        descriptionSize = std::max(descriptionSize, mvParameterLayout.at(i)->mDescriptionLabel.width());
//        nameSize = std::max(nameSize, mvParameterLayout.at(i)->mNameLabel.width());
//    }
//    //Start values
//    for(int i=0; i<mvStartValueLayout.size(); ++i)
//    {
//        descriptionSize = std::max(descriptionSize, mvStartValueLayout.at(i)->mDescriptionLabel.width());
//        nameSize = std::max(nameSize, mvStartValueLayout.at(i)->mNameLabel.width());
//    }
//    //Paramters
//    for(int i=0; i<mvParameterLayout.size(); ++i)
//    {
//        mvParameterLayout.at(i)->mDescriptionLabel.setFixedWidth(descriptionSize+10);   //Offset of 10 as extra margin
//        mvParameterLayout.at(i)->mNameLabel.setFixedWidth(nameSize+10);
//    }
//    //Start values
//    for(int i=0; i<mvStartValueLayout.size(); ++i)
//    {
//        mvStartValueLayout.at(i)->mDescriptionLabel.setFixedWidth(descriptionSize+10);   //Offset of 10 as extra margin
//        mvStartValueLayout.at(i)->mNameLabel.setFixedWidth(nameSize+10);
//    }

//    //qDebug() << "after parnames";







//    //mainLayout->setSizeConstraint(QLayout::SetFixedSize);
//    int lr = 0; //Layout row


//    ++lr;

//    mpMainLayout->addLayout(pNameLayout, lr, 0);
//    mpMainLayout->addWidget(mpButtonBox, lr, 1);

//    ++lr;

//    if(!(mvParameterLayout.empty()))
//    {
//        mainLayout->addWidget(pParameterLabel, lr, 0, 1, 2);
//        ++lr;
//        mainLayout->addLayout(parameterLayout, lr, 0, 1, 2);
//        ++lr;
//    }
//    else
//    {
//        pParameterLabel->hide();
//    }
//    if(!(mvStartValueLayout.isEmpty()))
//    {
//        mainLayout->addWidget(pStartValueLabel,lr, 0, 1, 2);
//        ++lr;
//        mainLayout->addLayout(startValueLayout, lr, 0, 1, 2);
//    }
//    else
//    {
//        pStartValueLabel->hide();
//    }

//    QWidget *pPrimaryWidget = new QWidget(this);
//    pPrimaryWidget->setLayout(mpMainLayout);
//    pPrimaryWidget->setPalette(gConfig.getPalette());

//    QScrollArea *pScrollArea = new QScrollArea(this);
//    pScrollArea->setWidget(pPrimaryWidget);
//    pScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

//    QGridLayout *pPrimaryLayout = new QGridLayout(this);
//    pPrimaryLayout->addWidget(pScrollArea);
//    setLayout(pPrimaryLayout);

//    pPrimaryWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
//    mainLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
//    pPrimaryLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
//    int maxHeight = qApp->desktop()->screenGeometry().height()-100;
//    pScrollArea->setFixedHeight(std::min(pPrimaryWidget->height()+3, maxHeight));
//    if(pScrollArea->minimumHeight() == maxHeight)
//    {
//        pScrollArea->setMinimumWidth(pPrimaryWidget->width()+19);
//    }
//    else
//    {
//        pScrollArea->setMinimumWidth(pPrimaryWidget->width()+3);
//    }
//    pScrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
//    pScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}


//! @brief Reads the values from the dialog and writes them into the core component
void ComponentPropertiesDialog2::okPressed()
{
    setName();
    setParametersAndStartValues();
    close();
}


void ComponentPropertiesDialog2::editPortPos()
{
    //! @todo who owns the dialog, is it ever removed?
    MovePortsDialog *dialog = new MovePortsDialog(mpComponent->getAppearanceData(), mpComponent->getParentContainerObject()->getGfxType());
    connect(dialog, SIGNAL(finished()), mpComponent, SLOT(refreshExternalPortsAppearanceAndPosition()), Qt::UniqueConnection);
}


//! @brief Sets the parameters and start values in the core component. Read the values from the dialog and write them into the core component.
//! @see setParametersAndStartValues(QVector<ParameterLayout *> vParLayout)
void ComponentPropertiesDialog2::setParametersAndStartValues()
{
//    if(setParameterValues(mvParameterLayout) && setParameterValues(mvStartValueLayout))
//    {
//        qDebug() << "Parameters and start values updated.";
//        this->close();
//    }
}

void ComponentPropertiesDialog2::setName()
{
    mpComponent->getParentContainerObject()->renameModelObject(mpComponent->getName(), mpNameEdit->text());
}


ParametersTableModel::ParametersTableModel(Component *pComponent, QObject *pParent) :
    QAbstractTableModel(pParent)
{
    mHeaders.resize(NumColumns);
    mHeaders[Name] = "Name";
    mHeaders[Unit] = "Unit";
    mHeaders[Type] = "Type";
    mHeaders[Alias] = "Alias";
    mHeaders[Value] = "Value";
    mHeaders[ResetValue] = "ResetValue";
    mHeaders[SetSysPar] = "SysPar";

    mpComponent = pComponent;
    pComponent->getParameters(mParameters);

    //Filter out start values
    int ctr=0;
    while (ctr<mParameters.size())
    {
        if (mParameters[ctr].mName.contains("::"))
        {
            mParameters.remove(ctr);
        }
        else
        {
            ++ctr;
        }
    }

}

int ParametersTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return mParameters.size();

}

int ParametersTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return NumColumns;
}

QVariant ParametersTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal)
    {
        if (section < mHeaders.size())
        {
            return mHeaders[section];
        }
    }
    else
    {
        return QString("%1").arg(section);
    }

    return QVariant();
}

QVariant ParametersTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() >= mParameters.size())
        return QVariant();

    if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
        switch(index.column())
        {
        case Name:
            return mParameters[index.row()].mName;
            break;
        case Unit:
            return mParameters[index.row()].mUnit;
            break;
        case Type:
            return mParameters[index.row()].mType;
            break;
        case Alias:
            return "Not supported";
        case Value:
            return mParameters[index.row()].mValue;
            break;
        }
    }

    if (role == Qt::DecorationRole)
    {
        switch(index.column())
        {
        case ResetValue:
            return QIcon(QString(ICONPATH) + "Hopsan-ResetDefault.png");
            break;
        case SetSysPar:
            return QIcon(QString(ICONPATH) + "Hopsan-SystemParameter.png");
            break;
        }
    }

    return QVariant();
}

bool ParametersTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    CoreParameterData data;
    bool isOk=false;
    if (index.isValid() && role == Qt::EditRole)
    {
        switch(index.column())
        {
        case Alias:
            break;
        case Value:
        case SetSysPar:
            // Cleanup value and check for errors
            QString val = value.toString();
            isOk = cleanAndVerifyParameterValue(val, mParameters[index.row()].mType);
            if (isOk)
            {
                // Try to set the parameter
                isOk = mpComponent->setParameterValue(mParameters[index.row()].mName, val);
                if (isOk)
                {
                    // Write parameter into mPArameters, to store new values localy also emit signal
                    mpComponent->getParameter(mParameters[index.row()].mName, mParameters[index.row()]);
                    emit dataChanged(index, index);
                }
            }
            break;
        }
    }
    return isOk;
}

Qt::ItemFlags ParametersTableModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    switch(index.column())
    {
    case Alias:
    case Value:
    case SetSysPar:
        return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
        break;
    default:
        return QAbstractItemModel::flags(index);
    }
}

bool ParametersTableModel::cleanAndVerifyParameterValue(QString &rValue, const QString paramType)
{
    QStringList sysParamNames = mpComponent->getParentContainerObject()->getParameterNames();
    QString error;

    bool isok = verifyParameterValue(rValue, paramType, sysParamNames, error);
    if(!isok)
    {
        QMessageBox::critical(0, "Error", error.append(" Resetting parameter value!"));
    }

    return isok;
}

SysParSelectButtonDelegate::SysParSelectButtonDelegate(ModelObject *pModelObject, QObject *pParent) :
    QStyledItemDelegate(pParent), mpModelObject(pModelObject)
{
}

QWidget *SysParSelectButtonDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);

//    QMenu *menu = new QMenu();
//    QMap<QAction*, QString> actionParamMap;

    QVector<CoreParameterData> paramDataVector;
    mpModelObject->getParentContainerObject()->getParameters(paramDataVector);

    QComboBox *pComboBox = new QComboBox(parent);
    pComboBox->addItem("Create New");
    pComboBox->insertSeparator(1);
    for (int i=0; i<paramDataVector.size(); ++i)
    {
        pComboBox->addItem(paramDataVector[i].mName+" = "+paramDataVector[i].mValue);
    }


//    for (int i=0; i<2; ++i)
//    {
//        //QAction *tempAction = menu.addAction(paramDataVector[i].mName+" = "+paramDataVector[i].mValue);
//        QString num;
//        num.setNum(i);
//        QAction *tempAction = menu->addAction("apa"+num);
//        tempAction->setIconVisibleInMenu(false);
//        actionParamMap.insert(tempAction, "apa"+num);
//    }

//    QCursor cursor;
//    QAction *selectedAction = menu->exec(cursor.pos());

//    QString parNameString = actionParamMap.value(selectedAction);
//    if(!parNameString.isEmpty())
//    {
//        mValueLineEdit.setText(parNameString);
//    }

    //ParameterTypeComboBox *editor = new ParameterTypeComboBox(parent);
    return pComboBox;
}

void SysParSelectButtonDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QComboBox *pComboBox = static_cast<QComboBox*>(editor);
    if (pComboBox->currentIndex() > 1)
    {
        QString param = pComboBox->currentText().section("=",0,0); //Split string to remove = value
        model->setData(index, param, Qt::EditRole);
    }
}

PortVariablesTableModel::PortVariablesTableModel(Component *pComponent, QObject *pParent):
    QAbstractTableModel(pParent)
{
    mHeaders.resize(NumColumns);
    mHeaders[Name] = "Name";
    mHeaders[Unit] = "Unit";
    mHeaders[Type] = "Type";
    mHeaders[Alias] = "Alias";
    mHeaders[StartValue] = "StartValue";
    mHeaders[ResetValue] = "ResetValue";
    mHeaders[SetSysPar] = "SysPar";
    mHeaders[Scale] = "Scale";

    mpComponent = pComponent;
    mpComponent->getParameters(mParameters);

    //Filter out ordinary parameters
    //! @todo maybe not use parameters for this, as alias will not be part of the startvalue
    int ctr=0;
    while (ctr<mParameters.size())
    {
        if (mParameters[ctr].mName.contains("::"))
        {
            ++ctr;
        }
        else
        {
            mParameters.remove(ctr);
        }
    }

    QVector<QPair<QString, QString> > aliases = pComponent->getVariableAliasList();
    // Now feetch alias for the variables
    for (int i=0; i<mParameters.size(); ++i)
    {
        for (int j=0; j<aliases.size(); ++j)
        {
            QString alias = aliases[j].first;
            QString fullName = aliases[j].second;

            QStringList splitname = mParameters[i].mName.split("::");

            //! @todo VERY unsafe check, and complete MADNESS regarding alias handling for variables (but I am in a hurry) /Peter
            if (fullName.contains(splitname[0]+QString('#')+splitname[1]))
            {
                mParameters[i].mAlias = alias;
            }
        }
    }
}

int PortVariablesTableModel::rowCount(const QModelIndex &parent) const
{
    return mParameters.size();
}

int PortVariablesTableModel::columnCount(const QModelIndex &parent) const
{
    return mHeaders.size();
}

QVariant PortVariablesTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal)
    {
        if (section < mHeaders.size())
        {
            return mHeaders[section];
        }
    }
    else
    {
        return QString("%1").arg(section);
    }

    return QVariant();
}

QVariant PortVariablesTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() >= mParameters.size())
        return QVariant();

    if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
        switch(index.column())
        {
        case Name:
            return mParameters[index.row()].mName;
            break;
        case Unit:
            return mParameters[index.row()].mUnit;
            break;
        case Type:
            return mParameters[index.row()].mType;
            break;
        case Alias:
            return mParameters[index.row()].mAlias;
            break;
        case StartValue:
            return mParameters[index.row()].mValue;
            break;
        case Scale:
            return 1;
            break;
        }
    }

    if (role == Qt::DecorationRole)
    {
        switch(index.column())
        {
        case ResetValue:
            return QIcon(QString(ICONPATH) + "Hopsan-ResetDefault.png");
            break;
        case SetSysPar:
            return QIcon(QString(ICONPATH) + "Hopsan-SystemParameter.png");
            break;
        }
    }

    return QVariant();
}

bool PortVariablesTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    CoreParameterData data;
    bool isOk=false;
    if (index.isValid() && role == Qt::EditRole)
    {
        switch(index.column())
        {
        case Alias: {
            QString alias = value.toString();
            QString port = mParameters[index.row()].mName.section("::",0,0);
            QString var = mParameters[index.row()].mName.section("::",1,1);
            mpComponent->getParentContainerObject()->setVariableAlias(mpComponent->getName(), port, var, alias);
            mParameters[index.row()].mAlias = alias; //!< @todo this is a quick hack
            emit dataChanged(index, index);
            isOk=true;
            break;
        }
        case StartValue:
            // Cleanup value and check for errors
            QString val = value.toString();
            isOk = cleanAndVerifyParameterValue(val, mParameters[index.row()].mType);
            if (isOk)
            {
                // Try to set the parameter
                isOk = mpComponent->setParameterValue(mParameters[index.row()].mName, val);
                if (isOk)
                {
                    // Write parameter into mParameters, to store new values localy also emit signal
                    mpComponent->getParameter(mParameters[index.row()].mName, mParameters[index.row()]);
                    emit dataChanged(index, index);
                }
            }
            break;
        }
    }
    return isOk;
}

Qt::ItemFlags PortVariablesTableModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    switch(index.column())
    {
    case Alias:
    case StartValue:
    case SetSysPar:
        return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
        break;
    default:
        return QAbstractItemModel::flags(index);
    }
}

bool PortVariablesTableModel::cleanAndVerifyParameterValue(QString &rValue, const QString paramType)
{
    QStringList sysParamNames = mpComponent->getParentContainerObject()->getParameterNames();
    QString error;

    bool isok = verifyParameterValue(rValue, paramType, sysParamNames, error);
    if(!isok)
    {
        QMessageBox::critical(0, "Error", error.append(" Resetting parameter value!"));
    }

    return isok;
}

//void SysParSelectButtonDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
//{

//}

//void SysParSelectButtonDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
//{

//}

//void SysParSelectButtonDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
//{

//}
