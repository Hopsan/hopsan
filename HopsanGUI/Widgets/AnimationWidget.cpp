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
//! @file   AnimationWidget.cpp
//! @author Pratik Deschpande <pratik661@gmail.com>
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2012-04-25
//!
//! @brief Contains a widget for showing animations
//!
//$Id$

//Qt includes
#include <QVector>
#include <QLineEdit>
#include <QGridLayout>
#include <QDialogButtonBox>
#include <QMessageBox>

//Hopsan includes
#include "global.h"
#include "AnimatedConnector.h"
#include "Configuration.h"
#include "GraphicsView.h"
#include "GUIConnector.h"
#include "GUIObjects/AnimatedComponent.h"
#include "GUIObjects/GUIContainerObject.h"
#include "GUIObjects/GUIWidgets.h"
#include "GUIPort.h"
#include "ModelHandler.h"
#include "ModelWidget.h"
#include "Widgets/AnimationWidget.h"
#include "Widgets/LibraryWidget.h"
#include "MainWindow.h"
#include "LibraryHandler.h"
#include "MessageHandler.h"
#include "SimulationThreadHandler.h"

//! @brief Constructor for the animation widget class
//! @param [in] parent Pointer to parent widget
AnimationWidget::AnimationWidget(QWidget *parent) :
    QWidget(parent)
{
    //Define public member pointer variables
    mpContainer = gpModelHandler->getCurrentViewContainerObject();
    mpAnimationData = mpContainer->getAppearanceData()->getAnimationDataPtr();
    mpCoreSimulationHandler = new CoreSimulationHandler();

    //Set palette
    this->setPalette(gpConfig->getPalette());

    //Create graphics scene
    mpGraphicsScene = new QGraphicsScene();
    mpGraphicsScene->setSceneRect(0,0,5000,5000);

    //Create graphics view
    GraphicsView *pOrgView = mpContainer->mpModelWidget->getGraphicsView();
    mpGraphicsView = new AnimatedGraphicsView(mpGraphicsScene,0); //!< @todo This graphics view is never deleted /Peter
    mpGraphicsView->setGeometry(0,0,500,500);
    mpGraphicsView->setInteractive(true);
    mpGraphicsView->setEnabled(true);
    mpGraphicsView->setAcceptDrops(false);
    mpGraphicsView->setViewportUpdateMode(QGraphicsView::MinimalViewportUpdate);
    mpGraphicsView->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    mpGraphicsView->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    mpGraphicsView->setCacheMode(QGraphicsView::CacheBackground);
    mpGraphicsView->setOptimizationFlags(QGraphicsView::DontSavePainterState | QGraphicsView::DontAdjustForAntialiasing);
    //mpGraphicsView->centerOn(mpGraphicsView->sceneRect().topLeft());
    mpGraphicsView->setZoomFactor(pOrgView->getZoomFactor());
    mpGraphicsView->setRenderHint(QPainter::Antialiasing, gpConfig->getBoolSetting(cfg::antialiasing));
    double X,Y,Z;
    pOrgView->getViewPort(X,Y,Z);
    mpGraphicsView->centerOn(X,Y+3/Z);

    connect(gpMainWindow->mpResetZoomAction,    SIGNAL(triggered()),    mpGraphicsView,    SLOT(resetZoom()),   Qt::UniqueConnection);
    connect(gpMainWindow->mpZoomInAction,       SIGNAL(triggered()),    mpGraphicsView,    SLOT(zoomIn()),      Qt::UniqueConnection);
    connect(gpMainWindow->mpZoomOutAction,      SIGNAL(triggered()),    mpGraphicsView,    SLOT(zoomOut()),     Qt::UniqueConnection);
    connect(gpMainWindow->mpCenterViewAction,   SIGNAL(triggered()),    mpGraphicsView,    SLOT(centerView()),  Qt::UniqueConnection);

    //Create control panel widgets
    mpTimeDisplay = new QLineEdit(this);
    mpTimeDisplay->setBaseSize(20,10);

    mpSettingsButton = new QToolButton(this);
    mpSettingsButton->setIcon(QIcon(QString(QString(ICONPATH) + "svg/Hopsan-Configure.svg")));

    mpStopButton = new QToolButton(this);
    mpStopButton->setIcon(QIcon(QString(QString(ICONPATH) + "svg/Hopsan-Stop.svg")));
    mpStopButton->setDisabled(true);

    mpRewindButton = new QToolButton(this);
    mpRewindButton->setIcon(QIcon(QString(QString(ICONPATH) + "svg/Hopsan-Rewind.svg")));

    mpPauseButton = new QToolButton(this);
    mpPauseButton->setIcon(QIcon(QString(QString(ICONPATH) + "svg/Hopsan-Pause.svg")));
    mpPauseButton->setDisabled(true);

    mpPlayButton = new QToolButton(this);
    mpPlayButton->setIcon(QIcon(QString(QString(ICONPATH) + "svg/Hopsan-Play.svg")));

    mpPlayRealTimeButton = new QToolButton(this);
    mpPlayRealTimeButton->setIcon(QIcon(QString(QString(ICONPATH) + "svg/Hopsan-PlayRealTime.svg")));

    mpCloseButton = new QToolButton(this);
    mpCloseButton->setIcon(QIcon(QString(ICONPATH)+"svg/Hopsan-Discard.svg"));

    QLabel *pTimeLabel = new QLabel(" Time:", this);
    QLabel *pSpeedLabel = new QLabel(" Speed:", this);

    mpTimeSlider = new QSlider(Qt::Horizontal);

    mpSpeedSpinBox = new QDoubleSpinBox(this);
    mpSpeedSpinBox->setRange(std::numeric_limits<double>::lowest(),std::numeric_limits<double>::max());
    mpSpeedSpinBox->setDecimals(5);
    mpSpeedSpinBox->setSingleStep(0.01);
    mpSpeedSpinBox->setValue(1.0);
    if (mpAnimationData)
    {
        mpSpeedSpinBox->setValue(mpAnimationData->animationSpeed);
    }

    //Create the layout and add widgets
    mpLayout= new QGridLayout(this);
    mpLayout->addWidget(mpSettingsButton,       0,  0);
    mpLayout->addWidget(mpStopButton,           0,  1);
    mpLayout->addWidget(mpPauseButton,          0,  2);
    mpLayout->addWidget(mpPlayButton,           0,  3);
    mpLayout->addWidget(mpPlayRealTimeButton,   0,  4);
    mpLayout->addWidget(mpRewindButton,         0,  5);
    mpLayout->addWidget(pSpeedLabel,            0,  6);
    //mpLayout->addWidget(mpSpeedSlider,          0,  7);
    mpLayout->addWidget(mpSpeedSpinBox,         0,  7);
    mpLayout->addWidget(pTimeLabel,             0,  8);
    mpLayout->addWidget(mpTimeSlider,           0,  9);
    mpLayout->addWidget(mpTimeDisplay,          0,  10);
    mpLayout->addWidget(mpCloseButton,          0,  11);
    mpLayout->addWidget(mpGraphicsView,         1,  0,  1,  12);
    mpLayout->setColumnStretch(9,1);
    mpLayout->setRowStretch(1,1);
    this->setLayout(mpLayout);

    //Create the timer object
    mpAnimationStepTrigger = new QTimer(this);
    mpFpsAdjustClock = new QTime();

    //Set default values for animation variables
    mRealTime = false;
    mCurrentAnimationTime = 0;
    mLastAnimationTime = 0;
    mSimulationSpeed = mpSpeedSpinBox->value();
    mTimeStep = gpModelHandler->getCurrentTopLevelSystem()->getTimeStep(); //! @todo This is not used, but it should be
    mFps=60;   //Frames per second

    mIntensityMaxMap.insert("NodeHydraulic", mpAnimationData->hydraulicMaxPressure);
    mIntensityMinMap.insert("NodeHydraulic", mpAnimationData->hydraulicMinPressure);
    mFlowSpeedMap.insert("NodeHydraulic",mpAnimationData->flowSpeed);

    mHydraulicIntensityMax = mpAnimationData->hydraulicMaxPressure;
    mHydraulicIntensityMin = mpAnimationData->hydraulicMinPressure;
    mHydraulicSpeed = mpAnimationData->flowSpeed;

    mpPlotData = mpContainer->getLogDataHandler().data();
    mpPlayButton->setDisabled(mpPlotData->isEmpty());
    mpRewindButton->setDisabled(mpPlotData->isEmpty());

    if(!mpPlotData->isEmpty() && !mpPlotData->getTimeVectorVariable(-1).isNull())
    {
        //Obtain time values from plot data
        mTimeValues = mpContainer->getLogDataHandler()->getTimeVectorVariable(-1)->getDataVectorCopy();

        //Calculate total simulation time and number of samples
        mTotalTime = mTimeValues.last();
        mnSamples = mTimeValues.size();

        //Set min, max and step values for time slider
        mpTimeSlider->setMinimum(0);
        mpTimeSlider->setMaximum(mTimeValues.size());
        mpTimeSlider->setSingleStep(mTimeValues.size()/mFps);
    }
    else
    {
        mpTimeSlider->setMinimum(0);
        mpTimeSlider->setMaximum(0);
    }

    QStringList hiddenComponents;
    hiddenComponents << "SignalInputInterface" << "SignalOutputInterface";

    //Generate list of model objects from container object
    QStringList modelObjectNames = mpContainer->getModelObjectNames();
    for(int i=0; i<modelObjectNames.size(); ++i)
    {
        if(!hiddenComponents.contains(mpContainer->getModelObject(modelObjectNames.at(i))->getTypeName()))
        {
            mModelObjectsList.append(mpContainer->getModelObject(modelObjectNames.at(i)));
        }
    }

    //Generate list of connectors from container object
    for(int d=0;d<mModelObjectsList.size();d++)
    {
        QList<Connector*> moConnectors = mModelObjectsList.at(d)->getConnectorPtrs();
        for(int e=0;e<moConnectors.size();e++)
        {
            Connector* tempConnector = moConnectors.at(e);
            if(tempConnector->isConnected() &&
               !mConnectorList.contains(tempConnector) &&
               !hiddenComponents.contains(tempConnector->getStartPort()->getParentModelObject()->getTypeName()) &&
               !hiddenComponents.contains(tempConnector->getEndPort()->getParentModelObject()->getTypeName()))
            {
                mConnectorList.append(tempConnector);
            }
        }
    }

    //Create animated components from components list
    for(ModelObject* pMO : mModelObjectsList)
    {
        AnimatedComponent *pAnimatedComponent;
        if (pMO->getTypeName() == HOPSANGUISCOPECOMPONENTTYPENAME) {
            pAnimatedComponent = new AnimatedScope(pMO, this);
        }
        else {
            pAnimatedComponent = new AnimatedComponent(pMO, this);
        }
        mAnimatedComponentList.append(pAnimatedComponent);
    }

    //Create animated connectors from connectors list
    for(int f=0;f<mConnectorList.size();f++)
    {
        AnimatedConnector *pAnimatedConnector = new AnimatedConnector(mConnectorList.at(f), this);
        mpGraphicsScene->addItem(pAnimatedConnector);
        mAnimatedConnectorList.append(pAnimatedConnector);
    }

    //Generate list of widgets from container object
    QList<Widget*> widgets = mpContainer->getWidgets();
    for(int i=0; i<widgets.size(); ++i)
    {
        TextBoxWidget *pWidget = new TextBoxWidget(*qobject_cast<TextBoxWidget*>(widgets[i]), mpContainer);
        //pWidget->setParent(this);
        mpGraphicsScene->addItem(pWidget);
        pWidget->setPos(widgets[i]->pos());
    }

    //Define button connections
    connect(mpSettingsButton,       SIGNAL(clicked()),          this,   SLOT(openPreferencesDialog()));
    connect(mpRewindButton,         SIGNAL(clicked()),          this,   SLOT(rewind()));
    connect(mpPlayButton,           SIGNAL(clicked()),          this,   SLOT(play()));
    connect(mpPlayRealTimeButton,   SIGNAL(clicked()),          this,   SLOT(playRT()));
    connect(mpPauseButton,          SIGNAL(clicked()),          this,   SLOT(pause()));
    connect(mpStopButton,           SIGNAL(clicked()),          this,   SLOT(stop_reset()));
    connect(mpCloseButton,          SIGNAL(pressed()),          gpModelHandler->getCurrentModel(), SLOT(closeAnimation()));

    //Define slider connections
    connect(mpTimeSlider,           SIGNAL(sliderPressed()),    this,   SLOT(pause()));
    connect(mpTimeSlider,           SIGNAL(valueChanged(int)),  this,   SLOT(changeIndex(int)));
    connect(mpTimeSlider,           SIGNAL(sliderMoved(int)),   this,   SLOT(updateMovables()));
    connect(mpSpeedSpinBox,         SIGNAL(valueChanged(double)), this, SLOT(changeSpeed(double)));

    //Connect timer with update function
    connect(mpAnimationStepTrigger,                SIGNAL(timeout()),          this,   SLOT(updateAnimation()));
}


//! @brief Destructor for animation widget class
AnimationWidget::~AnimationWidget()
{
    stop();
    delete mpFpsAdjustClock;

    //Make sure any changes made in zoom and position are transfered back to original graphics view
    GraphicsView *pOrgView = mpContainer->mpModelWidget->getGraphicsView();
    if(pOrgView)
    {
        pOrgView->setZoomFactor(mpGraphicsView->getZoomFactor());
        double X,Y,Z;
        mpGraphicsView->getViewPort(X,Y,Z);
        pOrgView->centerOn(X,Y+33/Z);
    }
}


void AnimationWidget::openPreferencesDialog()
{
    QDialog *pDialog = new QDialog(this);
    pDialog->setWindowTitle("Animation Preferences");

    QLabel *pFpsLabel = new QLabel("Frames per second: ", pDialog);
    QLineEdit *pFpsLineEdit = new QLineEdit(QString::number(mFps), pDialog);
    pFpsLineEdit->setValidator(new QIntValidator);

    QLabel *pLowPressureLabel = new QLabel("Low pressure (hydraulic): ", pDialog);
    QLineEdit *pLowPressureLineEdit = new QLineEdit(QString::number(mIntensityMinMap.find("NodeHydraulic").value()), pDialog);
    pLowPressureLineEdit->setValidator(new QDoubleValidator);

    QLabel *pHighPressureLabel = new QLabel("High pressure (hydraulic): ", pDialog);
    QLineEdit *pHighPressureLineEdit = new QLineEdit(QString::number(mIntensityMaxMap.find("NodeHydraulic").value()), pDialog);
    pHighPressureLineEdit->setValidator(new QDoubleValidator);

    QLabel *pFlowSpeedLabel = new QLabel("Flow speed (hydraulic): ", pDialog);
    QLineEdit *pFlowSpeedLineEdit = new QLineEdit(QString::number(mFlowSpeedMap.find("NodeHydraulic").value()), pDialog);
    pFlowSpeedLineEdit->setValidator(new QDoubleValidator);

    QPushButton *pCancelButton = new QPushButton("Cancel", pDialog);
    QPushButton *pOkButton = new QPushButton("Ok", pDialog);
    QPushButton *pResetAllButton = new QPushButton("Reset all", pDialog);
    QPalette palette = gpConfig->getPalette();
    palette.setColor(QPalette::Text,Qt::darkRed);
    pResetAllButton->setPalette(palette);

    QDialogButtonBox *pButtonBox = new QDialogButtonBox(pDialog);
    pButtonBox->addButton(pResetAllButton, QDialogButtonBox::ActionRole);
    pButtonBox->addButton(pCancelButton, QDialogButtonBox::RejectRole);
    pButtonBox->addButton(pOkButton, QDialogButtonBox::AcceptRole);


    QGridLayout *pLayout = new QGridLayout(pDialog);
    pLayout->addWidget(pFpsLabel,               0, 0);
    pLayout->addWidget(pFpsLineEdit,            0, 1);
    pLayout->addWidget(pLowPressureLabel,       1, 0);
    pLayout->addWidget(pLowPressureLineEdit,    1, 1);
    pLayout->addWidget(pHighPressureLabel,      2, 0);
    pLayout->addWidget(pHighPressureLineEdit,   2, 1);
    pLayout->addWidget(pFlowSpeedLabel,         3, 0);
    pLayout->addWidget(pFlowSpeedLineEdit,      3, 1);
    pLayout->addWidget(pButtonBox,              4, 0, 1, 2);

    pDialog->setLayout(pLayout);

    connect(pResetAllButton, SIGNAL(clicked()), this, SLOT(resetAllAnimationDataToDefault()));
    connect(pCancelButton,  SIGNAL(clicked()), pDialog, SLOT(reject()));
    connect(pOkButton,      SIGNAL(clicked()), pDialog, SLOT(accept()));

    if(pDialog->exec() == QDialog::Accepted)
    {
        if(pFlowSpeedLineEdit->text().toDouble() != mpAnimationData->flowSpeed)
        {
            mpContainer->mpModelWidget->hasChanged();
            mpAnimationData->flowSpeed = pFlowSpeedLineEdit->text().toDouble();
        }
        if(pLowPressureLineEdit->text().toDouble() != mpAnimationData->hydraulicMinPressure)
        {
            mpContainer->mpModelWidget->hasChanged();
            mpAnimationData->hydraulicMinPressure = pLowPressureLineEdit->text().toDouble();
        }
        if(pHighPressureLineEdit->text().toDouble() != mpAnimationData->hydraulicMaxPressure)
        {
            mpContainer->mpModelWidget->hasChanged();
            mpAnimationData->hydraulicMaxPressure = pHighPressureLineEdit->text().toDouble();
        }

        mFps = pFpsLineEdit->text().toInt();
        mIntensityMinMap.insert("NodeHydraulic", pLowPressureLineEdit->text().toDouble());
        mIntensityMaxMap.insert("NodeHydraulic", pHighPressureLineEdit->text().toDouble());
        mFlowSpeedMap.insert("NodeHydraulic", pFlowSpeedLineEdit->text().toDouble());

        mHydraulicIntensityMin = pLowPressureLineEdit->text().toDouble();
        mHydraulicIntensityMax = pHighPressureLineEdit->text().toDouble();
        mHydraulicSpeed = pFlowSpeedLineEdit->text().toDouble();
    }

    delete(pDialog);
}


//! @brief Slot that stops the animation
void AnimationWidget::stop_reset()
{
    stop();
    mpTimeSlider->setValue(0);
}


//! @brief Slot that rewinds the animation (with full speed)
//! @todo This function does not work very well, if you click rewind, after simulation is complete, then it will jsut set speed = -1 and it wont actually rewind
void AnimationWidget::rewind()
{
    mRealTime=false;
    // This should also trigger the actual update slot
    mpSpeedSpinBox->setValue(-1);
}


//! @brief Slot that pauses the animation
void AnimationWidget::pause()
{
    mpPlayButton->setDisabled(mpPlotData->isEmpty());
    mpRewindButton->setDisabled(mpPlotData->isEmpty());
    mpAnimationStepTrigger->stop();
}


//! @brief Slot that plays the animation
void AnimationWidget::play()
{
    mRealTime=false;
    // Stop animation timer if speed is zero (it shouldn't be running for no reason)
    if(mSimulationSpeed == 0) {
        mpAnimationStepTrigger->stop();
    }
    // Start the timer with correct FPS, if it is not already started
    else if (!mpAnimationStepTrigger->isActive()) {
        mFps = 60;
        mLastFpsAdjustTime = mpTimeDisplay->text().toDouble()*1000;
        mpAnimationStepTrigger->start(1000.0/double(mFps));  //Timer object uses milliseconds
        mpFpsAdjustClock->start();
        //mpTime->setHMS(0,0,0,mpTimeDisplay->text().toDouble()*1000);
    }

    mpPlayButton->setDisabled(true);
    mpPlayRealTimeButton->setDisabled(true);
    mpStopButton->setDisabled(false);
    mpPauseButton->setDisabled(false);
}


//! @brief Slot that plays the animation continuously in real-time
//! @todo The plot data object in container should perhaps be cleared of these generations
void AnimationWidget::playRT()
{
    if(mpSpeedSpinBox->value() < 0) {
        gpMessageHandler->addErrorMessage("Cannot play real-time animation with negative speed");
        return;
    }
    if(!mpContainer->getCoreSystemAccessPtr()->isSimulationOk())
    {
        gpMessageHandler->addErrorMessage("Could not start real-time animation, model is not OK");
        return;
    }
    if(!mpContainer->getCoreSystemAccessPtr()->initialize(0,10,0))
    {
        gpMessageHandler->addErrorMessage("Could not start real-time animation, model failed to initialize");
        return;
    }

    if(!mpContainer->mpModelWidget->startRealtimeSimulation(mpSpeedSpinBox->value())) {
        gpMessageHandler->addErrorMessage("Could not start real-time animation.");
        return;
    }

    mRealTime = true;
    mpTimeSlider->setValue(1);
    changeIndex(0);

    // Stop animation timer if speed is zero (it shouldn't be running for no reason)
    if(mSimulationSpeed == 0) {
        mpAnimationStepTrigger->stop();
    }
    //Start the timer with correct FPS, if it is not already started
    else if (!mpAnimationStepTrigger->isActive()) {
        mFps = 60;
        mLastFpsAdjustTime = mpTimeDisplay->text().toDouble()*1000;
        mpAnimationStepTrigger->start(1000.0/double(mFps));  //Timer object uses milliseconds
        mpFpsAdjustClock->start();
        //mpTime->setHMS(0,0,0,mpTimeDisplay->text().toDouble()*1000);
    }

    mpPauseButton->setDisabled(true);
    mpSpeedSpinBox->setDisabled(true);
    mpTimeSlider->setDisabled(true);
    mpPlayRealTimeButton->setDisabled(true);
    mpPlayButton->setDisabled(true);
    mpRewindButton->setDisabled(true);
    mpStopButton->setDisabled(false);
}


//! @brief Slot that changes animation speed
//! @param [in] newSpeed New speed for animation
void AnimationWidget::changeSpeed(double newSpeed)
{
    updateAnimationSpeed(newSpeed);
}


//! @brief Slot that changes time index of animation (used when time slider is moved)
void AnimationWidget::changeIndex(int newIndex)
{
    if(isRealTimeAnimation())
    {
        mIndex = newIndex;
        mLastAnimationTime = 0;
        mpTimeDisplay->setText(0);
    }
    else
    {
        mIndex = std::min(std::max(newIndex,0), mTimeValues.size()-1);
        mLastAnimationTime = mTimeValues.at(mIndex);
        mpTimeDisplay->setText(QString::number(mTimeValues.at(mIndex)));
    }
}


//! @brief Updates animation speed
void AnimationWidget::updateAnimationSpeed(const double speedScale)
{
    mSimulationSpeed = speedScale;
    if(mSimulationSpeed == 0)
    {
        // Stop animation timer if speed is zero (it shouldn't be running for no reason)
        mpAnimationStepTrigger->stop();
    }
//    else
//    {
//        //Start the timer with correct FPS, if it is not already started
//        if(!mpTimer->isActive())
//        {
//            mFps = 60;
//            mLastTimeCheck = mpTimeDisplay->text().toDouble()*1000;
//            mpTimer->start(1000.0/double(mFps));  //Timer object uses milliseconds
//            mpTime->start();
//            //mpTime->setHMS(0,0,0,mpTimeDisplay->text().toDouble()*1000);
//        }
//    }

    // We remember the animation speed set for this container, in case we reopen the animation widget again
    if (mpAnimationData)
    {
        mpAnimationData->animationSpeed = mSimulationSpeed;
    }
}


//! @brief Slot that updates the animation, called by animation timer object
void AnimationWidget::updateAnimation()
{
    // Real-time simulation
    if(mRealTime)
    {
        //Update last animation time
        mLastAnimationTime = mpContainer->getCoreSystemAccessPtr()->getCurrentTime();
        mpTimeDisplay->setText(QString::number(mLastAnimationTime));

        //Update animated connectors and components
        updateMovables();

        // Auto adjust fps
        adjustFps();
    }
    // Not real-time simulation (and we have time data)
    else if(!mTimeValues.isEmpty())
    {
        //Calculate animation time (with limitations)
        mCurrentAnimationTime = mLastAnimationTime+mSimulationSpeed/double(mFps);
        mCurrentAnimationTime = std::min(mTotalTime, std::max(0.0, mCurrentAnimationTime));
        mLastAnimationTime = mCurrentAnimationTime;

        //Calculate index for time slider (with limitations)
        mIndex = static_cast<int>(mnSamples*(mCurrentAnimationTime/mTotalTime));
        mIndex = std::min(mTimeValues.size()-1, std::max(0, mIndex));
        mpTimeSlider->setValue(mIndex);
        mpTimeDisplay->setText(QString::number(mTimeValues.at(mIndex)));

        //Update animated connectors and components
        updateMovables();

        // Detect if the end of playback has been reached
        if(mIndex == mTimeValues.size()-1) {
            stop();
        }
        else {
            adjustFps();
        }
    }
}


//! @brief Help function that updates all animated components and connectors
void AnimationWidget::updateMovables()
{
   //Update animated components
   for(int c=0; c<mAnimatedComponentList.size(); ++c)
   {
       mAnimatedComponentList[c]->updateAnimation();
   }

   //Update animated connectors
   for(int c=0; c<mAnimatedConnectorList.size(); ++c)
   {
       mAnimatedConnectorList[c]->updateAnimation();
   }

   mpGraphicsView->update();
}


//! @brief Resets animation data in all animated components to default (but warns user first)
void AnimationWidget::resetAllAnimationDataToDefault()
{
    int ret = QMessageBox::question(this, "Warning!", "This will reset animation data for all components to default values. Continue?",
                                    QMessageBox::Ok, QMessageBox::Cancel);

    if(ret == QMessageBox::Cancel) return;

    for(int c=0; c<mAnimatedComponentList.size(); ++c)
    {
        AnimatedComponent *pComp = mAnimatedComponentList.at(c);

        QDomDocument domDocument;
        QDomElement animationRoot = domDocument.createElement(hmf::animation);
        domDocument.appendChild(animationRoot);

        QString subTypeName = pComp->mpModelObject->getSubTypeName();
        QString typeName = pComp->mpModelObject->getTypeName();
        SharedModelObjectAppearanceT pAppearanceData = gpLibraryHandler->getModelObjectAppearancePtr(typeName, subTypeName);
        pAppearanceData->getAnimationDataPtr()->saveToDomElement(animationRoot);
        QString baseIconPath = gpLibraryHandler->getModelObjectAppearancePtr(pComp->mpModelObject->getTypeName())->getAnimationDataPtr()->baseIconPath;

        //Store icon paths (they are not included in saveToDomElement() )
        QStringList iconPaths;
        for(const ModelObjectAnimationMovableData &m : pComp->getAnimationDataPtr()->movables) {
            iconPaths << m.iconPath;
        }

        //! @todo Maybe more things are not included in saveToDomElement(), make sure they are added here...

        pComp->getAnimationDataPtr()->movables.clear();
        pComp->getAnimationDataPtr()->readFromDomElement(animationRoot,baseIconPath);

        //Restore icon paths
        for(int m=0; m<iconPaths.size(); ++m)
        {
            pComp->getAnimationDataPtr()->movables[m].iconPath = iconPaths[m];
        }
    }
}

//! @brief Auto-adjust FPS
void AnimationWidget::adjustFps()
{
    const double dT = mpFpsAdjustClock->elapsed();
    // Only do this every .1 seconds
    if(dT > 100)
    {
        const int currentTime_ms = static_cast<int>(mpTimeDisplay->text().toDouble()*1000.0);
        if(currentTime_ms-mLastFpsAdjustTime < 0.95*dT)
        {
            mFps = qMax(10, int(mFps*0.8));    //Too slow, decrease FPS
        }
        else
        {
            mFps = qMin(100, int(mFps*1.11));  //Not too slow, increase FPS slightly
        }
        mpAnimationStepTrigger->start(static_cast<int>(1000.0/double(mFps)));  //Set timer interval
        mLastFpsAdjustTime = currentTime_ms;                    //Store last check time
        mpFpsAdjustClock->restart();                            //Restart speed check counter
    }
}

void AnimationWidget::stop()
{
    if(mRealTime) {
        mpContainer->mpModelWidget->stopRealtimeSimulation();
    }
    mLastAnimationTime = 0.0;
    mRealTime=false;
    mpAnimationStepTrigger->stop();

    mpPauseButton->setDisabled(true);
    mpSpeedSpinBox->setDisabled(false);
    mpTimeSlider->setDisabled(false);
    mpPlayRealTimeButton->setDisabled(false);
    mpPlayButton->setDisabled(mpPlotData->isEmpty());
    mpRewindButton->setDisabled(mpPlotData->isEmpty());
}


//! @brief Tells whether or not the current animation is a real-time animation
bool AnimationWidget::isRealTimeAnimation()
{
    return mRealTime;
}


//! @brief Returns last (not current) animation time
double AnimationWidget::getLastAnimationTime()
{
    return mLastAnimationTime;
}


AnimatedComponent *AnimationWidget::getAnimatedComponent(QString name)
{
    for(int c=0; c<mAnimatedComponentList.size(); ++c)
    {
        if(mAnimatedComponentList.at(c)->mpModelObject->getName() == name)
        {
            return mAnimatedComponentList.at(c);
        }
    }

    return 0;
}

void AnimationWidget::disablePlayback()
{
    mpPlayButton->setDisabled(true);
    mpPlayRealTimeButton->setDisabled(true);
    mpSettingsButton->setDisabled(true);
    mpStopButton->setDisabled(true);
    mpRewindButton->setDisabled(true);
    mpPauseButton->setDisabled(true);
    mpTimeSlider->setDisabled(true);
    mpSpeedSpinBox->setDisabled(true);
    mpTimeDisplay->setDisabled(true);
}


//! @brief Returns a pointer to the graphics scene
QGraphicsScene* AnimationWidget::getGraphicsScene()
{
    return this->mpGraphicsScene;
}


//! @brief Returns a pointer to the plot data object
LogDataHandler2* AnimationWidget::getPlotDataPtr()
{
    return mpPlotData;
}



//! @brief Returns current time index of animation
int AnimationWidget::getIndex()
{
 return mIndex;
}


//! @brief Returns last time index of animation
int AnimationWidget::getLastIndex()
{
 return mTimeValues.size()-1;
}

 void AnimationWidget::closeEvent(QCloseEvent *event)
 {
     this->stop_reset();
     //delete mpParent->mpCentralTabs->getCurrentTab()->mpAnimationWidget;
     event->accept();
 }
