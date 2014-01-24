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
//! @file   PlotCurveControlBox.cpp
//! @author Flumes
//! @date   2014
//!
//! @brief Contains a class for plot curve control box
//!
//$Id: ModelHandler.cpp 5551 2013-06-20 08:54:16Z petno25 $

#include <QDropEvent>

#include "PlotCurveControlBox.h"
#include "PlotArea.h"
#include "PlotCurve.h"
#include "global.h"
#include "Configuration.h"



CustomXDataDropEdit::CustomXDataDropEdit(QWidget *pParent)
    : QLineEdit(pParent)
{
    //Nothing
}

void CustomXDataDropEdit::dropEvent(QDropEvent *e)
{
    QLineEdit::dropEvent(e);
    QString mimeText = e->mimeData()->text();
    if(mimeText.startsWith("HOPSANPLOTDATA:"))
    {
        //! @todo what about model name, if draging from other model (it should not work but sowhere we need to block and warn) (maybe not here)
        QStringList fields = mimeText.split(":");
        if (fields.size() > 2)
        {
            // We do not want to include gen here, as the curve should decide for it self what gen to use
            emit newXData(fields[1]);
        }
    }
    else
    {
        emit newXData(mimeText);
    }
}


//! @brief Constructor for plot info box
//! @param pParentPlotCurve pointer to parent plot curve
//! @param pParent Pointer to parent widget
PlotCurveControlBox::PlotCurveControlBox(PlotCurve *pPlotCurve, PlotArea *pParentArea)
    : QWidget(pParentArea)
{
    mpPlotArea = pParentArea;
    mpPlotCurve = pPlotCurve;
    connect(mpPlotCurve, SIGNAL(curveInfoUpdated()), this, SLOT(updateInfo()));
    connect(mpPlotCurve, SIGNAL(colorChanged(QColor)), this, SLOT(updateColor(QColor)));
    connect(mpPlotCurve, SIGNAL(markedActive(bool)), this, SLOT(markActive(bool)));

    mpColorBlob = new QToolButton(this);
    mpColorBlob->setFixedSize(20,20);
    mpColorBlob->setCheckable(true);
    mpColorBlob->setChecked(false);
    updateColor(mpPlotCurve->getLineColor());

    mpTitle = new QLabel(this);
    mpTitle->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    mpTitle->setAlignment(Qt::AlignHCenter);
    refreshTitle();

    mpCustomXDataDrop = new CustomXDataDropEdit(this);
    mpCustomXDataDrop->setToolTip("Drag and Drop here to set Custom XData Vector");
    mpResetTimeButton = new QToolButton(this);
    mpResetTimeButton->setToolTip("Reset Time Vector");
    mpResetTimeButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-ResetTimeVector.png"));
    mpResetTimeButton->setEnabled(false);

    mpGenerationSpinBox = new QSpinBox(this);
    mpGenerationSpinBox->setToolTip("Change generation");

    mpGenerationLabel = new QLabel(this);
    mpGenerationLabel->setToolTip("Available generations");
    QFont tempFont = mpGenerationLabel->font();
    tempFont.setBold(true);
    mpGenerationLabel->setFont(tempFont);

    mpSourceLable = new QLabel(this);
    mpSourceLable->setFont(tempFont);
    mpSourceLable->setText("U");
    mpSourceLable->setToolTip(variableSourceTypeAsShortString(UndefinedVariableSourceType));

    QCheckBox *pAutoUpdateCheckBox = new QCheckBox("Auto Update");
    pAutoUpdateCheckBox->setChecked(true);

    QToolButton *pColorButton = new QToolButton(this);
    pColorButton->setToolTip("Select Line Color");
    pColorButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-LineColor.png"));

    QToolButton *pFrequencyAnalysisButton = new QToolButton(this);
    pFrequencyAnalysisButton->setToolTip("Frequency Analysis");
    pFrequencyAnalysisButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-FrequencyAnalysis.png"));

    QToolButton *pScaleButton = new QToolButton(this);
    pScaleButton->setToolTip("Scale Curve");
    pScaleButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-PlotCurveScale.png"));

    QLabel *pSizeLabel = new QLabel(tr("Line Width: "));
    pSizeLabel->setAcceptDrops(false);
    QSpinBox *pSizeSpinBox = new QSpinBox(this);
    pSizeSpinBox->setAcceptDrops(false);
    pSizeSpinBox->setRange(1,10);
    pSizeSpinBox->setSingleStep(1);
    pSizeSpinBox->setValue(2);
    pSizeSpinBox->setSuffix(" pt");

    // New Combo Box for Line Style
    QComboBox *pLineStyleCombo = new QComboBox;
    pLineStyleCombo->addItem(tr("Solid Line"));
    pLineStyleCombo->addItem(tr("Dash Line"));
    pLineStyleCombo->addItem(tr("Dot Line"));
    pLineStyleCombo->addItem(tr("Dash Dot Line"));
    pLineStyleCombo->addItem(tr("Dash Dot Dot Line"));
    pLineStyleCombo->addItem(tr("No Curve")); //CustomDashLine

    // New Combo Box for Symbol Style
    QComboBox *pLineSymbol = new QComboBox;
    pLineSymbol->addItem(tr("None"));
    pLineSymbol->addItem(tr("Cross"));
    pLineSymbol->addItem(tr("Ellipse"));
    pLineSymbol->addItem(tr("XCross"));
    pLineSymbol->addItem(tr("Triangle"));
    pLineSymbol->addItem(tr("Rectangle"));
    pLineSymbol->addItem(tr("Diamond"));
    pLineSymbol->addItem(tr("Down Triangle"));
    pLineSymbol->addItem(tr("Up Triangle"));
    pLineSymbol->addItem(tr("Right Triangle"));
    pLineSymbol->addItem(tr("Hexagon"));
    pLineSymbol->addItem(tr("Horizontal Line"));
    pLineSymbol->addItem(tr("Vertical Line"));
    pLineSymbol->addItem(tr("Star 1"));
    pLineSymbol->addItem(tr("Star 2"));
    //mpLineSymbol->addItem(tr("Dots"));


    QToolButton *pCloseButton = new QToolButton(this);
    pCloseButton->setToolTip("Remove Curve");
    pCloseButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-Discard.png"));

    QLabel *pDummy = new QLabel("", this);

    QHBoxLayout *pInfoBoxLayout = new QHBoxLayout(this);
    pInfoBoxLayout->addWidget(mpColorBlob);
    pInfoBoxLayout->addWidget(mpTitle);
    pInfoBoxLayout->addWidget(mpCustomXDataDrop);
    pInfoBoxLayout->addWidget(mpResetTimeButton);
    pInfoBoxLayout->addWidget(mpGenerationSpinBox);
    pInfoBoxLayout->addWidget(mpGenerationLabel);
    pInfoBoxLayout->addWidget(mpSourceLable);
    pInfoBoxLayout->addWidget(pAutoUpdateCheckBox);
    pInfoBoxLayout->addWidget(pFrequencyAnalysisButton);
    pInfoBoxLayout->addWidget(pScaleButton);
    pInfoBoxLayout->addWidget(pSizeSpinBox);
    pInfoBoxLayout->addWidget(pColorButton);
    pInfoBoxLayout->addWidget(pLineStyleCombo);
    pInfoBoxLayout->addWidget(pLineSymbol);
    pInfoBoxLayout->addWidget(pCloseButton);
    pInfoBoxLayout->addWidget(pDummy); // This one must be here to prevent colorblob from having a very small clickable area, (really strange)

    setLayout(pInfoBoxLayout);

    connect(mpColorBlob,               SIGNAL(clicked(bool)),       this,               SLOT(activateCurve(bool)));
    connect(mpCustomXDataDrop,         SIGNAL(newXData(QString)),   this,               SLOT(setXData(QString)));
    connect(mpResetTimeButton,         SIGNAL(clicked()),           this,               SLOT(resetTimeVector()));
    connect(mpGenerationSpinBox,       SIGNAL(valueChanged(int)),   this,               SLOT(setGeneration(int)));
    connect(pCloseButton,              SIGNAL(clicked()),           this,               SLOT(removeTheCurve()));
    connect(pAutoUpdateCheckBox,       SIGNAL(toggled(bool)),       mpPlotCurve,  SLOT(setAutoUpdate(bool)));
    connect(pFrequencyAnalysisButton,  SIGNAL(clicked(bool)),       mpPlotCurve,  SLOT(openFrequencyAnalysisDialog())); //!< @todo this should probably be in the plot area, and signaled directly with curve
    connect(pColorButton,              SIGNAL(clicked()),           mpPlotCurve,  SLOT(setLineColor()));
    connect(pScaleButton,              SIGNAL(clicked()),           mpPlotCurve,  SLOT(openScaleDialog()));
    connect(pSizeSpinBox,    SIGNAL(valueChanged(int)),             mpPlotCurve,  SLOT(setLineWidth(int)));
    connect(pLineStyleCombo, SIGNAL(currentIndexChanged(QString)),  mpPlotCurve,  SLOT(setLineStyle(QString)));
    connect(pLineSymbol,     SIGNAL(currentIndexChanged(QString)),  mpPlotCurve,  SLOT(setLineSymbol(QString)));

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setPalette(gpConfig->getPalette());

    if(mpPlotCurve->getCurveType() != PortVariableType)
    {
        pAutoUpdateCheckBox->setDisabled(true);
        mpGenerationSpinBox->setDisabled(true);
        pFrequencyAnalysisButton->setDisabled(true);
    }
    updateInfo();
}

PlotCurve *PlotCurveControlBox::getCurve()
{
    return mpPlotCurve;
}

void PlotCurveControlBox::updateColor(const QColor color)
{
    QString buttonStyle;

    // Update color blob in plot info box
    buttonStyle.append(QString("QToolButton                 { border: 1px solid gray;           border-style: outset;   border-radius: 5px;     padding: 2px;   background-color: rgb(%1,%2,%3)}").arg(color.red()).arg(color.green()).arg(color.blue()));
    buttonStyle.append(QString("QToolButton:unchecked		{ border: 1px solid gray;           border-style: outset;	border-radius: 5px;    	padding: 0px;   background-color: rgb(%1,%2,%3)}").arg(color.red()).arg(color.green()).arg(color.blue()));
    buttonStyle.append(QString("QToolButton:checked         { border: 1px solid gray;           border-style: inset;    border-radius: 5px;    	padding: 1px;   background-color: rgb(%1,%2,%3)}").arg(color.red()).arg(color.green()).arg(color.blue()));
    buttonStyle.append(QString("QToolButton:pressed 		{ border: 2px solid rgb(70,70,150); border-style: outset;   border-radius: 5px;     padding: 0px;   background-color: rgb(%1,%2,%3)}").arg(color.red()).arg(color.green()).arg(color.blue()));
    buttonStyle.append(QString("QToolButton:hover           { border: 2px solid gray;           border-style: outset;   border-radius: 10px;    padding: 0px;   background-color: rgb(%1,%2,%3)}").arg(color.red()).arg(color.green()).arg(color.blue()));
    buttonStyle.append(QString("QToolButton:hover:unchecked { border: 1px solid gray;           border-style: outset;   border-radius: 5px;     padding: 2px;   background-color: rgb(%1,%2,%3)}").arg(color.red()).arg(color.green()).arg(color.blue()));
    buttonStyle.append(QString("QToolButton:hover:checked   { border: 2px solid rgb(70,70,150); border-style: outset;   border-radius: 5px;     padding: 0px;   background-color: rgb(%1,%2,%3)}").arg(color.red()).arg(color.green()).arg(color.blue()));
    buttonStyle.append(QString("QToolButton:hover:pressed   { border: 2px solid rgb(70,70,150); border-style: outset;   border-radius: 5px;     padding: 0px;   background-color: rgb(%1,%2,%3)}").arg(color.red()).arg(color.green()).arg(color.blue()));

    //! @todo need to fix the syle so that it is shown which is activated

    mpColorBlob->setStyleSheet(buttonStyle);
}

//! @brief Updates buttons and text in plot info box to correct values
void PlotCurveControlBox::updateInfo()
{
    // Enable/diable generation buttons
    const int lowGen = mpPlotCurve->getDataVariable()->getLowestGeneration();
    const int highGen = mpPlotCurve->getDataVariable()->getHighestGeneration();
    const int gen = mpPlotCurve->getGeneration();
    const int nGen = mpPlotCurve->getDataVariable()->getNumGenerations();
    disconnect(mpGenerationSpinBox,         SIGNAL(valueChanged(int)),   this,  SLOT(setGeneration(int))); //Need to temporarily disconnect to avoid loop
    mpGenerationSpinBox->setRange(lowGen+1, highGen+1);
    mpGenerationSpinBox->setValue(gen+1);
    connect(mpGenerationSpinBox,            SIGNAL(valueChanged(int)),   this,  SLOT(setGeneration(int)));
    mpGenerationSpinBox->setEnabled(nGen > 1);

    // Set generation number strings
    //! @todo this will show strange when we have deleted old generations, maybe we should reassign all generations when we delete old data (costly)
    mpGenerationLabel->setText(QString("[%1,%2]").arg(lowGen+1).arg(highGen+1));

    // Set source lable
    mpSourceLable->setText(variableSourceTypeAsShortString(mpPlotCurve->getDataSource()));
    switch(mpPlotCurve->getDataSource())
    {
    case ModelVariableType:
        mpSourceLable->setToolTip(variableSourceTypeAsString(mpPlotCurve->getDataSource())+": "+mpPlotCurve->getDataModelPath());
        break;
    case ImportedVariableType:
        mpSourceLable->setToolTip(variableSourceTypeAsString(mpPlotCurve->getDataSource())+": "+mpPlotCurve->getDataVariable()->getImportedFileName());
        break;
    default:
        mpSourceLable->setToolTip(variableSourceTypeAsString(mpPlotCurve->getDataSource()));
    }

    // Update curve name
    refreshTitle();

    // Update Xdata
    if (mpPlotCurve->hasCustomXVariable())
    {
        mpCustomXDataDrop->setText(mpPlotCurve->getSharedCustomXVariable()->getFullVariableName());
        if (mpPlotCurve->getSharedTimeOrFrequencyVariable())
        {
            mpResetTimeButton->setEnabled(true);
        }
    }
    else
    {
        mpCustomXDataDrop->setText("");
        mpResetTimeButton->setEnabled(false);
    }
}

void PlotCurveControlBox::refreshTitle()
{
    mpTitle->setText(mpPlotCurve->getCurveName() + " ["+mpPlotCurve->getCurrentUnit()+"]");
}

void PlotCurveControlBox::markActive(bool active)
{
    //! @todo this is not visible (marking doesnt show)
    mpColorBlob->setChecked(active);
    if (active)
    {
        setAutoFillBackground(true);
        setPalette(gpConfig->getPalette());
    }
    else
    {
        setAutoFillBackground(false);
    }
}

void PlotCurveControlBox::activateCurve(bool active)
{
    if(active)
    {
        mpPlotArea->setActivePlotCurve(mpPlotCurve);
    }
    else
    {
        mpPlotArea->setActivePlotCurve(0);
    }
}

void PlotCurveControlBox::setXData(QString fullName)
{
    mpPlotCurve->setCustomXData(fullName);
}

void PlotCurveControlBox::resetTimeVector()
{
    mpPlotCurve->setCustomXData(0);
}

void PlotCurveControlBox::setGeneration(const int gen)
{
    // Since info box begins counting at 1 we need to subtract one, but we do not want underflow as that would set latest generation (-1)
    if (!mpPlotCurve->setGeneration(qMax(gen-1,0)))
    {
        mpGenerationSpinBox->setPrefix("NA<");
        mpGenerationSpinBox->setSuffix(">");
    }
    else
    {
        mpGenerationSpinBox->setPrefix("");
        mpGenerationSpinBox->setSuffix("");
    }
}

void PlotCurveControlBox::removeTheCurve()
{
    emit removeCurve(mpPlotCurve);
}
