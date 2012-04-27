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

#include "MainWindow.h"
#include "GraphicsView.h"
#include "GUIObjects/GUISystem.h"
#include "GUIObjects/GUIContainerObject.h"
#include "GUIConnector.h"
#include "GUIObjects/GUIObject.h"
#include "GUIObjects/GUIModelObjectAppearance.h"
//#include "GUIObjects/GUIComponent.h"
//#include "GUIObjects/GUIGroup.h"
#include "MainWindow.h"
#include "Configuration.h"
#include "Widgets/LibraryWidget.h"
#include "Widgets/ProjectTabWidget.h"
//#include "Widgets/LibraryWidget.h"
#include "MainWindow.h"
#include "GUIObjects/AnimatedComponent.h"
#include "GUIPort.h"
#include "AnimatedConnector.h"

AnimationWidget::AnimationWidget(MainWindow *parent) :
    QWidget(parent)
{
    mpParent = parent;

    this->setPalette(gConfig.getPalette());

    mpGraphicsScene = new QGraphicsScene();
    mpGraphicsScene->setSceneRect(0,0,5000,5000);

    mpQGraphicsView = new QGraphicsView(mpGraphicsScene,0); // 0 used to be this
    mpQGraphicsView->setGeometry(0,0,500,500);


 //   mpQGraphicsView->setDragMode(RubberBandDrag);
    mpQGraphicsView->setInteractive(true);
    mpQGraphicsView->setEnabled(true);
    mpQGraphicsView->setAcceptDrops(false);

    mpQGraphicsView->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    mpQGraphicsView->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    mpQGraphicsView->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    mpQGraphicsView->centerOn(mpQGraphicsView->sceneRect().topLeft());

    mpQGraphicsView->setRenderHint(QPainter::Antialiasing, gConfig.getAntiAliasing());
    mpQGraphicsView->centerOn(2500,2500);

    mpTextDisplay = new QLineEdit(this);
    mpTextDisplay->setBaseSize(20,10);

    mpStopButton = new QToolButton(this);
    mpStopButton->setIcon(QIcon(QString(QString(ICONPATH) + "Hopsan-Stop.png")));

    mpRewindButton = new QToolButton(this);
    mpRewindButton->setIcon(QIcon(QString(QString(ICONPATH) + "Hopsan-Rewind.png")));

    mpPauseButton = new QToolButton(this);
    mpPauseButton->setIcon(QIcon(QString(QString(ICONPATH) + "Hopsan-Pause.png")));

    mpPlayButton = new QToolButton(this);
    mpPlayButton->setIcon(QIcon(QString(QString(ICONPATH) + "Hopsan-Play.png")));

    mpCloseButton = new QToolButton(this);
    mpCloseButton->setIcon(QIcon(QString(ICONPATH)+"Hopsan-Discard.png"));


    QLabel *pTimeLabel = new QLabel(" Time:", this);
    QLabel *pSpeedLabel = new QLabel(" Speed:", this);

    mpTimeSlider = new QSlider(Qt::Horizontal);


    mpSpeedSlider = new QSlider(Qt::Horizontal);
    mpSpeedSlider->setMinimum(-40);
    mpSpeedSlider->setMaximum(40);
    mpSpeedSlider->setSingleStep(1);


    QGridLayout *vbox= new QGridLayout(this);
    vbox->addWidget(mpStopButton,0,0);
    vbox->addWidget(mpPauseButton,0,1);
    vbox->addWidget(mpPlayButton,0,2);
    vbox->addWidget(mpRewindButton,0,3);
    vbox->addWidget(pSpeedLabel,0,4);
    vbox->addWidget(mpSpeedSlider,0,5);
    vbox->addWidget(pTimeLabel,0,6);
    vbox->addWidget(mpTimeSlider,0,7);
    vbox->addWidget(mpTextDisplay,0,8);
    vbox->addWidget(mpCloseButton,0,9);
    vbox->addWidget(mpQGraphicsView,1,0,1,10);
    vbox->setColumnStretch(8,1);
    vbox->setRowStretch(1,1);
    this->setLayout(vbox);



    currentSimulationTime = 0;
    previousSimulationTime = 0;
    simulationSpeed = 0;

    //

    mpContainer = this->mpParent->mpProjectTabs->getCurrentContainer();
    mpContainer->collectPlotData();
    mTimeStep = mpParent->mpProjectTabs->getCurrentTopLevelSystem()->getTimeStep();



    mpTimer = new QTimer(0);

    mpPlotData = mpContainer->getAllPlotData();
    //This might need to be updated
    numberOfPlotGenerations = mpContainer->getNumberOfPlotGenerations();
    QString componentName;
    QString portName;
    int i=0;
    while(true)
    {
        componentName = mpContainer->getModelObjectNames().at(i);
        portName = mpContainer->getModelObject(componentName)->getPortListPtrs().first()->getPortName();
        if(mpContainer->getModelObject(componentName)->getPort(portName)->isConnected())
            break;
        else
            ++i;
    }

    mpTimeValues = new QVector<double>((mpContainer->getTimeVector(numberOfPlotGenerations-1, componentName, portName)));
    timeStep = mpTimeValues->at(1);//at(1);

    //define slider max and min here. the time slider should have the index
    mpTimeSlider->setMinimum(0);
    mpTimeSlider->setMaximum(mpTimeValues->size());
    mpTimeSlider->setSingleStep(1);

    mpSpeedSlider->setValue(simulationSpeed);

    QStringList modelObjectNames = mpContainer->getModelObjectNames();
    for(int i=0; i<modelObjectNames.size(); ++i)
    {
        mModelObjectsList.append(mpContainer->getModelObject(modelObjectNames.at(i)));
    }


    //Generate list of connectors
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


    for(int f=0;f<mConnectorList.size();f++)
    {
        AnimatedConnector *pAnimatedConnector = new AnimatedConnector(mConnectorList.at(f), this);
        mpGraphicsScene->addItem(pAnimatedConnector);
        mAnimatedConnectorList.append(pAnimatedConnector);
    }


    for(int g=0;g<mModelObjectsList.size();g++)
    {
        mAnimatedComponentList.append(AnimationWidget::createComponent(mModelObjectsList.at(g),this));
    }

    for(int h=0;h<mAnimatedComponentList.size();h++)
    {
        mAnimatedComponentList.at(h)->draw();
    }



    connect(mpRewindButton, SIGNAL(clicked()),          this,   SLOT(rewind()));
    connect(mpPlayButton,   SIGNAL(clicked()),          this,   SLOT(play()));
    connect(mpPauseButton,  SIGNAL(clicked()),          this,   SLOT(pause()));
    connect(mpStopButton,   SIGNAL(clicked()),          this,   SLOT(stop()));
    connect(mpCloseButton,  SIGNAL(pressed()),          mpParent->mpProjectTabs->getCurrentTab(), SLOT(closeAnimation()));

    connect(mpTimeSlider,   SIGNAL(sliderPressed()),    this,   SLOT(pause()));
    connect(mpTimeSlider,   SIGNAL(sliderMoved(int)),   this,   SLOT(changeIndex(int)));
    connect(mpTimeSlider,   SIGNAL(sliderReleased()),   this,   SLOT(play()));
    connect(mpTimer,        SIGNAL(timeout()),          this,   SLOT(updateAnimation()));
    connect(mpSpeedSlider,  SIGNAL(valueChanged(int)),  this,   SLOT(changeSpeed(int)));
}


AnimationWidget::~AnimationWidget()
{
    mpTimer->stop();
    delete(mpTimer);
}


// define all the slots here

void AnimationWidget::stop()
{
    simulationSpeed = 0;
    previousSimulationTime = 0.0;
    updateAnimationSpeed();
}

void AnimationWidget::rewind()
{
    simulationSpeed = -1;
    updateAnimationSpeed();
}

void AnimationWidget::pause()
{
    simulationSpeed = 0;
    updateAnimationSpeed();
}

void AnimationWidget::play()
{
    simulationSpeed = 1;
    updateAnimationSpeed();
}

void AnimationWidget::changeSpeed(int newSpeed)
{
    simulationSpeed = newSpeed;
    updateAnimationSpeed();
}

void AnimationWidget::changeIndex(int newIndex)
{
    qDebug() << "newIndex = " << newIndex;
    index = std::min(std::max(newIndex,0), mpTimeValues->size()-1);
    previousSimulationTime = mpTimeValues->at(index);
    updateMovables();
}


void AnimationWidget::updateAnimationSpeed()
{
    qDebug() << "Setting animation speed to: " << simulationSpeed;
    qDebug() << "Timer interval: " << fabs(1000*mTimeStep/simulationSpeed);
    if(simulationSpeed == 0)
    {
        mpTimer->stop();
    }
    else
    {
        mpTimer->start(fabs(1000*mTimeStep/simulationSpeed));
    }
}


void AnimationWidget::updateAnimation()
{
    //This code snippet converts the actual time into simulation time
    //currentSimulationTime  = std::min(mpTimeValues->last(), std::max(0.0,previousSimulationTime + simulationSpeed*mTimeStep));
    //previousSimulationTime = currentSimulationTime;

    //This is the index that points to where in the Data/Time Vector the simulation is currently in.
    double currentTime = previousSimulationTime+double(simulationSpeed)*mTimeStep;
    double totalTime = mpTimeValues->last();
    double nSamples = mpTimeValues->size();
    double newIndex = currentTime/totalTime*nSamples;
    index = ceil(std::min(double(mpTimeValues->size()-1), std::max(0.0, newIndex)));
    previousSimulationTime = mpTimeValues->at(index);

    qDebug() << "index = " << index;

    mpTimeSlider->setValue(index);
    mpTextDisplay->setText(QString::number(mpTimeValues->at(index)));

    updateMovables();

    if(index==mpTimeValues->size()-1)
    {
        simulationSpeed=0;
        updateAnimationSpeed();
    }
}


void AnimationWidget::updateMovables()
{
    //This snippet iterates through the list of animated (or unanimated) objects and
    //invokes their update() method. If the object is unanimated, its update() method
    //is blank.
   for(int c=0; c<mAnimatedComponentList.size(); ++c)
   {
      mAnimatedComponentList.at(c)->update();
   }
   for(int c=0; c<mAnimatedConnectorList.size(); ++c)
   {
       mAnimatedConnectorList.at(c)->update();
   }
}


QGraphicsScene* AnimationWidget::getScenePtr()
{
    return this->mpGraphicsScene;
}


//This method should make a proper sublcass of the AnimatedComponent based on the
//type of the unanimatedComponent, its parameters, AND the datavalues
AnimatedComponent* AnimationWidget::createComponent(ModelObject* unanimatedComponent, AnimationWidget* parent)
{

    // The component's parameters are stored inside GUIModelObject
 //   gpMainWindow->mpMessageWidget->printGUIInfoMessage(unanimatedComponent->getTypeName());


    //if(unanimatedComponent->getTypeName()=="HydraulicPressureSensor")
    //{
    //   return (new AnimatedPressureGauge(unanimatedComponent,1,parent)); // might need to remove 0
    //}
//    if(unanimatedComponent->getTypeName()=="HydraulicCylinderC")
//    {
        ModelObjectAppearance *pApp = gpMainWindow->mpLibrary->getAppearanceData(unanimatedComponent->getTypeName());
        QString basePath = pApp->getAnimationBaseIconPath();
        QStringList movablePaths = pApp->getAnimationMovableIconPaths();
        QStringList dataPorts = pApp->getAnimationDataPorts();
        QStringList dataNames = pApp->getAnimationDataNames();
        QStringList parameterMultipliers = pApp->getAnimationMultipliers();
        QStringList parameterDivisors = pApp->getAnimationDivisors();
        QVector<double> movementX = pApp->getAnimationSpeedX();
        QVector<double> movementY = pApp->getAnimationSpeedY();
        QVector<double> movementTheta = pApp->getAnimationSpeedTheta();
        QVector<double> startX = pApp->getAnimationStartX();
        QVector<double> startY = pApp->getAnimationStartY();
        QVector<double> startTheta = pApp->getAnimationStartTheta();
        QVector<double> transformOriginX = pApp->getAnimationTransformOriginX();
        QVector<double> transformOriginY = pApp->getAnimationTransformOriginY();

        return new AnimatedComponent(unanimatedComponent,basePath, movablePaths, dataPorts, dataNames,
                                     parameterMultipliers, parameterDivisors, movementX, movementY, movementTheta,
                                     startX, startY, startTheta, transformOriginX, transformOriginY, parent);

//        //return (new AnimatedCTypeCylinder(unanimatedComponent,1,parent));
//    }
//    else if(unanimatedComponent->getTypeName()=="Hydraulic43Valve")
//    {
//        QString basePath = gExecPath + QString(COMPONENTPATH) +  "hydraulic/valves/43valve_user_base.svg";
//        QStringList movablePaths = QStringList() << gExecPath + QString(COMPONENTPATH) + "hydraulic/valves/43valve_user_movable.svg";
//        QStringList dataPorts = QStringList() << "xv";
//        QStringList dataNames = QStringList() << "Value";
//        QVector<double> movementX = QVector<double>() << 4000;
//        QVector<double> movementY = QVector<double>() << 0;
//        QVector<double> movementTheta = QVector<double>() << 0;
//        QVector<double> startX = QVector<double>() << 0;
//        QVector<double> startY = QVector<double>() << 0;
//        QVector<double> startTheta = QVector<double>() << 0;
//        QVector<double> transformOriginX = QVector<double>() << 0;
//        QVector<double> transformOriginY = QVector<double>() << 0;

//        return new AnimatedComponent(unanimatedComponent,basePath, movablePaths, dataPorts, dataNames,
//                                     movementX, movementY, movementTheta, startX, startY, startTheta,
//                                     transformOriginX, transformOriginY, parent);

//        //return (new AnimatedCTypeCylinder(unanimatedComponent,1,parent));
//    }
//    else
//    {
//        return new AnimatedComponent(unanimatedComponent,QString(), QStringList(), QStringList(), QStringList(), QVector<double>(), QVector<double>(), QVector<double>(), QVector<double>(), QVector<double>(), QVector<double>(),QVector<double>(), QVector<double>(), parent);
//    }
}

QList< QMap< QString, QMap< QString, QMap<QString, QPair<QVector<double>, QVector<double> > > > > >* AnimationWidget::getPlotDataPtr()
{
    return &mpPlotData;
}

int AnimationWidget::getNumberOfPlotGenerations()
{
    return numberOfPlotGenerations;
}

 int AnimationWidget::getIndex()
 {
     return index;
 }

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
