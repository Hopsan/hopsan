#include "PlotCurve.h"

#include "PlotWindow.h"
#include "PlotTab.h"

#include "Configuration.h"
#include "MainWindow.h"

#include "Widgets/ProjectTabWidget.h"
#include "Utilities/GUIUtilities.h"

#include "LogDataHandler.h"

#include <limits>
const double DoubleMax = std::numeric_limits<double>::max();

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
        mimeText.remove("HOPSANPLOTDATA:");
    }
    emit newXData(mimeText);
}


//! @brief Constructor for plot info box
//! @param pParentPlotCurve pointer to parent plot curve
//! @param parent Pointer to parent widget
CurveInfoBox::CurveInfoBox(PlotCurve *pParentPlotCurve, QWidget *parent)
    : QWidget(parent)
{
    mpParentPlotCurve = pParentPlotCurve;

    mpColorBlob = new QToolButton(this);
    setLineColor(mpParentPlotCurve->mLineColor);
    mpColorBlob->setFixedSize(20,20);
    mpColorBlob->setCheckable(true);
    mpColorBlob->setChecked(false);

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

    connect(mpColorBlob,               SIGNAL(clicked(bool)),       this,               SLOT(actiavateCurve(bool)));
    connect(mpCustomXDataDrop,         SIGNAL(newXData(QString)),   this,               SLOT(setXData(QString)));
    connect(mpResetTimeButton,         SIGNAL(clicked()),           this,               SLOT(resetTimeVector()));
    connect(mpGenerationSpinBox,       SIGNAL(valueChanged(int)),   this,               SLOT(setGeneration(int)));
    connect(pAutoUpdateCheckBox,       SIGNAL(toggled(bool)),       mpParentPlotCurve,  SLOT(setAutoUpdate(bool)));
    connect(pFrequencyAnalysisButton,  SIGNAL(clicked(bool)),       mpParentPlotCurve,  SLOT(performFrequencyAnalysis()));
    connect(pColorButton,              SIGNAL(clicked()),           mpParentPlotCurve,  SLOT(setLineColor()));
    connect(pScaleButton,              SIGNAL(clicked()),           mpParentPlotCurve,  SLOT(openScaleDialog()));
    connect(pCloseButton,              SIGNAL(clicked()),           mpParentPlotCurve,  SLOT(removeMe()));
    connect(pSizeSpinBox,    SIGNAL(valueChanged(int)),             mpParentPlotCurve,  SLOT(setLineWidth(int)));
    connect(pLineStyleCombo, SIGNAL(currentIndexChanged(QString)),  mpParentPlotCurve,  SLOT(setLineStyle(QString)));
    connect(pLineSymbol,     SIGNAL(currentIndexChanged(QString)),  mpParentPlotCurve,  SLOT(setLineSymbol(QString)));

    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    if(mpParentPlotCurve->getCurveType() != PortVariableType)
    {
        pAutoUpdateCheckBox->setDisabled(true);
        mpGenerationSpinBox->setDisabled(true);
        pFrequencyAnalysisButton->setDisabled(true);
    }
}

void CurveInfoBox::setLineColor(const QColor color)
{
    QString buttonStyle;
//    buttonStyle.append(QString("QToolButton                 { border: 1px solid gray;               border-style: outset;	border-radius: 0px;    	padding: 2px;   background-color: rgb(%1,%2,%3) } ").arg(color.red()).arg(color.green()).arg(color.blue()));
//    buttonStyle.append(QString("QToolButton:pressed 		{ border: 2px solid rgb(70,70,150);   	border-style: outset;   border-radius: 0px;     padding: 0px;   background-color: rgb(%1,%2,%3) } ").arg(color.red()).arg(color.green()).arg(color.blue()));
//    buttonStyle.append(QString("QToolButton:hover:pressed   { border: 2px solid rgb(70,70,150);   	border-style: outset;   border-radius: 0px;     padding: 0px;   background-color: rgb(%1,%2,%3) } ").arg(color.red()).arg(color.green()).arg(color.blue()));
//    buttonStyle.append(QString("QToolButton:hover           { border: 2px solid rgb(70,70,150);   	border-style: outset;   border-radius: 0px;     padding: 0px;   background-color: rgb(%1,%2,%3) } ").arg(color.red()).arg(color.green()).arg(color.blue()));
//    buttonStyle.append(QString("QToolButton:checked         { border: 1px solid gray;               border-style: inset;    border-radius: 0px;    	padding: 1px;   background-color: rgb(%1,%2,%3) } ").arg(color.red()).arg(color.green()).arg(color.blue()));
//    buttonStyle.append(QString("QToolButton:hover:checked   { border: 2px solid rgb(70,70,150);   	border-style: outset;   border-radius: 0px;     padding: 0px;   background-color: rgb(%1,%2,%3) } ").arg(color.red()).arg(color.green()).arg(color.blue()));
//    buttonStyle.append(QString("QToolButton:unchecked		{ border: 1px solid gray;               border-style: outset;	border-radius: 0px;    	padding: 0px;   background-color: rgb(%1,%2,%3) } ").arg(color.red()).arg(color.green()).arg(color.blue()));
//    buttonStyle.append(QString("QToolButton:hover:unchecked { border: 1px solid gray;               border-style: outset;   border-radius: 0px;     padding: 2px;   background-color: rgb(%1,%2,%3) } ").arg(color.red()).arg(color.green()).arg(color.blue()));

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
void CurveInfoBox::updateInfo()
{
    // Enable/diable generation buttons
    const int lowGen = mpParentPlotCurve->getLogDataVariablePtr()->getLowestGeneration();
    const int highGen = mpParentPlotCurve->getLogDataVariablePtr()->getHighestGeneration();
    const int gen = mpParentPlotCurve->getGeneration();
    const int nGen = mpParentPlotCurve->getLogDataVariablePtr()->getNumGenerations();
    disconnect(mpGenerationSpinBox,       SIGNAL(valueChanged(int)),   this,  SLOT(setGeneration(int))); //Need to temporarily disconnect to avoid loop
    mpGenerationSpinBox->setRange(lowGen+1, highGen+1);
    mpGenerationSpinBox->setValue(gen+1);
    connect(mpGenerationSpinBox,       SIGNAL(valueChanged(int)),   this,  SLOT(setGeneration(int)));
    mpGenerationSpinBox->setEnabled(nGen > 1);

    // Set generation number strings
    //! @todo this will show strange when we have deleted old generations, maybe we should reassign all generations when we delete old data (costly)
    mpGenerationLabel->setText(QString("[%1,%2]").arg(lowGen+1).arg(highGen+1));

    // Update curve name
    refreshTitle();

    // Update Xdata
    if (mpParentPlotCurve->hasCustomXData())
    {
        mpCustomXDataDrop->setText(mpParentPlotCurve->getCustomXData()->getFullVariableName());
        mpResetTimeButton->setEnabled(true);
    }
    else
    {
        mpCustomXDataDrop->setText("");
        mpResetTimeButton->setEnabled(false);
    }
}

void CurveInfoBox::refreshTitle()
{
    mpTitle->setText(mpParentPlotCurve->getCurveName() + " ["+mpParentPlotCurve->getDataUnit()+"]");
}

void CurveInfoBox::refreshActive(bool active)
{
    mpColorBlob->setChecked(active);
}

void CurveInfoBox::actiavateCurve(bool active)
{
    if(active)
    {
        mpParentPlotCurve->mpParentPlotTab->setActivePlotCurve(mpParentPlotCurve);
    }
    else
    {
        mpParentPlotCurve->mpParentPlotTab->setActivePlotCurve(0);
    }
}

void CurveInfoBox::setXData(QString fullName)
{
    mpParentPlotCurve->setCustomXData(fullName);
}

void CurveInfoBox::resetTimeVector()
{
    mpParentPlotCurve->setCustomXData("");
}

void CurveInfoBox::setGeneration(int gen)
{
    mpParentPlotCurve->setGeneration(--gen);
}

//! @brief Constructor for plot curves.
//! @param generation Generation of plot data to use
//! @param componentName Name of component where plot data is located
//! @param portName Name of port where plot data is located
//! @param dataName Name of physical quantity to use (e.g. "Pressure", "Velocity"...)
//! @param dataUnit Name of unit to show data in
//! @param axisY Which Y-axis to use (QwtPlot::yLeft or QwtPlot::yRight)
//! @param parent Pointer to plot tab which curve shall be created it
PlotCurve::PlotCurve(SharedLogVariableDataPtrT pData,
                     int axisY,
                     PlotTab *parent,
                     HopsanPlotIDEnumT plotID,
                     HopsanPlotCurveTypeEnumT curveType)
{
    mHaveCustomData = false;
    mpData = pData;
    commonConstructorCode(axisY, parent, plotID, curveType);
}

//! @brief Consturctor for custom data
PlotCurve::PlotCurve(const VariableDescription &rVarDesc,
                     const QVector<double> &rXVector,
                     const QVector<double> &rYVector,
                     int axisY,
                     PlotTab *parent,
                     HopsanPlotIDEnumT plotID,
                     HopsanPlotCurveTypeEnumT curveType)
{
    //! @todo see if it is possible to reuse the setCustomData member function
    LogVariableContainer *pDataContainer = new LogVariableContainer(rVarDesc,0);
    pDataContainer->addDataGeneration(0, rXVector, rYVector);
    mHaveCustomData = true;
    mpData = pDataContainer->getDataGeneration(0);
    commonConstructorCode(axisY, parent, plotID, curveType);
}

void PlotCurve::commonConstructorCode(int axisY,
                                      PlotTab* parent,
                                      HopsanPlotIDEnumT plotID,
                                      HopsanPlotCurveTypeEnumT curveType)
{
    mCustomDataUnitScale = 1.0;
    mpCurveSymbol = 0;
    mIsActive = false;
    mCurveType = curveType;
    mpParentPlotTab = parent;

    mAxisY = axisY;
    mAutoUpdate = true;
    mPlotScaleX = 1.0;
    mPlotScaleY = 1.0;
    mPlotOffsetX = 0;
    mPlotOffsetY = 0;

    // Set QwtPlotCurve stuff
    //! @todo maybe this code should be run when we are adding a curve to a plottab
    this->setTitle(getCurveName());
    updateCurve();
    this->setRenderHint(QwtPlotItem::RenderAntialiased);
    this->setYAxis(axisY);
    this->attach(parent->getPlot(plotID));
    this->setItemAttribute(QwtPlotItem::Legend, true);


    //Create the plot info box
    mpPlotCurveInfoBox = new CurveInfoBox(this, mpParentPlotTab);
    mpPlotCurveInfoBox->setPalette(gConfig.getPalette());
    updatePlotInfoBox();
    mpParentPlotTab->mpCurveInfoScrollArea->widget()->layout()->addWidget(mpPlotCurveInfoBox);
    mpPlotCurveInfoBox->show();

    if(curveType != PortVariableType)
    {
        setAutoUpdate(false);
    }

    //Create connections
    //! @todo we should not connect like this /Peter
    connect(gpMainWindow->mpModelHandler->getCurrentModel(),SIGNAL(simulationFinished()),this,SLOT(updateToNewGeneration()));
    connect(gpMainWindow->mpCentralTabs,SIGNAL(simulationFinished()),this,SLOT(updateToNewGeneration()));

    connectDataSignals();

    if (mpData->getLogDataHandler())
    {
        mpData->getLogDataHandler()->incrementOpenPlotCurves();
    }
}

//! @brief Destructor for plot curves
PlotCurve::~PlotCurve()
{
    // If the curve data had a data handler then decrement its open curves counter
    LogDataHandler* pDataHandler = mpData->getLogDataHandler();
    if (pDataHandler)
    {
        pDataHandler->decrementOpenPlotCurves();
    }

    // Delete the plot info box for this curve
    delete mpPlotCurveInfoBox;

    // Delete custom data if any
    deleteCustomData();
}


//! @brief Returns the current generation a plot curve is representing
int PlotCurve::getGeneration() const
{
    return mpData->getGeneration();
}

QString PlotCurve::getCurveName() const
{
    if(mCurveType == PortVariableType)
    {
        if (mpData->getAliasName().isEmpty())
        {
            return mpData->getFullVariableNameWithSeparator(", ");
        }
        else
        {
            return mpData->getAliasName();
        }
    }
    else if(mCurveType == FrequencyAnalysisType)
        return "Frequency Spectrum";
    else if(mCurveType == NyquistType)
        return "Nyquist Plot";
    else if(mCurveType == BodeGainType)
        return "Magnitude Plot";
    else if(mCurveType == BodePhaseType)
        return "Phase Plot";
    else
        return "Unnamed Curve";
}


//! @brief Returns the type of the curve
HopsanPlotCurveTypeEnumT PlotCurve::getCurveType()
{
    return mCurveType;
}


//! @brief Returns the name of the component a plot curve is created from
QString PlotCurve::getComponentName()
{
    return mpData->getComponentName();
}


//! @brief Returns the name of the port a plot curve is created from
QString PlotCurve::getPortName()
{
    return mpData->getPortName();
}


//! @brief Returns the data name (physical quantity) of a plot curve
QString PlotCurve::getDataName()
{
    return mpData->getDataName();
}


//! @brief Returns the current data unit of a plot curve
QString PlotCurve::getDataUnit()
{
    if (mCustomDataUnit.isEmpty())
    {
        return mpData->getDataUnit();
    }
    else
    {
        return mCustomDataUnit;
    }
}

const SharedLogVariableDataPtrT PlotCurve::getLogDataVariablePtr() const
{
    return mpData;
}


//! @brief Tells which Y-axis a plot curve is assigned to
int PlotCurve::getAxisY()
{
    return mAxisY;
}


//! @brief Returns the (unscaled) data vector of a plot curve
QVector<double> PlotCurve::getDataVector() const
{
    //! @todo this is no longer a reference need to see where it was used to avoid REALY slow code feetching data all the time /Peter
    return mpData->getDataVector();
}


//! @brief Returns the (unscaled) time vector of a plot curve
//! This returns the TIME vector, NOT any special X-axes if they are used.
const QVector<double> &PlotCurve::getTimeVector() const
{
    return *(mpData->mSharedTimeVectorPtr.data());
}

bool PlotCurve::hasCustomXData() const
{
    return !mpCustomXdata.isNull();
}

const SharedLogVariableDataPtrT PlotCurve::getCustomXData() const
{
    return mpCustomXdata;
}


//! @brief Sets the generation of a plot curve
//! Updates the data to specified generation, and updates plot info box.
//! @param genereation Genereation to use
void PlotCurve::setGeneration(int generation)
{
    // Make sure we don try to use logdata handler from a variable that does not have one
    if(mpData->getLogDataHandler())
    {
        SharedLogVariableDataPtrT pNewData = mpData->getLogDataHandler()->getPlotData(mpData->getFullVariableName(), generation);
        if (pNewData)
        {
            this->disconnect(mpData.data());
            mpData = pNewData;
            connectDataSignals();
        }

        //! @todo should not all updates happen automatically from one command
//        mpParentPlotTab->rescaleAxesToCurves();
//        mpParentPlotTab->update();

        if (hasCustomXData())
        {
            //! @todo why not be able to ask parent data container for other generations
            if (mpCustomXdata->getLogDataHandler())
            {
                SharedLogVariableDataPtrT pNewXData = mpCustomXdata->getLogDataHandler()->getPlotData(mpCustomXdata->getFullVariableName(), generation);
                if (pNewXData)
                {
                    setCustomXData(pNewXData);
                }
            }
        }

        updateCurve();
        updatePlotInfoBox();
    }
}


//! @brief Sets the unit of a plot curve
//! @param unit Name of new unit
//! @note If unit is not registered for data then nothing will happen
void PlotCurve::setCustomDataUnit(QString unit)
{
    if (gConfig.hasUnitScale(getDataName(),unit))
    {
        setCustomDataUnit(unit, gConfig.getUnitScale(getDataName(),unit));
    }
}

//! @brief Sets a custom unit and scale of a plot curve
//! @param unit Name of new unit
//! @param scale What scaling towards default (usually SI) unit to use
void PlotCurve::setCustomDataUnit(const QString unit, double scale)
{
    mCustomDataUnit = unit;
    mCustomDataUnitScale = scale;

    updateCurve();

    //! @todo shouldnt these be triggered by signal in update curve?
    mpParentPlotTab->updateLabels();
    mpParentPlotTab->update();
}


//! @brief Sets the (plot only) scaling of a plot curve
//! @param scaleX Scale factor for X-axis
//! @param scaleY Scale factor for Y-axis
//! @param offsetX Offset value for X-axis
//! @param offsetY Offset value for Y-axis
void PlotCurve::setScaling(double scaleX, double scaleY, double offsetX, double offsetY)
{
    mPlotScaleX=scaleX;
    mPlotScaleY=scaleY;
    mPlotOffsetX=offsetX;
    mPlotOffsetY=offsetY;
    updateCurve();
}


void PlotCurve::setCustomData(const VariableDescription &rVarDesc, const QVector<double> &rvTime, const QVector<double> &rvData)
{
    //First disconnect all signals from the old data
    this->disconnect(mpData.data());

    //If we already have custom data, then delete it from memory as it is being replaced
    deleteCustomData();

    //Create new custom data
    LogVariableContainer *pDataContainer = new LogVariableContainer(rVarDesc, 0);
    pDataContainer->addDataGeneration(0, rvTime, rvData);
    mHaveCustomData = true;
    mpData = pDataContainer->getDataGeneration(0);

    //Connect signals
    connectDataSignals();

    updateCurve();
}

void PlotCurve::setCustomXData(const VariableDescription &rVarDesc, const QVector<double> &rvXdata)
{
    //! @todo need a nicer way to easily create a new shared logdatavariables
    LogVariableContainer *pData = new LogVariableContainer(rVarDesc,0);
    pData->addDataGeneration(0, SharedTimeVectorPtrT(), rvXdata);
    setCustomXData(pData->getDataGeneration(0));
}

void PlotCurve::setCustomXData(SharedLogVariableDataPtrT pData)
{
    // Disconnect any signals first, in case we are changing x-data
    if (mpCustomXdata)
    {
        disconnect(mpCustomXdata.data(),0,this,0);
    }
    // Set new data and connect signals
    mpCustomXdata = pData;
    connectDataSignals();

    // Redraw curve
    updateCurve();
    mpPlotCurveInfoBox->updateInfo();
}

void PlotCurve::setCustomXData(const QString fullName)
{
    // If empty then reset time vector
    if (fullName.isEmpty())
    {
        setCustomXData(SharedLogVariableDataPtrT());
    }
    else
    {
        LogDataHandler *pHandler = mpData->getLogDataHandler();
        if (pHandler)
        {
            SharedLogVariableDataPtrT pData = pHandler->getPlotData(fullName, mpData->getGeneration());
            if (pData)
            {
                setCustomXData(pData);
            }
        }
    }
}


//! @brief Converts the plot curve to its frequency spectrum by using FFT
void PlotCurve::toFrequencySpectrum(const bool doPowerSpectrum)
{
    QVector<double> timeVec, dataVec;
    timeVec = *(mpData->mSharedTimeVectorPtr.data());
    dataVec = mpData->getDataVector();

    //Vector size has to be an even potential of 2.
    //Calculate largets potential that is smaller than or equal to the vector size.
    int n = pow(2, int(log2(dataVec.size())));
    if(n != dataVec.size())     //Vector is not an exact potential, so reduce it
    {
        QString oldString, newString;
        oldString.setNum(dataVec.size());
        newString.setNum(n);
        QMessageBox::information(gpMainWindow, gpMainWindow->tr("Wrong Vector Size"),
                                 "Size of data vector must be an even power of 2. Number of log samples was reduced from " + oldString + " to " + newString + ".");
        reduceVectorSize(dataVec, n);
        reduceVectorSize(timeVec, n);
    }

    //Create a complex vector
    QVector< std::complex<double> > vComplex = realToComplex(dataVec);

    //Apply the fourier transform
    FFT(vComplex);

    //Scalar multiply complex vector with its conjugate, and divide it with its size
    dataVec.clear();
    for(int i=1; i<n/2; ++i)        //FFT is symmetric, so only use first half
    {
        if(doPowerSpectrum)
        {
            dataVec.append(real(vComplex[i]*conj(vComplex[i]))/n);
        }
        else
        {
            dataVec.append(sqrt(vComplex[i].real()*vComplex[i].real() + vComplex[i].imag()*vComplex[i].imag()));
        }
    }

    //Create the x vector (frequency)
    double max = timeVec.last();
    timeVec.clear();
    for(int i=1; i<n/2; ++i)
    {
        timeVec.append(double(i)/max);
    }

    VariableDescription varDesc;
    varDesc.mDataName = "Value";
    varDesc.mDataUnit = "-";
    this->setCustomData(varDesc, timeVec, dataVec);

    varDesc.mDataName = "Frequency";
    varDesc.mDataUnit = "Hz";
    this->setCustomXData(varDesc,timeVec);

    updateCurve();
    updatePlotInfoBox();
    mpParentPlotTab->update();
}

void PlotCurve::resetLegendSize()
{
    // For now hardcoded but maybe in the future be possible to select, (default 8x8 is to small to see difference between dashed and solid lines)
    setLegendIconSize(QSize(40,12));
}

SharedLogVariableDataPtrT PlotCurve::getLogDataVariablePtr()
{
    return mpData;
}


//! @brief Changes a curve to the previous available gneraetion of its data
void PlotCurve::setPreviousGeneration()
{
    // Prevent cykeling back to last by not sending in -1
    setGeneration(std::max(getGeneration()-1,0));
}


//! @brief Changes a curve to the next available generation of its data
void PlotCurve::setNextGeneration()
{
    setGeneration(getGeneration()+1);
}


//! @brief Sets the line width of a plot curve
//! @param lineWidth Line width to give curve
void PlotCurve::setLineWidth(int lineWidth)
{
    mLineWidth = lineWidth;
    QPen tempPen = pen();
    // Add one pt extra width for active curves
    if (mIsActive)
    {
        tempPen.setWidth(lineWidth+1);
    }
    else
    {
        tempPen.setWidth(lineWidth);
    }
    setPen(tempPen);
}


void PlotCurve::setLineStyle(QString lineStyle)
{
    mLineStyle = lineStyle;
    setStyle(PlotCurve::Lines); //Assume we want lines
    QPen tempPen = pen();
    if(lineStyle == "Solid Line")
    {
        tempPen.setStyle(Qt::SolidLine);
    }
    else if(lineStyle == "Dash Line")
    {
        tempPen.setStyle(Qt::DashLine);
    }
    else if(lineStyle == "Dot Line")
    {
        tempPen.setStyle(Qt::DotLine);
    }
    else if(lineStyle == "Dash Dot Line")
    {
        tempPen.setStyle(Qt::DashDotLine);
    }
    else if(lineStyle == "Dash Dot Dot Line")
    {
        tempPen.setStyle(Qt::DashDotDotLine);
    }
    else
    {
        // Deactivate line completely
        setStyle(PlotCurve::NoCurve);
    }
    setPen(tempPen);
    resetLegendSize();
}

void PlotCurve::setLineSymbol(QString lineSymbol)
{
    mLineSymbol = lineSymbol;
    mpCurveSymbol = new QwtSymbol();
    if(lineSymbol == "Cross")
    {
        mpCurveSymbol->setStyle(QwtSymbol::Cross);
    }
    else if(lineSymbol == "XCross")
    {
        mpCurveSymbol->setStyle(QwtSymbol::XCross);
    }
    else if(lineSymbol == "Ellipse")
    {
        mpCurveSymbol->setStyle(QwtSymbol::Ellipse);
    }
    else if(lineSymbol == "Star 1")
    {
        mpCurveSymbol->setStyle(QwtSymbol::Star1);
    }
    else if(lineSymbol == "Star 2")
    {
        mpCurveSymbol->setStyle(QwtSymbol::Star2);
    }
    else if(lineSymbol == "Hexagon")
    {
        mpCurveSymbol->setStyle(QwtSymbol::Hexagon);
    }
    else if(lineSymbol == "Rectangle")
    {
        mpCurveSymbol->setStyle(QwtSymbol::Rect);
    }
    else if(lineSymbol == "Horizontal Line")
    {
        mpCurveSymbol->setStyle(QwtSymbol::HLine);
    }
    else if(lineSymbol == "Vertical Line")
    {
        mpCurveSymbol->setStyle(QwtSymbol::VLine);
    }
    else if(lineSymbol == "Diamond")
    {
        mpCurveSymbol->setStyle(QwtSymbol::Diamond);
    }
    else if(lineSymbol == "Triangle")
    {
        mpCurveSymbol->setStyle(QwtSymbol::Triangle);
    }
    else if(lineSymbol == "Up Triangle")
    {
        mpCurveSymbol->setStyle(QwtSymbol::UTriangle);
    }
    else if(lineSymbol == "Down Triangle")
    {
        mpCurveSymbol->setStyle(QwtSymbol::DTriangle);
    }
    else if(lineSymbol == "Right Triangle")
    {
        mpCurveSymbol->setStyle(QwtSymbol::RTriangle);
    }
    else if(lineSymbol == "Left Triangle")
    {
        mpCurveSymbol->setStyle(QwtSymbol::LTriangle);
    }
    else
    {
        mpCurveSymbol->setStyle(QwtSymbol::NoSymbol);
    }

    QPen tempPen = pen();
    tempPen.setStyle(Qt::SolidLine);
    mpCurveSymbol->setPen(tempPen);
    mpCurveSymbol->setSize(8,8);
    setSymbol(mpCurveSymbol);

    //! @todo Add a color or size picker for the markers
    resetLegendSize();
}

//! @brief Sets the color of a line
//! @brief color Color to give the line.
void PlotCurve::setLineColor(QColor color)
{
    QPen tempPen;
    mLineColor = color;

    // Set line color
    tempPen = pen();
    tempPen.setColor(color);
    setPen(tempPen);

    // Set symbol color, (but only if we have one, else an empty symbold will be created)
    if (mpCurveSymbol)
    {
        // Need to recreate the symbol so that legend will update
        setLineSymbol(mLineSymbol);
    }

    // Set blob color
    mpPlotCurveInfoBox->setLineColor(color);
}


//! @brief Sets the color of a line
//! @param colorName Svg name of the color
//! @see setLineColor(QColor color)
void PlotCurve::setLineColor(QString colorName)
{
    QColor color;
    if(colorName.isEmpty())
    {
        color = QColorDialog::getColor(pen().color(), gpMainWindow);
        if (!color.isValid()) { return; }
    }
    else
    {
        color = QColor(colorName);
    }
    setLineColor(color);
}


//! @brief Opens the scaling dialog for a plot curve
void PlotCurve::openScaleDialog()
{
    QDialog *pScaleDialog = new QDialog(mpParentPlotTab->mpParentPlotWindow);
    pScaleDialog->setWindowTitle("Change Curve Scale");

    QLabel *pXScaleLabel = new QLabel("Time Axis Scale: ", pScaleDialog);
    mpXScaleSpinBox = new QDoubleSpinBox(pScaleDialog);
    mpXScaleSpinBox->setRange(-DoubleMax, DoubleMax);
    mpXScaleSpinBox->setDecimals(10);
    mpXScaleSpinBox->setSingleStep(0.1);
    mpXScaleSpinBox->setValue(mPlotScaleX);

    QLabel *pXOffsetLabel = new QLabel("Time Axis Offset: ", pScaleDialog);
    mpXOffsetSpinBox = new QDoubleSpinBox(pScaleDialog);
    mpXOffsetSpinBox->setDecimals(10);
    mpXOffsetSpinBox->setRange(-DoubleMax, DoubleMax);
    mpXOffsetSpinBox->setSingleStep(0.1);
    mpXOffsetSpinBox->setValue(mPlotOffsetX);

    QLabel *pYScaleLabel = new QLabel("Y-Axis Scale: ", pScaleDialog);
    mpYScaleSpinBox = new QDoubleSpinBox(pScaleDialog);
    mpYScaleSpinBox->setSingleStep(0.1);
    mpYScaleSpinBox->setDecimals(10);
    mpYScaleSpinBox->setRange(-DoubleMax, DoubleMax);
    mpYScaleSpinBox->setValue(mPlotScaleY);

    QLabel *pYOffsetLabel = new QLabel("Y-Axis Offset: ", pScaleDialog);
    mpYOffsetSpinBox = new QDoubleSpinBox(pScaleDialog);
    mpYOffsetSpinBox->setDecimals(10);
    mpYOffsetSpinBox->setRange(-DoubleMax, DoubleMax);
    mpYOffsetSpinBox->setSingleStep(0.1);
    mpYOffsetSpinBox->setValue(mPlotOffsetY);

    QPushButton *pDoneButton = new QPushButton("Done", pScaleDialog);
    QDialogButtonBox *pButtonBox = new QDialogButtonBox(Qt::Horizontal);
    pButtonBox->addButton(pDoneButton, QDialogButtonBox::ActionRole);

    QGridLayout *pDialogLayout = new QGridLayout(pScaleDialog);
    pDialogLayout->addWidget(pXScaleLabel,0,0);
    pDialogLayout->addWidget(mpXScaleSpinBox,0,1);
    pDialogLayout->addWidget(pXOffsetLabel,1,0);
    pDialogLayout->addWidget(mpXOffsetSpinBox,1,1);
    pDialogLayout->addWidget(pYScaleLabel,2,0);
    pDialogLayout->addWidget(mpYScaleSpinBox,2,1);
    pDialogLayout->addWidget(pYOffsetLabel,3,0);
    pDialogLayout->addWidget(mpYOffsetSpinBox,3,1);
    pDialogLayout->addWidget(pButtonBox,4,0,1,2);
    pScaleDialog->setLayout(pDialogLayout);
    pScaleDialog->show();

    connect(pDoneButton,SIGNAL(clicked()),pScaleDialog,SLOT(close()));
    connect(mpXScaleSpinBox, SIGNAL(valueChanged(double)), SLOT(updateScaleFromDialog()));
    connect(mpXOffsetSpinBox, SIGNAL(valueChanged(double)), SLOT(updateScaleFromDialog()));
    connect(mpYScaleSpinBox, SIGNAL(valueChanged(double)), SLOT(updateScaleFromDialog()));
    connect(mpYOffsetSpinBox, SIGNAL(valueChanged(double)), SLOT(updateScaleFromDialog()));
}


//! @brief Updates the scaling of a plot curve form values in scaling dialog
void PlotCurve::updateScaleFromDialog()
{
    setScaling(mpXScaleSpinBox->value(), mpYScaleSpinBox->value(), mpXOffsetSpinBox->value(), mpYOffsetSpinBox->value());
    mpParentPlotTab->rescaleAxesToCurves();
}


//! @brief Tells the parent plot tab of a curve to remove it
void PlotCurve::removeMe()
{
    mpParentPlotTab->removeCurve(this);
}


//! @brief Updates a plot curve to the most recent available generation of its data
void PlotCurve::updateToNewGeneration()
{
    if(mAutoUpdate)     //Only change the generation if auto update is on
    {
        setGeneration(-1);
    }
    updatePlotInfoBox();    //Update the plot info box regardless of auto update setting, to show number of available generations correctly
//    mpParentPlotTab->rescaleAxesToCurves();
}

void PlotCurve::updatePlotInfoBox()
{
    mpPlotCurveInfoBox->updateInfo();
}


//! @brief Activates (highlights) the plot curve
void PlotCurve::markActive(bool value)
{
    if(value)
    {
        mIsActive = true;
        //mpPlotInfoBox->setPalette(QPalette(QColor("lightgray"), QColor("lightgray")));
        mpPlotCurveInfoBox->setAutoFillBackground(true);
        mpPlotCurveInfoBox->setPalette(gConfig.getPalette());
    }
    else
    {
        mIsActive = false;
        mpPlotCurveInfoBox->setAutoFillBackground(true);
    }

    setLineWidth(mLineWidth);
    mpPlotCurveInfoBox->refreshActive(mIsActive);
}


//! @brief Redraws the curve
//! Updates a curve with regard to special X-axis, units and scaling.
//! @todo after updating from python, scale is not refreshed maybe this should be done in here
//! @todo add optional index if we only want to update particular value
void PlotCurve::updateCurve()
{
    QVector<double> tempX;
    // We copy here, it should be faster then peek (at least when data is cached on disc)
    QVector<double> tempY = mpData->getDataVector();

    if(mpCustomXdata.isNull())
    {
        // No special X-data use time vector if it exist else we cant draw curve (yet, x-date might be set later)
        if (mpData->mSharedTimeVectorPtr)
        {
            tempX.resize(mpData->mSharedTimeVectorPtr->size());
            for(int i=0; i<tempX.size() && i<tempY.size(); ++i)
            {
                tempX[i] = mpData->mSharedTimeVectorPtr->at(i)*mPlotScaleX + mPlotOffsetX;
                tempY[i] = tempY[i]*mCustomDataUnitScale*mPlotScaleY + mPlotOffsetY;
            }
        }
        else
        {
            //! @todo this is a HACK really need a curve constructor for two variables x,y /Peter
            tempX = tempY;
        }
    }
    else
    {
        // Use special X-data
        // We copy here, it should be faster then peek (at least when data is cached on disc)
        tempX = mpCustomXdata->getDataVector();
        for(int i=0; i<tempX.size() && i<tempY.size(); ++i)
        {
            tempX[i] = tempX[i]*mPlotScaleX + mPlotOffsetX;
            tempY[i] = tempY[i]*mCustomDataUnitScale*mPlotScaleY + mPlotOffsetY;
        }
    }

    setSamples(tempX, tempY);
    emit curveDataUpdated();
}

void PlotCurve::updateCurveName()
{
    if (mpData->getAliasName().isEmpty())
    {
        setTitle(mpData->getFullVariableNameWithSeparator(", "));
    }
    else
    {
        setTitle(mpData->getAliasName());
    }
    updatePlotInfoBox();
}

void PlotCurve::deleteCustomData()
{
    if (mHaveCustomData)
    {
        mpData.clear();
        mHaveCustomData = false;
    }
}

void PlotCurve::connectDataSignals()
{
    connect(mpData.data(), SIGNAL(dataChanged()), this, SLOT(updateCurve()), Qt::UniqueConnection);
    connect(mpData.data(), SIGNAL(nameChanged()), this, SLOT(updateCurveName()), Qt::UniqueConnection);
    if (mpCustomXdata)
    {
        connect(mpCustomXdata.data(), SIGNAL(dataChanged()), this, SLOT(updateCurve()), Qt::UniqueConnection);
    }
}




//! @brief Sets auto update flag for a plot curve
//! If this is activated, plot will automatically change to latest plot generation after next simulation.
void PlotCurve::setAutoUpdate(bool value)
{
    mAutoUpdate = value;
}


void PlotCurve::performFrequencyAnalysis()
{
    mpParentPlotTab->mpParentPlotWindow->performFrequencyAnalysis(this);
}



//! @brief Constructor for plot markers
//! @param pCurve Pointer to curve the marker belongs to
//! @param pPlotTab Plot tab the marker is located in
//! @param markerSymbol The symbol the marker shall use
PlotMarker::PlotMarker(PlotCurve *pCurve, PlotTab *pPlotTab)
    : QwtPlotMarker()
{
    mpCurve = pCurve;
    mpPlotTab = pPlotTab;
    mIsBeingMoved = false;
    mIsMovable = true;

    mpMarkerSymbol = new QwtSymbol();
    mpMarkerSymbol->setStyle(QwtSymbol::XCross);
    mpMarkerSymbol->setSize(10,10);
    mpMarkerSymbol->setPen(pCurve->pen().color(),3);
    setSymbol(mpMarkerSymbol); //!< @todo is this symbol auto removed with PlotMarker ?
}


//! @brief Event filter for plot markers
//! This will interrupt events from plot canvas, to enable using mouse and key events for modifying markers.
//! @returns True if event was interrupted, false if its propagation shall continue
//! @param object Pointer to the object the event belongs to (in this case the plot canvas)
//! @param ev ent Event to be interrupted
bool PlotMarker::eventFilter(QObject */*object*/, QEvent *event)
{
    if(!mIsMovable)
        return false; //!< @todo this will also block delete, is that OK?

    // Mouse press events, used to initiate moving of a marker if mouse cursor is close enough
    if (event->type() == QEvent::MouseButtonPress)
    {
        QCursor cursor;
        QPointF midPoint;
        midPoint.setX(this->plot()->transform(QwtPlot::xBottom, value().x()));
        midPoint.setY(this->plot()->transform(QwtPlot::yLeft, value().y()));

        if(!mpPlotTab->mpZoomerLeft[FirstPlot]->isEnabled() && !mpPlotTab->mpPanner[FirstPlot]->isEnabled())
        {
            if((this->plot()->canvas()->mapToGlobal(midPoint.toPoint()) - cursor.pos()).manhattanLength() < 35)
            {
                mIsBeingMoved = true;
                return true;
            }
        }
    }

    // Mouse move (hover) events, used to change marker color or move marker if cursor is close enough.
    else if (event->type() == QEvent::MouseMove)
    {
        bool retval = false;
        QCursor cursor;
        QPointF midPoint;
        midPoint.setX(this->plot()->transform(QwtPlot::xBottom, value().x()));
        midPoint.setY(this->plot()->transform(QwtPlot::yLeft, value().y()));
        if((this->plot()->canvas()->mapToGlobal(midPoint.toPoint()) - cursor.pos()).manhattanLength() < 35)
        {
            mpMarkerSymbol->setPen(mpCurve->pen().color().lighter(165), 3);
            //this->setSymbol(mpMarkerSymbol);
            this->plot()->replot();
            this->plot()->updateGeometry();
            retval=true;
        }
        else
        {
            if(!mIsBeingMoved)
            {
                mpMarkerSymbol->setPen(mpCurve->pen().color(), 3);
                //this->setSymbol(mpMarkerSymbol);
                this->plot()->replot();
                this->plot()->updateGeometry();
            }
        }

        if(mIsBeingMoved)
        {
            double x = mpCurve->sample(mpCurve->closestPoint(this->plot()->canvas()->mapFromGlobal(cursor.pos()))).x();
            double y = mpCurve->sample(mpCurve->closestPoint(this->plot()->canvas()->mapFromGlobal(cursor.pos()))).y();
            setXValue(x);
            setYValue(this->plot()->invTransform(QwtPlot::yLeft, this->plot()->transform(mpCurve->yAxis(), y)));

            refreshLabel(x, y);
        }
        return retval;
    }

    // Mouse release event, will stop moving marker
    else if (event->type() == QEvent::MouseButtonRelease && mIsBeingMoved == true)
    {
        mIsBeingMoved = false;
        return false;
    }

    // Keypress event, will delete marker if delete key is pressed
    else if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if(keyEvent->key() == Qt::Key_Delete)
        {
            QCursor cursor;
            QPointF midPoint;
            midPoint.setX(this->plot()->transform(QwtPlot::xBottom, value().x()));
            midPoint.setY(this->plot()->transform(mpCurve->yAxis(), value().y()));
            if((this->plot()->canvas()->mapToGlobal(midPoint.toPoint()) - cursor.pos()).manhattanLength() < 35)
            {
                plot()->canvas()->removeEventFilter(this);
                for(int plotID=0; plotID<2; ++plotID)
                {
                    mpPlotTab->mMarkerPtrs[plotID].removeAll(this);     //Cycle all plots and remove the marker if it is found
                }
                this->hide();           // This will only hide and inactivate the marker. Deleting it seem to make program crash.
                this->detach();
                this->deleteLater();
                return true;
            }
        }
        return false;
    }
    return false;
}


void PlotMarker::setMovable(bool movable)
{
    mIsMovable = movable;
}

void PlotMarker::refreshLabel(const double x, const double y)
{
    refreshLabel(QString("(%1, %2)").arg(x).arg(y));
}

void PlotMarker::refreshLabel(const QString label)
{
    QwtText qwtlabel(label);
    qwtlabel.setColor(Qt::black);
    qwtlabel.setBackgroundBrush(QColor(255,255,255,220));
    qwtlabel.setFont(QFont("Calibri", 12, QFont::Normal));
    this->setLabel(qwtlabel);
    this->setLabelAlignment(Qt::AlignTop);
}


//! @brief Returns a pointer to the curve a plot marker belongs to
PlotCurve *PlotMarker::getCurve()
{
    return mpCurve;
}


QList<QwtLegendData> PlotCurve::legendData() const
{
    QList<QwtLegendData> list = QwtPlotCurve::legendData();
    for (int i=0; i<list.size(); ++i)
    {
        list[i].setValue( AxisIdRole, this->yAxis() );
    }
    return list;
}

PlotLegend::PlotLegend(QwtPlot::Axis axisId) :
    QwtPlotLegendItem()
{
    setMaxColumns(1);
    setRenderHint( QwtPlotItem::RenderAntialiased );
    setBackgroundMode(LegendBackground);
    setBackgroundBrush(QColor(Qt::white));
    setBorderRadius(8);
    setMargin(4);
    setSpacing(2);
    setItemMargin(0);
    QFont font = this->font();
    font.setPointSize(11);
    setFont(font);

    mAxis = axisId;
}

void PlotLegend::updateLegend(const QwtPlotItem *plotItem, const QList<QwtLegendData> &data)
{
    // Use only those curve pointers that should belong to this particular legend
    QList<QwtLegendData> myData;
    for (int i=0; i<data.size(); ++i)
    {
        if (data[i].value(AxisIdRole) == mAxis)
        {
            myData.push_back(data[i]);
        }
    }

    QwtPlotLegendItem::updateLegend( plotItem, myData );
}

