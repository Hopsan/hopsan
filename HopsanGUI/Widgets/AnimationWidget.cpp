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
#include "GUIObjects/GUISystem.h"
#include "GUIObjects/GUIWidgets.h"
#include "GUIPort.h"
#include "ModelHandler.h"
#include "ModelWidget.h"
#include "Widgets/AnimationWidget.h"
#include "Widgets/LibraryWidget.h"
#include "MainWindow.h"
#include "LibraryHandler.h"
#include "MessageHandler.h"

//! @brief Constructor for the animation widget class
//! @param [in] parent Pointer to parent widget
AnimationWidget::AnimationWidget(QWidget *parent) :
    QWidget(parent)
{
    //Define public member pointer variables
    mpContainer = gpModelHandler->getCurrentViewContainerObject();
    mpAnimationData = mpContainer->getAppearanceData()->getAnimationDataPtr();

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
    mpGraphicsView->setRenderHint(QPainter::Antialiasing, gpConfig->getBoolSetting(CFG_ANTIALIASING));
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
    mpSettingsButton->setIcon(QIcon(QString(QString(ICONPATH) + "Hopsan-Configure.png")));

    mpStopButton = new QToolButton(this);
    mpStopButton->setIcon(QIcon(QString(QString(ICONPATH) + "Hopsan-Stop.png")));

    mpRewindButton = new QToolButton(this);
    mpRewindButton->setIcon(QIcon(QString(QString(ICONPATH) + "Hopsan-Rewind.png")));

    mpPauseButton = new QToolButton(this);
    mpPauseButton->setIcon(QIcon(QString(QString(ICONPATH) + "Hopsan-Pause.png")));

    mpPlayButton = new QToolButton(this);
    mpPlayButton->setIcon(QIcon(QString(QString(ICONPATH) + "Hopsan-Play.png")));

    mpPlayRealTimeButton = new QToolButton(this);
    mpPlayRealTimeButton->setIcon(QIcon(QString(QString(ICONPATH) + "Hopsan-PlayRealTime.png")));

    mpCloseButton = new QToolButton(this);
    mpCloseButton->setIcon(QIcon(QString(ICONPATH)+"Hopsan-Discard.png"));

    QLabel *pTimeLabel = new QLabel(" Time:", this);
    QLabel *pSpeedLabel = new QLabel(" Speed:", this);

    mpTimeSlider = new QSlider(Qt::Horizontal);

    mpSpeedSpinBox = new QDoubleSpinBox(this);
    mpSpeedSpinBox->setMinimum(-20.0);
    mpSpeedSpinBox->setMaximum(20.0);
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
    mpTimer = new QTimer(this);
    mpTime = new QTime();

    //Set default values for animation variables
    mRealTime=true;
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

    //Collect plot data from container (for non-realtime animations)
    //mpContainer->collectPlotData();
    mpPlotData = mpContainer->getLogDataHandler();
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
    for(int g=0;g<mModelObjectsList.size();g++)
    {
        AnimatedComponent *pAnimatedComponent = new AnimatedComponent(mModelObjectsList.at(g), this);
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
    connect(mpStopButton,           SIGNAL(clicked()),          this,   SLOT(stop()));
    connect(mpCloseButton,          SIGNAL(pressed()),          gpModelHandler->getCurrentModel(), SLOT(closeAnimation()));

    //Define slider connections
    connect(mpTimeSlider,           SIGNAL(sliderPressed()),    this,   SLOT(pause()));
    connect(mpTimeSlider,           SIGNAL(valueChanged(int)),  this,   SLOT(changeIndex(int)));
    connect(mpTimeSlider,           SIGNAL(sliderMoved(int)),   this,   SLOT(updateMovables()));
    connect(mpSpeedSpinBox,         SIGNAL(valueChanged(double)), this, SLOT(changeSpeed(double)));

    //Connect timer with update function
    connect(mpTimer,                SIGNAL(timeout()),          this,   SLOT(updateAnimation()));
}


//! @brief Destructor for animation widget class
AnimationWidget::~AnimationWidget()
{
    mpTimer->stop();
    delete mpTime;

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
void AnimationWidget::stop()
{
    mpTimeSlider->setValue(0);
    mLastAnimationTime = 0.0;
    mRealTime=false;
    mpTimer->stop();
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
    mpTimer->stop();
}


//! @brief Slot that plays the animation
void AnimationWidget::play()
{
    mRealTime=false;
    if(mSimulationSpeed == 0)
    {
        //Stop animation timer if speed is zero (it shouldn't be running for no reason)
        mpTimer->stop();
    }
    else
    {
        //Start the timer with correct FPS, if it is not already started
        if(!mpTimer->isActive())
        {
            mFps = 60;
            mLastTimeCheck = mpTimeDisplay->text().toDouble()*1000;
            mpTimer->start(1000.0/double(mFps));  //Timer object uses milliseconds
            mpTime->start();
            //mpTime->setHMS(0,0,0,mpTimeDisplay->text().toDouble()*1000);
        }
    }
}


//! @brief Slot that plays the animation continuously in real-time
//! @todo The plot data object in container should perhaps be cleared of these generations
void AnimationWidget::playRT()
{
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

    mRealTime = true;
    mpTimeSlider->setValue(1);
    changeIndex(0);

    if(mSimulationSpeed == 0)
    {
        //Stop animation timer if speed is zero (it shouldn't be running for no reason)
        mpTimer->stop();
    }
    else
    {
        //Start the timer with correct FPS, if it is not already started
        if(!mpTimer->isActive())
        {
            mFps = 60;
            mLastTimeCheck = mpTimeDisplay->text().toDouble()*1000;
            mpTimer->start(1000.0/double(mFps));  //Timer object uses milliseconds
            mpTime->start();
            //mpTime->setHMS(0,0,0,mpTimeDisplay->text().toDouble()*1000);
        }
    }
    //updateAnimationSpeed(1);
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
        mpTimer->stop();
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
        //Calculate time to simulate (equals interval of animation timer)
        double dT = mSimulationSpeed/double(mFps);

        //Simulate one interval (does NOT equal one time step, time step is usually much smaller)
        mpContainer->getCoreSystemAccessPtr()->simulate(mLastAnimationTime, mLastAnimationTime+dT, -1, true);

        //Update last animation time
        mLastAnimationTime = mLastAnimationTime+dT;
        mpTimeDisplay->setText(QString::number(mLastAnimationTime));

        //Update animated connectors and components
        updateMovables();
    }
    // Not real-time simulation (and we have time data)
    else if(!mTimeValues.isEmpty())
    {
        //Calculate animation time (with limitations)
        mCurrentAnimationTime = mLastAnimationTime+double(mSimulationSpeed)/mFps;
        mCurrentAnimationTime = std::min(mTotalTime, std::max(0.0, mCurrentAnimationTime));
        mLastAnimationTime = mCurrentAnimationTime;

        //Calculate index for time slider (with limitations)
        mIndex = mCurrentAnimationTime/mTotalTime*mnSamples;
        mIndex = round(std::min(mTimeValues.size()-1, std::max(0, mIndex)));
        mpTimeSlider->setValue(mIndex);
        if(mIndex == mTimeValues.size()-1)
        {
            mLastAnimationTime = 0.0;
            mRealTime=false;
            mpTimer->stop();
        }
        //! @todo Crash on next line when mIndex = -1, this should not happen
        mpTimeDisplay->setText(QString::number(mTimeValues.at(mIndex)));

        //Update animated connectors and components
        updateMovables();
    }

    // Auto-adjust FPS
    int dT = mpTime->elapsed();
    if(dT > 100)    //Only do this every .1 seconds
    {
        if(mpTimeDisplay->text().toDouble()*1000-mLastTimeCheck < 0.95*dT)
        {
            mFps = qMax(10.0, mFps*0.8);    //Too slow, decrease FPS
        }
        else
        {
            mFps = qMin(100.0, mFps*1.11);  //Not too slow, increase FPS slightly
        }
        mpTimer->start(1000.0/double(mFps));                    //Set timer interval
        mLastTimeCheck = mpTimeDisplay->text().toDouble()*1000; //Store last check time
        mpTime->restart();                                      //Restart speed check counter
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
        QDomElement animationRoot = domDocument.createElement("animation");
        domDocument.appendChild(animationRoot);

        QString subTypeName = pComp->mpModelObject->getSubTypeName();
        QString typeName = pComp->mpModelObject->getTypeName();
        SharedModelObjectAppearanceT pAppearanceData = gpLibraryHandler->getModelObjectAppearancePtr(typeName, subTypeName);
        pAppearanceData->getAnimationDataPtr()->saveToDomElement(animationRoot);
        QString baseIconPath = gpLibraryHandler->getModelObjectAppearancePtr(pComp->mpModelObject->getTypeName())->getAnimationDataPtr()->baseIconPath;

        //Store icon paths (they are not included in saveToDomElement() )
        QStringList iconPaths;
        foreach(const ModelObjectAnimationMovableData &m, pComp->getAnimationDataPtr()->movables)
        {
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
     this->stop();
     //delete mpParent->mpCentralTabs->getCurrentTab()->mpAnimationWidget;
     event->accept();
 }
