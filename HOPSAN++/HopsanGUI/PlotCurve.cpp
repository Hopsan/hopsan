#include "PlotCurve.h"

#include "PlotWindow.h"
#include "PlotTab.h"

#include "Configuration.h"
#include "MainWindow.h"

#include "Widgets/ProjectTabWidget.h"
#include "Utilities/GUIUtilities.h"

#include "LogDataHandler.h"

#include <limits>
const double DBLMAX = std::numeric_limits<double>::max();


//! @brief Constructor for plot info box
//! @param pParentPlotCurve pointer to parent plot curve
//! @param parent Pointer to parent widget
PlotCurveInfoBox::PlotCurveInfoBox(PlotCurve *pParentPlotCurve, QWidget *parent)
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

    mpPreviousButton = new QToolButton(this);
    mpPreviousButton->setToolTip("Previous Generation");
    mpPreviousButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-StepLeft.png"));

    mpNextButton = new QToolButton(this);
    mpNextButton->setToolTip("Next Generation");
    mpNextButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-StepRight.png"));

    mpGenerationLabel = new QLabel(this);
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
    pInfoBoxLayout->addWidget(mpGenerationLabel);
    pInfoBoxLayout->addWidget(mpPreviousButton);
    pInfoBoxLayout->addWidget(mpNextButton);
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

    connect(mpColorBlob,               SIGNAL(clicked(bool)),  this,               SLOT(actiavateCurve(bool)));
    connect(mpPreviousButton,          SIGNAL(clicked(bool)),  mpParentPlotCurve,  SLOT(setPreviousGeneration()));
    connect(mpNextButton,              SIGNAL(clicked(bool)),  mpParentPlotCurve,  SLOT(setNextGeneration()));
    connect(pAutoUpdateCheckBox,       SIGNAL(toggled(bool)),  mpParentPlotCurve,  SLOT(setAutoUpdate(bool)));
    connect(pFrequencyAnalysisButton,  SIGNAL(clicked(bool)),  mpParentPlotCurve,  SLOT(performFrequencyAnalysis()));
    connect(pColorButton,              SIGNAL(clicked()),      mpParentPlotCurve,  SLOT(setLineColor()));
    connect(pScaleButton,              SIGNAL(clicked()),      mpParentPlotCurve,  SLOT(openScaleDialog()));
    connect(pCloseButton,              SIGNAL(clicked()),      mpParentPlotCurve,  SLOT(removeMe()));
    connect(pSizeSpinBox,    SIGNAL(valueChanged(int)),            mpParentPlotCurve, SLOT(setLineWidth(int)));
    connect(pLineStyleCombo, SIGNAL(currentIndexChanged(QString)), mpParentPlotCurve, SLOT(setLineStyle(QString)));
    connect(pLineSymbol,     SIGNAL(currentIndexChanged(QString)), mpParentPlotCurve, SLOT(setLineSymbol(QString)));

    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    if(mpParentPlotCurve->getCurveType() != PORTVARIABLE)
    {
        pAutoUpdateCheckBox->setDisabled(true);
        mpNextButton->setDisabled(true);
        mpPreviousButton->setDisabled(true);
        pFrequencyAnalysisButton->setDisabled(true);
    }
}

void PlotCurveInfoBox::setLineColor(const QColor color)
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
void PlotCurveInfoBox::updateInfo()
{
    // Enable/diable generation buttons
    const int lowGen = mpParentPlotCurve->getLogDataVariablePtr()->getLowestGeneration();
    const int highGen = mpParentPlotCurve->getLogDataVariablePtr()->getHighestGeneration();
    const int gen = mpParentPlotCurve->getGeneration();
    const int nGen = mpParentPlotCurve->getLogDataVariablePtr()->getNumGenerations();
    mpPreviousButton->setEnabled( (gen > lowGen) && (nGen > 1) );
    mpNextButton->setEnabled( (gen < highGen) && ( nGen > 1) );

    // Set generation number strings
    QString numString1, numString2, numString3;
    numString1.setNum(gen+1);
    //! @todo this will show strange when we have deleted old generations, maybe we should reassign all generations when we delete old data (costly)
    numString2.setNum(lowGen+1);
    numString3.setNum(highGen+1);
    mpGenerationLabel->setText(numString1 + " (" + numString2 + "," + numString3 + ")");

    // Update curve name
    refreshTitle();
}

void PlotCurveInfoBox::refreshTitle()
{
    mpTitle->setText(mpParentPlotCurve->getCurveName() + " ["+mpParentPlotCurve->getDataUnit()+"]");
}

void PlotCurveInfoBox::refreshActive(bool active)
{
    mpColorBlob->setChecked(active);
}

void PlotCurveInfoBox::actiavateCurve(bool active)
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
                     HopsanPlotID plotID,
                     HopsanPlotCurveType curveType)
{
    mHaveCustomData = false;
    mpData = pData;
    commonConstructorCode(axisY, parent, plotID, curveType);
}

//! @brief Consturctor for custom data
PlotCurve::PlotCurve(const SharedVariableDescriptionT &rVarDesc,
                     const QVector<double> &rXVector,
                     const QVector<double> &rYVector,
                     int axisY,
                     PlotTab *parent,
                     HopsanPlotID plotID,
                     HopsanPlotCurveType curveType)
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
                                      HopsanPlotID plotID,
                                      HopsanPlotCurveType curveType)
{
    mpCurveSymbol = 0;
    mIsActive = false;
    mCurveType = curveType;
    mpParentPlotTab = parent;

    QString dataUnit = mpData->getDataUnit();
    if(dataUnit.isEmpty())
    {
        dataUnit = gConfig.getDefaultUnit(mpData->getDataName());   //Apply default unit if not specified
    }

    mAxisY = axisY;
    mAutoUpdate = true;
    mScaleX = 1.0;
    mScaleY = 1.0;

    //! @todo FIX /Peter (should not be here)
    mOffsetX = mpData->mAppliedTimeOffset;
    mOffsetY = mpData->mAppliedValueOffset;

    //Create the actual curve
    mpQwtPlotCurve = new HopQwtPlotCurve(this->getCurveName());
    updateCurve();
    mpQwtPlotCurve->setRenderHint(QwtPlotItem::RenderAntialiased);
    mpQwtPlotCurve->setYAxis(axisY);
    mpQwtPlotCurve->attach(parent->getPlot(plotID));


    //Create the plot info box
    mpPlotCurveInfoBox = new PlotCurveInfoBox(this, mpParentPlotTab);
    mpPlotCurveInfoBox->setPalette(gConfig.getPalette());
    updatePlotInfoBox();

    // Maybe tab should add this instad of the curve istelf, and info box speak with curve
    mpParentPlotTab->mpParentPlotWindow->mpPlotCurveInfoLayout->addWidget(mpPlotCurveInfoBox);

    if(curveType != PORTVARIABLE)
    {
        setAutoUpdate(false);
    }

    //! @todo for now allways create a legend (wheter it is visible or not is an
    mpQwtPlotCurve->setItemAttribute(QwtPlotItem::Legend, true);

    //Create connections

    connect(mpParentPlotTab->mpParentPlotWindow->getPlotTabWidget(), SIGNAL(currentChanged(int)), this, SLOT(updatePlotInfoVisibility()));
    ////connect(mpParentPlotTab->mpParentPlotWindow->mpShowCurveInfoButton, SIGNAL(toggled(bool)), SLOT(updatePlotInfoVisibility()));
    connect(gpMainWindow->mpProjectTabs->getCurrentTab(),SIGNAL(simulationFinished()),this,SLOT(updateToNewGeneration()));
    connect(gpMainWindow->mpProjectTabs,SIGNAL(simulationFinished()),this,SLOT(updateToNewGeneration()));


    //! @todo FIXA /Peter
    //connect(mpContainerObject, SIGNAL(objectDeleted()), this, SLOT(removeMe()));
    //connect(mpContainerObject, SIGNAL(objectDeleted()), mpParentPlotTab->mpParentPlotWindow, SLOT(closeIfEmpty()), Qt::UniqueConnection);
    //connect(mpContainerObject->getModelObject(mComponentName), SIGNAL(objectDeleted()), this, SLOT(removeMe()));
    //connect(mpContainerObject->getModelObject(mComponentName), SIGNAL(nameChanged()), this, SLOT(removeMe()));
    //connect(mpContainerObject, SIGNAL(connectorRemoved()), this, SLOT(removeIfNotConnected()));


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
    if(mCurveType == PORTVARIABLE)
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
    else if(mCurveType == FREQUENCYANALYSIS)
        return "Frequency Spectrum";
    else if(mCurveType == NYQUIST)
        return "Nyquist Plot";
    else if(mCurveType == BODEGAIN)
        return "Magnitude Plot";
    else if(mCurveType == BODEPHASE)
        return "Phase Plot";
    else
        return "Unnamed Curve";
}


//! @brief Returns the type of the curve
HopsanPlotCurveType PlotCurve::getCurveType()
{
    return mCurveType;
}


//! @brief Returns a pointer to the actual Qwt curve in a plot curve object
HopQwtPlotCurve *PlotCurve::getQwtPlotCurvePtr()
{
    return mpQwtPlotCurve;
}


////! @brief Returns a pointer to the plot info dock of a plot curve
//QDockWidget *PlotCurve::getPlotInfoDockWidget()
//{
//    return mpPlotInfoDockWidget;
//}


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
    return mpData->getDataUnit();
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
const QVector<double> &PlotCurve::getDataVector() const
{
    return mpData->mDataVector;
}


//! @brief Returns the (unscaled) time vector of a plot curve
//! This returns the TIME vector, NOT any special X-axes if they are used.
const QVector<double> &PlotCurve::getTimeVector() const
{
    return *(mpData->mSharedTimeVectorPtr.data());
}


//! @brief Sets the generation of a plot curve
//! Updates the data to specified generation, and updates plot info box.
//! @param genereation Genereation to use
void PlotCurve::setGeneration(int generation)
{
    SharedLogVariableDataPtrT pNewData = mpData->getLogDataHandler()->getPlotData(mpData->getFullVariableName(), generation);
    if (pNewData)
    {
        mpData = pNewData;
    }

    //! @todo should not all updates happen automatically from one command
    mpParentPlotTab->rescaleToCurves();
    mpParentPlotTab->update();
    updateCurve();
    updatePlotInfoBox();

    //! @todo FIXA What about special X-axis /Peter
    //    mGeneration = generation;
    //    mDataVector = mpContainerObject->getPlotDataPtr()->getPlotDataValues(mGeneration, mComponentName, mPortName, mDataName);
    //    if(mpParentPlotTab->mVectorX.size() == 0)
    //        mTimeVector = mpContainerObject->getPlotDataPtr()->getTimeVector(mGeneration);
    //    else
    //    {
    //        mpParentPlotTab->mVectorX = mpContainerObject->getPlotDataPtr()->getPlotDataValues(mGeneration, mpParentPlotTab->mVectorXComponent,
    //                                                                   mpParentPlotTab->mVectorXPortName, mpParentPlotTab->mVectorXDataName);
    //        mTimeVector = mpParentPlotTab->mVectorX;
    //    }
}


//! @brief Sets the unit of a plot curve
//! @param unit Name of new unit
void PlotCurve::setDataUnit(QString unit)
{
    //! @todo FIXA /Peter
    //mDataUnit = unit;
    updateCurve();
    mpParentPlotTab->updateLabels();
    mpParentPlotTab->rescaleToCurves();
    mpParentPlotTab->update();
}


//! @brief Sets the scaling of a plot curve
//! @param scaleX Scale factor for X-axis
//! @param scaleY Scale factor for Y-axis
//! @param offsetX Offset value for X-axis
//! @param offsetY Offset value for Y-axis
void PlotCurve::setScaling(double scaleX, double scaleY, double offsetX, double offsetY)
{
    mScaleX=scaleX;
    mScaleY=scaleY;
    mOffsetX=offsetX;
    mOffsetY=offsetY;
    updateCurve();
}


void PlotCurve::setCustomData(const VariableDescription &rVarDesc, const QVector<double> &rvTime, const QVector<double> &rvData)
{
    //First disconnect all signals from the old data
    this->disconnect(mpData.data());

    //If we already have custom data, then delete it from memory as it is being replaced
    deleteCustomData();

    //Create new custom data
    SharedVariableDescriptionT pVarDesc = SharedVariableDescriptionT(new VariableDescription(rVarDesc));
    LogVariableContainer *pDataContainer = new LogVariableContainer(pVarDesc, 0);
    pDataContainer->addDataGeneration(0, rvTime, rvData);
    mHaveCustomData = true;
    mpData = pDataContainer->getDataGeneration(0);

    //Connect signals
    connectDataSignals();

    updateCurve();
}


//! @brief Converts the plot curve to its frequency spectrum by using FFT
void PlotCurve::toFrequencySpectrum()
{
    QVector<double> timeVec, dataVec;
    timeVec = *(mpData->mSharedTimeVectorPtr.data());
    dataVec = mpData->mDataVector;

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
        if(mpParentPlotTab->mpParentPlotWindow->mpPowerSpectrumCheckBox->isChecked())
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
    updateCurve();

    varDesc.mDataName = "Frequency";
    varDesc.mDataUnit = "Hz";
    mpParentPlotTab->changeXVector(timeVec, varDesc);
    mpParentPlotTab->update();
    updatePlotInfoBox();
}

void PlotCurve::resetLegendSize()
{
    // For now hardcoded but maybe in the future be possible to select, (default 8x8 is to small to see difference between dashed and solid lines)
    mpQwtPlotCurve->setLegendIconSize(QSize(40,12));
}

SharedLogVariableDataPtrT PlotCurve::getLogDataVariablePtr()
{
    return mpData;
}


//! @brief Changes a curve to the previous available gneraetion of its data
void PlotCurve::setPreviousGeneration()
{
    setGeneration(getGeneration()-1);
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
    QPen tempPen = mpQwtPlotCurve->pen();
    // Add one pt extra width for active curves
    if (mIsActive)
    {
        tempPen.setWidth(lineWidth+1);
    }
    else
    {
        tempPen.setWidth(lineWidth);
    }
    mpQwtPlotCurve->setPen(tempPen);
}


void PlotCurve::setLineStyle(QString lineStyle)
{
    mLineStyle = lineStyle;
    mpQwtPlotCurve->setStyle(HopQwtPlotCurve::Lines); //Assume we want lines
    QPen tempPen = mpQwtPlotCurve->pen();
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
        mpQwtPlotCurve->setStyle(HopQwtPlotCurve::NoCurve);
    }
    mpQwtPlotCurve->setPen(tempPen);
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

    QPen tempPen = mpQwtPlotCurve->pen();
    tempPen.setStyle(Qt::SolidLine);
    mpCurveSymbol->setPen(tempPen);
    mpCurveSymbol->setSize(8,8);
    mpQwtPlotCurve->setSymbol(mpCurveSymbol);

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
    tempPen = mpQwtPlotCurve->pen();
    tempPen.setColor(color);
    mpQwtPlotCurve->setPen(tempPen);

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
        color = QColorDialog::getColor(mpQwtPlotCurve->pen().color(), gpMainWindow);
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
    mpXScaleSpinBox->setRange(-DBLMAX, DBLMAX);
    mpXScaleSpinBox->setDecimals(10);
    mpXScaleSpinBox->setSingleStep(0.1);
    mpXScaleSpinBox->setValue(mScaleX);

    QLabel *pXOffsetLabel = new QLabel("Time Axis Offset: ", pScaleDialog);
    mpXOffsetSpinBox = new QDoubleSpinBox(pScaleDialog);
    mpXOffsetSpinBox->setDecimals(10);
    mpXOffsetSpinBox->setRange(-DBLMAX, DBLMAX);
    mpXOffsetSpinBox->setSingleStep(0.1);
    mpXOffsetSpinBox->setValue(mOffsetX);

    QLabel *pYScaleLabel = new QLabel("Y-Axis Scale: ", pScaleDialog);
    mpYScaleSpinBox = new QDoubleSpinBox(pScaleDialog);
    mpYScaleSpinBox->setSingleStep(0.1);
    mpYScaleSpinBox->setDecimals(10);
    mpYScaleSpinBox->setRange(-DBLMAX, DBLMAX);
    mpYScaleSpinBox->setValue(mScaleY);

    QLabel *pYOffsetLabel = new QLabel("Y-Axis Offset: ", pScaleDialog);
    mpYOffsetSpinBox = new QDoubleSpinBox(pScaleDialog);
    mpYOffsetSpinBox->setDecimals(10);
    mpYOffsetSpinBox->setRange(-DBLMAX, DBLMAX);
    mpYOffsetSpinBox->setSingleStep(0.1);
    mpYOffsetSpinBox->setValue(mOffsetY);

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
    mpParentPlotTab->rescaleToCurves();
}


//! @brief Shows or hides plot info dock
//! Changes visibility depending on whether or not the tab is currently open, and whether or not the hide plot info dock setting is activated.
void PlotCurve::updatePlotInfoVisibility()
{
    if(mpParentPlotTab == mpParentPlotTab->mpParentPlotWindow->getCurrentPlotTab() && mpParentPlotTab->mpParentPlotWindow->mpShowCurveInfoButton->isChecked())
    {
        // mpParentPlotTab->mpParentPlotWindow->mpPlotInfoWidget->show();
        mpPlotCurveInfoBox->show();
        //mpParentPlotTab->mpParentPlotWindow->addDockWidget(Qt::BottomDockWidgetArea, mpPlotInfoDockWidget, Qt::Vertical);
        mpParentPlotTab->mpParentPlotWindow->mpPlotCurveInfoLayout->addWidget(mpPlotCurveInfoBox);

    }
    else
    {
        //mpParentPlotTab->mpParentPlotWindow->mpPlotInfoLayout->removeWidget(mpPlotInfoBox);
        //mpParentPlotTab->mpParentPlotWindow->mpPlotInfoScrollArea->hide();
        //mpParentPlotTab->mpParentPlotWindow->mpPlotInfoWidget->hide();
        //! @todo FIXA /Peter
        mpPlotCurveInfoBox->hide();
        //                if(mpParentPlotTab->mpParentPlotWindow->mpPlotInfoLayout->isEmpty())
        //                {
        // mpParentPlotTab->mpParentPlotWindow->mpPlotInfoWidget->hide();
        //                    //mpParentPlotTab->mpParentPlotWindow->mpPlotInfoScrollArea;
        //                }
        mpParentPlotTab->mpParentPlotWindow->mpPlotCurveInfoLayout->removeWidget(mpPlotCurveInfoBox);

        //mpParentPlotTab->mpParentPlotWindow->mpPlotInfoWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    }
}


//! @brief Tells the parent plot tab of a curve to remove it
void PlotCurve::removeMe()
{
    mpParentPlotTab->removeCurve(this);
}


//! @brief Slot that checks that the plotted port is still connected, and removes the curve if not
void PlotCurve::removeIfNotConnected()
{
    //! @todo FiXA Peter
    //    if(!mpContainerObject->getModelObject(mComponentName)->getPort(mPortName)->isConnected())
    //    {
    //        removeMe();
    //    }
}

//! @brief Updates a plot curve to the most recent available generation of its data
void PlotCurve::updateToNewGeneration()
{
    if(mAutoUpdate)     //Only change the generation if auto update is on
        setGeneration(-1);
    updatePlotInfoBox();    //Update the plot info box regardless of auto update setting, to show number of available generations correctly
    mpParentPlotTab->rescaleToCurves();
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


//! @brief Updates the values of a curve
//! Updates a curve with regard to special X-axis, units and scaling.
//! @todo after updating from python, scale is not refreshed maybe this should be done in here
void PlotCurve::updateCurve()
{
    double unitScale = 1;
    if (gConfig.getCustomUnits(getDataName()).contains(getDataUnit()))
    {
        unitScale = gConfig.getCustomUnits(getDataName()).find(getDataUnit()).value();
    }

    QVector<double> tempX;
    QVector<double> tempY;
    if(mpParentPlotTab->mHasSpecialXAxis)
    {
        for(int i=0; i<mpParentPlotTab->mSpecialXVector.size() && i<mpData->mDataVector.size(); ++i)
        {
            tempX.append(mpParentPlotTab->mSpecialXVector[i]*mScaleX + mOffsetX);
            tempY.append(mpData->mDataVector[i]*unitScale*mScaleY + mOffsetY);
        }
    }
    else
    {
        for(int i=0; i<mpData->mSharedTimeVectorPtr->size() && i<mpData->mDataVector.size(); ++i)
        {
            tempX.append(mpData->mSharedTimeVectorPtr->at(i)*mScaleX + mOffsetX);
            tempY.append(mpData->mDataVector[i]*unitScale*mScaleY + mOffsetY);
        }
    }
    mpQwtPlotCurve->setSamples(tempX, tempY);
}

void PlotCurve::updateCurveName()
{
    if (mpData->getAliasName().isEmpty())
    {
        mpQwtPlotCurve->setTitle(mpData->getFullVariableNameWithSeparator(", "));
    }
    else
    {
        mpQwtPlotCurve->setTitle(mpData->getAliasName());
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
    connect(mpData.data(), SIGNAL(dataChanged()), this, SLOT(updateCurve()));
    connect(mpData.data(), SIGNAL(nameChanged()), this, SLOT(updateCurveName()));
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

//void PlotCurve::performSetAxis()
//{
//    mpParentPlotTab->mpParentPlotWindow->performSetAxis(this);
//}


//! @brief Constructor for plot markers
//! @param pCurve Pointer to curve the marker belongs to
//! @param pPlotTab Plot tab the marker is located in
//! @param markerSymbol The symbol the marker shall use
PlotMarker::PlotMarker(PlotCurve *pCurve, PlotTab *pPlotTab, QwtSymbol *markerSymbol)
    : QwtPlotMarker()
{
    mpCurve = pCurve;
    mpPlotTab = pPlotTab;
    mIsBeingMoved = false;
    mpMarkerSymbol = markerSymbol;
    setSymbol(mpMarkerSymbol);
    mIsMovable = true;
}


//! @brief Event filter for plot markers
//! This will interrupt events from plot canvas, to enable using mouse and key events for modifying markers.
//! @returns True if event was interrupted, false if its propagation shall continue
//! @param object Pointer to the object the event belongs to (in this case the plot canvas)
//! @param ev ent Event to be interrupted
bool PlotMarker::eventFilter(QObject */*object*/, QEvent *event)
{
    if(!mIsMovable)
        return false;

    // Mouse press events, used to initiate moving of a marker if mouse cursor is close enough
    if (event->type() == QEvent::MouseButtonPress)
    {
        QCursor cursor;
        QPointF midPoint;
        midPoint.setX(this->plot()->transform(QwtPlot::xBottom, value().x()));
        midPoint.setY(this->plot()->transform(QwtPlot::yLeft, value().y()));

        if(!mpPlotTab->mpZoomerLeft[FIRSTPLOT]->isEnabled() && !mpPlotTab->mpPanner[FIRSTPLOT]->isEnabled())
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
            mpMarkerSymbol->setPen(QPen(mpCurve->getQwtPlotCurvePtr()->pen().brush().color().lighter(165), 3));
            this->setSymbol(mpMarkerSymbol);
            this->plot()->replot();
            this->plot()->updateGeometry();
            retval=true;
        }
        else
        {
            if(!mIsBeingMoved)
            {
                mpMarkerSymbol->setPen(QPen(mpCurve->getQwtPlotCurvePtr()->pen().brush().color(), 3));
                this->setSymbol(mpMarkerSymbol);
                this->plot()->replot();
                this->plot()->updateGeometry();
            }
        }

        if(mIsBeingMoved)
        {
            double x = mpCurve->getQwtPlotCurvePtr()->sample(mpCurve->getQwtPlotCurvePtr()->closestPoint(this->plot()->canvas()->mapFromGlobal(cursor.pos()))).x();
            double y = mpCurve->getQwtPlotCurvePtr()->sample(mpCurve->getQwtPlotCurvePtr()->closestPoint(this->plot()->canvas()->mapFromGlobal(cursor.pos()))).y();
            setXValue(x);
            setYValue(this->plot()->invTransform(QwtPlot::yLeft, this->plot()->transform(mpCurve->getQwtPlotCurvePtr()->yAxis(), y)));

            QString xString;
            QString yString;
            xString.setNum(x);
            yString.setNum(y);
            QwtText tempLabel;
            tempLabel.setText("("+xString+", "+yString+")");
            tempLabel.setColor(mpCurve->getQwtPlotCurvePtr()->pen().brush().color());
            tempLabel.setBackgroundBrush(QColor(255,255,255,220));
            tempLabel.setFont(QFont("Calibri", 12, QFont::Normal));
            setLabel(tempLabel);
        }
        return retval;
    }

    //Mouse release event, will stop moving marker
    else if (event->type() == QEvent::MouseButtonRelease && mIsBeingMoved == true)
    {
        mIsBeingMoved = false;
        return false;
    }

    //!Keypress event, will delete marker if delete key is pressed
    else if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if(keyEvent->key() == Qt::Key_Delete)
        {
            QCursor cursor;
            QPointF midPoint;
            midPoint.setX(this->plot()->transform(QwtPlot::xBottom, value().x()));
            midPoint.setY(this->plot()->transform(mpCurve->getQwtPlotCurvePtr()->yAxis(), value().y()));
            if((this->plot()->canvas()->mapToGlobal(midPoint.toPoint()) - cursor.pos()).manhattanLength() < 35)
            {
                plot()->canvas()->removeEventFilter(this);
                for(int plotID=0; plotID<2; ++plotID)
                {
                    mpPlotTab->mMarkerPtrs[plotID].removeAll(this);     //Cycle all plots and remove the marker if it is found
                }
                this->hide();           // This will only hide and inactivate the marker. Deleting it seem to make program crash.
                this->detach();
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


//! @brief Returns a pointer to the curve a plot marker belongs to
PlotCurve *PlotMarker::getCurve()
{
    return mpCurve;
}



HopQwtPlotCurve::HopQwtPlotCurve(QString label) :
    QwtPlotCurve(label)
{
    //nothing for now
}

QList<QwtLegendData> HopQwtPlotCurve::legendData() const
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

