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
//! @file   AnimationWidget.cpp
//! @author Pratik Deschpande <pratik661@gmail.com>
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2012-04-25
//!
//! @brief Contains a widget for showing animations
//!
//$Id$

#include <QVector>

#include "AnimatedConnector.h"
#include "Configuration.h"
#include "GraphicsView.h"
#include "GUIPort.h"
#include "MainWindow.h"
#include "GUIObjects/AnimatedComponent.h"
#include "GUIObjects/GUIContainerObject.h"
#include "GUIObjects/GUISystem.h"
#include "Widgets/AnimationWidget.h"
#include "Widgets/LibraryWidget.h"
#include "Widgets/ProjectTabWidget.h"

//! @brief Constructor for the animation widget class
//! @param [in] parent Pointer to parent widget
AnimationWidget::AnimationWidget(MainWindow *parent) :
    QWidget(parent)
{
    //Define public member pointer variables
    mpContainer = gpMainWindow->mpProjectTabs->getCurrentContainer();

    //Set palette
    this->setPalette(gConfig.getPalette());

    //Create graphics scene
    mpGraphicsScene = new QGraphicsScene();
    mpGraphicsScene->setSceneRect(0,0,5000,5000);

    //Create graphics view
    mpGraphicsView = new AnimatedGraphicsView(mpGraphicsScene,0);
    mpGraphicsView->setGeometry(0,0,500,500);
    mpGraphicsView->setInteractive(true);
    mpGraphicsView->setEnabled(true);
    mpGraphicsView->setAcceptDrops(false);
    mpGraphicsView->setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
    mpGraphicsView->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    mpGraphicsView->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    mpGraphicsView->centerOn(mpGraphicsView->sceneRect().topLeft());
    mpGraphicsView->setRenderHint(QPainter::Antialiasing, gConfig.getAntiAliasing());
    mpGraphicsView->centerOn(2500,2500);

    //Create control panel widgets
    mpTimeDisplay = new QLineEdit(this);
    mpTimeDisplay->setBaseSize(20,10);

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

    mpTimeLabel = new QLabel(" Time:", this);
    mpSpeedLabel = new QLabel(" Speed:", this);

    mpTimeSlider = new QSlider(Qt::Horizontal);

    mpSpeedSlider = new QSlider(Qt::Horizontal);
    mpSpeedSlider->setMinimum(-20);
    mpSpeedSlider->setMaximum(20);
    mpSpeedSlider->setSingleStep(1);

    //Create the layout and add widgets
    QGridLayout *vbox= new QGridLayout(this);
    vbox->addWidget(mpStopButton,           0,  0);
    vbox->addWidget(mpPauseButton,          0,  1);
    vbox->addWidget(mpPlayButton,           0,  2);
    vbox->addWidget(mpPlayRealTimeButton,   0,  3);
    vbox->addWidget(mpRewindButton,         0,  4);
    vbox->addWidget(mpSpeedLabel,           0,  5);
    vbox->addWidget(mpSpeedSlider,          0,  6);
    vbox->addWidget(mpTimeLabel,            0,  7);
    vbox->addWidget(mpTimeSlider,           0,  8);
    vbox->addWidget(mpTimeDisplay,          0,  9);
    vbox->addWidget(mpCloseButton,          0,  10);
    vbox->addWidget(mpGraphicsView,         1,  0,  1,  11);
    vbox->setColumnStretch(8,1);
    vbox->setRowStretch(1,1);
    this->setLayout(vbox);

    //Create the timer object
    mpTimer = new QTimer(0);

    //Set default values for animation variables
    mRealTime=false;
    mCurrentAnimationTime = 0;
    mLastAnimationTime = 0;
    mSimulationSpeed = 0;
    mTimeStep = gpMainWindow->mpProjectTabs->getCurrentTopLevelSystem()->getTimeStep(); //! @todo This is not used, but it should be
    mFps=50;   //Frames per second
    mSpeedSliderSensitivity=10;

    //Collect plot data from container (for non-realtime animations)
    //mpContainer->collectPlotData();
    mpPlotData = mpContainer->getAllPlotData();

    //Obtain time values from plot data
    mpTimeValues = getTimeValues();

    //Calculate total simulation time and number of samples
    mTotalTime = mpTimeValues->last();
    mnSamples = mpTimeValues->size();

    //Set min, max and step values for time slider
    mpTimeSlider->setMinimum(0);
    mpTimeSlider->setMaximum(mpTimeValues->size());
    mpTimeSlider->setSingleStep(mpTimeValues->size()/mFps);

    //Set initial values for speed slider
    mpSpeedSlider->setValue(mSimulationSpeed);

    //Generate list of model objects from container object
    QStringList modelObjectNames = mpContainer->getModelObjectNames();
    for(int i=0; i<modelObjectNames.size(); ++i)
    {
        mModelObjectsList.append(mpContainer->getModelObject(modelObjectNames.at(i)));
    }

    //Generate list of connectors from container object
    for(int d=0;d<mModelObjectsList.size();d++)
    {
        for(int e=0;e<mModelObjectsList.at(d)->getConnectorPtrs().size();e++)
        {
            Connector* tempConnector = mModelObjectsList.at(d)->getConnectorPtrs().at(e);
            if(!mConnectorList.contains(tempConnector))
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

    //Define button connections
    connect(mpRewindButton,         SIGNAL(clicked()),          this,   SLOT(rewind()));
    connect(mpPlayButton,           SIGNAL(clicked()),          this,   SLOT(play()));
    connect(mpPlayRealTimeButton,   SIGNAL(clicked()),          this,   SLOT(playRT()));
    connect(mpPauseButton,          SIGNAL(clicked()),          this,   SLOT(pause()));
    connect(mpStopButton,           SIGNAL(clicked()),          this,   SLOT(stop()));
    connect(mpCloseButton,          SIGNAL(pressed()),          gpMainWindow->mpProjectTabs->getCurrentTab(), SLOT(closeAnimation()));

    //Define slider connections
    connect(mpTimeSlider,           SIGNAL(sliderPressed()),    this,   SLOT(pause()));
    connect(mpTimeSlider,           SIGNAL(valueChanged(int)),  this,   SLOT(changeIndex(int)));
    connect(mpTimeSlider,           SIGNAL(sliderMoved(int)),   this,   SLOT(updateMovables()));
    connect(mpSpeedSlider,          SIGNAL(valueChanged(int)),  this,   SLOT(changeSpeed(int)));

    //Connect timer with update function
    connect(mpTimer,                SIGNAL(timeout()),          this,   SLOT(updateAnimation()));
}


//! @brief Destructor for animation widget class
AnimationWidget::~AnimationWidget()
{
    mpTimer->stop();
    delete(mpTimer);
}


//! @brief Calculates time values from plot data object
QVector<double> *AnimationWidget::getTimeValues()
{
    mnPlotGenerations = mpContainer->getNumberOfPlotGenerations();
    QString componentName;
    QString portName;
    int i=0;
    while(true)
    {
        componentName = mpContainer->getModelObjectNames().at(i);
        portName = mpContainer->getModelObject(componentName)->getPortListPtrs().first()->getPortName();
        if(mpContainer->getModelObject(componentName)->getPort(portName)->isConnected() && mpContainer->getModelObject(componentName)->getPort(portName)->getPortType() != "POWERMULTIPORT")
            break;
        else
            ++i;
    }

    return new QVector<double>((mpContainer->getTimeVector(mnPlotGenerations-1, componentName, portName)));
}


//! @brief Slot that stops the animation
void AnimationWidget::stop()
{
    mLastAnimationTime = 0.0;
    mRealTime=false;
    updateAnimationSpeed(0);
}


//! @brief Slot that rewindes the animation (with full speed)
void AnimationWidget::rewind()
{
    mRealTime=false;
    updateAnimationSpeed(-1);
}


//! @brief Slot that pauses the animation
void AnimationWidget::pause()
{
    updateAnimationSpeed(0);
}


//! @brief Slot that plays the animation (with full speed)
void AnimationWidget::play()
{
    mRealTime=false;
    updateAnimationSpeed(1);
}


//! @brief Slot that plays the animation continously in real-time
//! @todo The plot data object in container should perhaps be cleared of these generations
void AnimationWidget::playRT()
{
    mpContainer->getCoreSystemAccessPtr()->initialize(0,10,0);
    mRealTime = true;
    mpTimeSlider->setValue(1);
    changeIndex(0);
    updateAnimationSpeed(1);
}


//! @brief Slot that changes animation speed
//! @param [in] newSpeed New speed for animation
void AnimationWidget::changeSpeed(int newSpeed)
{
    updateAnimationSpeed(double(newSpeed)/double(mSpeedSliderSensitivity));
}


//! @brief Slot that changes time index of animation (used when time slider is moved)
void AnimationWidget::changeIndex(int newIndex)
{
    mIndex = std::min(std::max(newIndex,0), mpTimeValues->size()-1);
    mLastAnimationTime = mpTimeValues->at(mIndex);
    mpTimeDisplay->setText(QString::number(mpTimeValues->at(mIndex)));
}


//! @brief Updates animation speed
void AnimationWidget::updateAnimationSpeed(double speed)
{
    //Speed slider sensitivity must be multiplied, because of int->double resolution
    mSimulationSpeed = speed*mSpeedSliderSensitivity;
    if(mSimulationSpeed == 0)
    {
        //Stop animation timer if speed is zero (it sholdn't be running for no reason)
        mpTimer->stop();
    }
    else
    {
        //Start the timer with correct FPS, if it is not already started
        if(!mpTimer->isActive())
        {
            mpTimer->start(1000.0/double(mFps));  //Timer object uses milliseconds
        }
    }
}


//! @brief Slot that updates the animation, called by animation timer object
void AnimationWidget::updateAnimation()
{
    if(!mRealTime)      //Not real-time simulation
    {
        //Calculate animation time (with limitations)
        mCurrentAnimationTime = mLastAnimationTime+double(mSimulationSpeed)/mSpeedSliderSensitivity/mFps;
        mCurrentAnimationTime = std::min(mTotalTime, std::max(0.0, mCurrentAnimationTime));
        mLastAnimationTime = mCurrentAnimationTime;

        //Calculate index for time slider (with limitations)
        mIndex = mCurrentAnimationTime/mTotalTime*mnSamples;
        mIndex = round(std::min(mpTimeValues->size()-1, std::max(0, mIndex)));
        mpTimeSlider->setValue(mIndex);
        mpTimeDisplay->setText(QString::number(mpTimeValues->at(mIndex)));

        //Update animated connectors and components
        updateMovables();

        //Stop playback if maximum time is reached
        if(mIndex==mpTimeValues->size()-1)
        {
            updateAnimationSpeed(0);
        }
    }
    else    //Real-time simualtion
    {
        //Calculate time to simulate (equals interval of animation timer)
        double dT = double(mSimulationSpeed)/double(mSpeedSliderSensitivity)/double(mFps);

        //Simulate one interval (does NOT equal one time step, time step is usually much smaller)
        mpContainer->getCoreSystemAccessPtr()->simulate(mLastAnimationTime, mLastAnimationTime+dT, SINGLECORE, 0, true);

        //Update last animation time
        mLastAnimationTime = mLastAnimationTime+dT;
        mpTimeDisplay->setText(QString::number(mLastAnimationTime));

        //Update animated connectors and components
        updateMovables();
    }
}


//! @brief Help function that updates all animated components and connectors
void AnimationWidget::updateMovables()
{
   //Update animated components
   for(int c=0; c<mAnimatedComponentList.size(); ++c)
   {
       mAnimatedComponentList[c]->update();
   }

   //Update animated connectors
   for(int c=0; c<mAnimatedConnectorList.size(); ++c)
   {
       mAnimatedConnectorList[c]->update();
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


//! @brief Returns a pointer to the graphics scene
QGraphicsScene* AnimationWidget::getGraphicsScene()
{
    return this->mpGraphicsScene;
}


//! @brief Returns a pointer to the plot data object
QList< QMap< QString, QMap< QString, QMap<QString, QPair<QVector<double>, QVector<double> > > > > >* AnimationWidget::getPlotDataPtr()
{
    return &mpPlotData;
}


//! @brief Returns number of plot generations in the plot data
int AnimationWidget::getNumberOfPlotGenerations()
{
    return mnPlotGenerations;
}


//! @brief Returns current time index of animation
int AnimationWidget::getIndex()
{
 return mIndex;
}


//! @brief Returns last time index of animation
int AnimationWidget::getLastIndex()
{
 return mpTimeValues->size()-1;
}

 void AnimationWidget::closeEvent(QCloseEvent *event)
 {
     this->stop();
     //delete mpParent->mpProjectTabs->getCurrentTab()->mpAnimationWidget;
     event->accept();
 }
