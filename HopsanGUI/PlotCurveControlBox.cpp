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
#include <QPalette>

#include "PlotCurveControlBox.h"
#include "PlotArea.h"
#include "PlotCurve.h"
#include "global.h"
#include "Configuration.h"
#include "LogDataHandler2.h"


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

    mpAutoUpdateCheckBox = new QCheckBox("Auto");
    mpAutoUpdateCheckBox->setToolTip("Auto update");
    mpAutoUpdateCheckBox->setChecked(mpPlotCurve->isAutoUpdating());

    mpInvertCurveCheckBox = new QCheckBox("Inv");
    mpInvertCurveCheckBox->setToolTip("Invert this plot curve (use model properties for persistence)");
    mpInvertCurveCheckBox->setChecked(mpPlotCurve->isInverted());

    QToolButton *pColorButton = new QToolButton(this);
    pColorButton->setToolTip("Select Line Color");
    pColorButton->setIcon(QIcon(QString(ICONPATH) + "svg/Hopsan-SelectColor.svg"));

    QToolButton *pFrequencyAnalysisButton = new QToolButton(this);
    pFrequencyAnalysisButton->setToolTip("Frequency Analysis");
    pFrequencyAnalysisButton->setIcon(QIcon(QString(ICONPATH) + "svg/Hopsan-FrequencyAnalysis.svg"));

    QToolButton *pScaleButton = new QToolButton(this);
    pScaleButton->setToolTip("Scale Curve");
    pScaleButton->setIcon(QIcon(QString(ICONPATH) + "svg/Hopsan-AxisScale.svg"));

    QLabel *pSizeLabel = new QLabel(tr("Line Width: "));
    pSizeLabel->setAcceptDrops(false);
    mpSizeSpinBox = new QSpinBox(this);
    mpSizeSpinBox->setAcceptDrops(false);
    mpSizeSpinBox->setRange(1,10);
    mpSizeSpinBox->setSingleStep(1);
    mpSizeSpinBox->setValue(2);
    mpSizeSpinBox->setSuffix(" pt");

    // New Combo Box for Line Style
    mpLineStyleCombo = new QComboBox;
    mpLineStyleCombo->addItem(tr("Solid Line"));
    mpLineStyleCombo->addItem(tr("Dash Line"));
    mpLineStyleCombo->addItem(tr("Dot Line"));
    mpLineStyleCombo->addItem(tr("Dash Dot Line"));
    mpLineStyleCombo->addItem(tr("Dash Dot Dot Line"));
    mpLineStyleCombo->addItem(tr("No Curve")); //CustomDashLine

    // New Combo Box for Symbol Style
    mpLineSymbolCombo = new QComboBox;
    for (const auto& symbol_name : PlotCurveStyle::symbol_names)
    {
        mpLineSymbolCombo->addItem(symbol_name);
    }

    QToolButton *pCloseButton = new QToolButton(this);
    pCloseButton->setToolTip("Remove Curve");
    pCloseButton->setIcon(QIcon(QString(ICONPATH) + "svg/Hopsan-Discard.svg"));

    QLabel *pDummy = new QLabel("", this);

    QHBoxLayout *pInfoBoxLayout = new QHBoxLayout(this);
    pInfoBoxLayout->setContentsMargins(0,0,0,0);
    pInfoBoxLayout->addWidget(pCloseButton);
    pInfoBoxLayout->addWidget(mpColorBlob);
    pInfoBoxLayout->addWidget(mpTitle);
    pInfoBoxLayout->addWidget(mpCustomXDataDrop);
    pInfoBoxLayout->addWidget(mpGenerationSpinBox);
    pInfoBoxLayout->addWidget(mpGenerationLabel);
    pInfoBoxLayout->addWidget(mpSourceLable);
    pInfoBoxLayout->addWidget(mpAutoUpdateCheckBox);
    pInfoBoxLayout->addWidget(mpInvertCurveCheckBox);
    pInfoBoxLayout->addWidget(pFrequencyAnalysisButton);
    pInfoBoxLayout->addWidget(pScaleButton);
    pInfoBoxLayout->addWidget(mpSizeSpinBox);
    pInfoBoxLayout->addWidget(pColorButton);
    pInfoBoxLayout->addWidget(mpLineStyleCombo);
    pInfoBoxLayout->addWidget(mpLineSymbolCombo);
    pInfoBoxLayout->addWidget(pDummy); // This one must be here to prevent colorblob from having a very small clickable area, (really strange)

    setLayout(pInfoBoxLayout);

    connect(mpColorBlob,               SIGNAL(clicked(bool)),       this,               SLOT(activateCurve(bool)));
    connect(mpCustomXDataDrop,         SIGNAL(newXData(QString)),   this,               SLOT(setXData(QString)));
    connect(mpCustomXDataDrop,         SIGNAL(resetXData()),        this,               SLOT(resetXData()));
    connect(mpGenerationSpinBox,       SIGNAL(valueChanged(int)),   this,               SLOT(setGeneration(int)));
    connect(pCloseButton,              SIGNAL(clicked()),           this,               SLOT(removeTheCurve()));
    connect(mpAutoUpdateCheckBox,      SIGNAL(toggled(bool)),       mpPlotCurve,  SLOT(setAutoUpdate(bool)));
    connect(mpInvertCurveCheckBox,     SIGNAL(toggled(bool)),       mpPlotCurve,  SLOT(setInvertPlot(bool)));
    connect(pFrequencyAnalysisButton,  SIGNAL(clicked(bool)),       mpPlotCurve,  SLOT(openFrequencyAnalysisDialog())); //!< @todo this should probably be in the plot area, and signaled directly with curve
    connect(pColorButton,              SIGNAL(clicked()),           mpPlotCurve,  SLOT(setLineColor()));
    connect(pScaleButton,              SIGNAL(clicked()),           mpPlotCurve,  SLOT(openScaleDialog()));
    connect(mpSizeSpinBox,    SIGNAL(valueChanged(int)),             mpPlotCurve,  SLOT(setLineWidth(int)));
    connect(mpLineStyleCombo, SIGNAL(currentIndexChanged(QString)),  mpPlotCurve,  SLOT(setLineStyle(QString)));
    connect(mpLineSymbolCombo,     SIGNAL(currentIndexChanged(QString)),  mpPlotCurve,  SLOT(setLineSymbol(QString)));

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

void PlotCurveControlBox::setSizeValue(int value)
{
    mpSizeSpinBox->setValue(value);
}

void PlotCurveControlBox::setLineType(Qt::PenStyle value)
{
    mpLineStyleCombo->setCurrentIndex(PlotCurveStyle::toLineStyleIndex(value));
}

void PlotCurveControlBox::setSymbol(QwtSymbol::Style  value)
{
    mpLineSymbolCombo->setCurrentIndex(PlotCurveStyle::toSymbolIndex(value));
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

    //! @todo need to fix the style so that it is shown which is activated

    mpColorBlob->setStyleSheet(buttonStyle);
}

//! @brief Updates buttons and text in plot info box to correct values
void PlotCurveControlBox::updateInfo()
{
    // Enable/disable generation buttons
    int gen = mpPlotCurve->getCurveGeneration();
    int lowGen, highGen;
    bool haveGens=false;
    auto pLDH = mpPlotCurve->getSharedVectorVariable()->getLogDataHandler();
    if (pLDH)
    {
        pLDH->getVariableGenerationInfo(mpPlotCurve->getDataFullName(), lowGen, highGen);
        haveGens = (lowGen != -1);
    }
    else
    {
        lowGen = highGen = gen;
        haveGens = true;
    }
    mpGenerationSpinBox->blockSignals(true);    // Need to temporarily disconnect to avoid loop
    mpGenerationSpinBox->setRange(lowGen+1, highGen+1);
    if (mpPlotCurve->isCurveGenerationValid())
    {
        QPalette textColor = mpGenerationSpinBox->palette();
        textColor.setColor(QPalette::Text,Qt::black);
        mpGenerationSpinBox->setPalette(textColor);
    }
    else
    {
        QPalette textColor = mpGenerationSpinBox->palette();
        textColor.setColor(QPalette::Text,Qt::red);
        mpGenerationSpinBox->setPalette(textColor);
    }
    mpGenerationSpinBox->setValue(gen+1);
    mpGenerationSpinBox->blockSignals(false);   // Unblock
    mpGenerationSpinBox->setEnabled(haveGens);


    // Set generation number strings
    mpGenerationLabel->setText(QString("[%1,%2]").arg(lowGen+1).arg(highGen+1));

    // Set source label
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

    // Update inverted
    mpInvertCurveCheckBox->blockSignals(true);
    mpInvertCurveCheckBox->setChecked(mpPlotCurve->isInverted());
    mpInvertCurveCheckBox->blockSignals(false);
}

void PlotCurveControlBox::refreshTitle()
{
    mpTitle->setText(mpPlotCurve->getCurveName() + " ["+mpPlotCurve->getCurrentPlotUnit()+"]");
    mpTitle->setToolTip(mpPlotCurve->getSharedVectorVariable()->getFullVariableName());
}

void PlotCurveControlBox::markActive(bool active)
{
    //! @todo this is not visible (marking doesn't show)
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
    mpPlotCurve->setCustomXData(SharedVectorVariableT());
    mpPlotCurve->resetCurveCustomXDataUnitScale();
}

void PlotCurveControlBox::setGeneration(const int gen)
{
    // Since info box begins counting at 1 we need to subtract one, but we do not want underflow as that would set latest generation (-1)
    if (!mpPlotCurve->setGeneration(qMax(gen-1,0)))
    {
        emit hideCurve(mpPlotCurve);
    }
    else
    {
        emit showCurve(mpPlotCurve);
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
    mpResetXDataButton->setIcon(QIcon(QString(ICONPATH) + "svg/Hopsan-ResetTimeVector.svg"));
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
        //! @todo what about model name, if dragging from other model (it should not work but somewhere we need to block and warn) (maybe not here)
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
