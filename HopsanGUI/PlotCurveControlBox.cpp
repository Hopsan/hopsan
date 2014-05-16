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
//$Id$

#include <QDropEvent>
#include <QMimeData>
#include <QHBoxLayout>

#include "PlotCurveControlBox.h"
#include "PlotArea.h"
#include "PlotCurve.h"
#include "global.h"
#include "Configuration.h"


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

    mpCustomXDataDrop = new CustomXDataControl(this);

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

    mpAutoUpdateCheckBox = new QCheckBox("Auto Update");
    mpAutoUpdateCheckBox->setChecked(mpPlotCurve->isAutoUpdating());

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
    pInfoBoxLayout->setContentsMargins(0,0,0,0);
    pInfoBoxLayout->addWidget(mpColorBlob);
    pInfoBoxLayout->addWidget(mpTitle);
    pInfoBoxLayout->addWidget(mpCustomXDataDrop);
    pInfoBoxLayout->addWidget(mpGenerationSpinBox);
    pInfoBoxLayout->addWidget(mpGenerationLabel);
    pInfoBoxLayout->addWidget(mpSourceLable);
    pInfoBoxLayout->addWidget(mpAutoUpdateCheckBox);
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
    connect(mpCustomXDataDrop,         SIGNAL(resetXData()),        this,               SLOT(resetXData()));
    connect(mpGenerationSpinBox,       SIGNAL(valueChanged(int)),   this,               SLOT(setGeneration(int)));
    connect(pCloseButton,              SIGNAL(clicked()),           this,               SLOT(removeTheCurve()));
    connect(mpAutoUpdateCheckBox,      SIGNAL(toggled(bool)),       mpPlotCurve,  SLOT(setAutoUpdate(bool)));
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
        mpAutoUpdateCheckBox->setDisabled(true);
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
    SharedVectorVariableContainerT pVarContainer = mpPlotCurve->getSharedVectorVariableContainer();
    int gen = mpPlotCurve->getGeneration();
    int lowGen, highGen, nGen;
    if (pVarContainer)
    {
        lowGen = pVarContainer->getLowestGeneration();
        highGen = pVarContainer->getHighestGeneration();
        nGen = pVarContainer->getNumGenerations();
    }
    else
    {
        lowGen = highGen = gen;
        nGen = 1;
    }
    mpGenerationSpinBox->blockSignals(true);    // Need to temporarily disconnect to avoid loop
    mpGenerationSpinBox->setRange(lowGen+1, highGen+1);
    mpGenerationSpinBox->setValue(gen+1);
    mpGenerationSpinBox->blockSignals(false);   // Unblock
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
        mpSourceLable->setToolTip(variableSourceTypeAsString(mpPlotCurve->getDataSource())+": "+mpPlotCurve->getSharedVectorVariable()->getImportedFileName());
        break;
    default:
        mpSourceLable->setToolTip(variableSourceTypeAsString(mpPlotCurve->getDataSource()));
    }

    // Update curve name
    refreshTitle();

    // Update Xdata
    if (mpPlotCurve->hasCustomXData())
    {
        mpCustomXDataDrop->updateInfo(mpPlotCurve->getSharedCustomXVariable().data());
    }
    else
    {
        mpCustomXDataDrop->updateInfo(0);
    }

    // Update auto refresh
    mpAutoUpdateCheckBox->blockSignals(true);
    mpAutoUpdateCheckBox->setChecked(mpPlotCurve->isAutoUpdating());
    mpAutoUpdateCheckBox->blockSignals(false);
}

void PlotCurveControlBox::refreshTitle()
{
    mpTitle->setText(mpPlotCurve->getCurveName() + " ["+mpPlotCurve->getCurrentUnit()+"]");
    mpTitle->setToolTip(mpPlotCurve->getSharedVectorVariable()->getFullVariableName());
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

void PlotCurveControlBox::resetXData()
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


CustomXDataControl::CustomXDataControl(QWidget *pParent) :
    QWidget(pParent)
{
    // Build widget
    QHBoxLayout *pLayout = new QHBoxLayout(this);

    mpXLabel = new QLabel("x: ", this);
    mpXLabel->setEnabled(false);
    mpNameLabel = new QLabel(this);
    mpNameLabel->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    mpResetXDataButton = new QToolButton(this);
    mpResetXDataButton->setToolTip("Reset XData");
    mpResetXDataButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-ResetTimeVector.png"));
    updateInfo(0);

    pLayout->addWidget(mpXLabel);
    pLayout->addWidget(mpNameLabel);
    pLayout->addWidget(mpResetXDataButton);
    pLayout->setSpacing(0);

    // Connect relay signal
    connect(mpResetXDataButton, SIGNAL(clicked()), this, SIGNAL(resetXData()));

    setAcceptDrops(true);
}

void CustomXDataControl::updateInfo(const VectorVariable *pData)
{
    if (pData)
    {
        const QString nameWithGen = QString("%1  (%2)").arg(pData->getSmartName()).arg(pData->getGeneration()+1);
        mpNameLabel->setText(nameWithGen);
        mpNameLabel->setFixedWidth(48*4);
        mpNameLabel->setAlignment(Qt::AlignRight);
        mpNameLabel->setToolTip(QString("<nobr>Drag and Drop here to set Custom XData Vector</nobr><br><nobr><b>%1</b></nobr>").arg(nameWithGen));
        mpResetXDataButton->setEnabled(true);
        mpXLabel->setEnabled(true);
    }
    else
    {
        mpNameLabel->clear();
        mpNameLabel->setFixedWidth(48);
        mpNameLabel->setToolTip(QString("<nobr>Drag and Drop here to set Custom XData Vector</nobr>"));
        mpResetXDataButton->setEnabled(false);
        mpXLabel->setEnabled(false);
    }
}

void CustomXDataControl::dragEnterEvent(QDragEnterEvent *e)
{
    if (e->mimeData()->text().startsWith("HOPSANPLOTDATA:"))
    {
        e->acceptProposedAction();
    }
}

void CustomXDataControl::dropEvent(QDropEvent *e)
{
    QWidget::dropEvent(e);
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
