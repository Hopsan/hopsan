#include "AnimationWidget.h"
#include "Widgets/MessageWidget.h"
#include "GUIObjects/AnimatedComponent.h"
#include "MainWindow.h"
#include <QVector>
#include "AnimatedConnector.h"
#include "GUIConnector.h"
#include "GUIPort.h"
#include "GUIObjects/GUIContainerObject.h"

AnimationWidget::AnimationWidget(MainWindow *parent) :
    QWidget(parent)
{
    mpParent = parent;
    //Set the name and size of the main window
    //this->setObjectName("Animation Window");
    //this->setWindowTitle("Animation Window");
    this->resize(650,500);

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
//    mpQGraphicsView->setSceneRect(0,0,5000,5000);
    mpQGraphicsView->centerOn(mpQGraphicsView->sceneRect().topLeft());

    mpQGraphicsView->setRenderHint(QPainter::Antialiasing, gConfig.getAntiAliasing());
    mpQGraphicsView->centerOn(2500,2500);



    mpTextDisplay = new QLineEdit(this);
    mpTextDisplay->setBaseSize(20,10);
  //  mpTextDisplay->setGeometry(500,20,100,20);

    mpStopButton = new QToolButton(this);
    //mpPushButton_stop->setText("Stop");
    mpStopButton->setIcon(QIcon(QString(QString(ICONPATH) + "Hopsan-Stop.png")));
 //   mpPushButton_stop->setGeometry(510,60,10,30);

    mpRewindButton = new QToolButton(this);
    mpRewindButton->setText("Rewind");
    mpRewindButton->setIcon(QIcon(QString(QString(ICONPATH) + "Hopsan-Rewind.png")));
 //   mpPushButton_rewind->setGeometry(510,120,10,30);

    mpPauseButton = new QToolButton(this);
    mpPauseButton->setText("Pause");
    mpPauseButton->setIcon(QIcon(QString(QString(ICONPATH) + "Hopsan-Pause.png")));
   // mpPushButton_pause->setGeometry(510, 180, 10,30);

    mpPlayButton = new QToolButton(this);
    mpPlayButton->setText("Play");
    mpPlayButton->setIcon(QIcon(QString(QString(ICONPATH) + "Hopsan-Play.png")));
 //   mpPushButton_play->setGeometry(510,240,10,30);

    mpCloseButton = new QToolButton(this);
    mpCloseButton->setIcon(QIcon(QString(ICONPATH)+"Hopsan-Discard.png"));


//    mpPushButton_forward = new QPushButton(this);
//    mpPushButton_forward->setText("Forward");
//    mpPushButton_forward->setIcon(QIcon(QString(QString(ICONPATH) + "forward_button.svg")));
  //  mpPushButton_forward->setGeometry(510,300,10,30);

    QLabel *pTimeLabel = new QLabel(" Time:", this);
    QLabel *pSpeedLabel = new QLabel(" Speed:", this);

    mpTimeSlider = new QSlider(Qt::Horizontal);
    connect(mpTimeSlider, SIGNAL(sliderPressed()),this,SLOT(pause()));
    connect(mpTimeSlider, SIGNAL(sliderMoved(int)), this, SLOT(changeIndex(int)));
    connect(mpTimeSlider, SIGNAL(sliderReleased()),this, SLOT(play()));

    mpSpeedSlider = new QSlider(Qt::Horizontal);
    mpSpeedSlider->setMinimum(-40);
    mpSpeedSlider->setMaximum(40);
    mpSpeedSlider->setSingleStep(1);
    connect( mpSpeedSlider, SIGNAL(valueChanged(int)), this, SLOT(changeSpeed(int)));

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


    mpTimer = new QTimer(0);
    mpTimer->setInterval(WALL_CLOCK_INTERVAL_MILLISECONDS);
    connect(mpTimer,SIGNAL(timeout()),this,SLOT(updateAnimation()));
    mpTimer->start(0);  //1000

    currentSimulationTime = 0;
    previousSimulationTime = 0;
    simulationSpeed = 0;

    //

    mpContainer = this->mpParent->mpProjectTabs->getCurrentContainer();
    mpContainer->collectPlotData();

    mpPlotData = mpContainer->getAllPlotData();
    //This might need to be updated
    numberOfPlotGenerations = mpContainer->getNumberOfPlotGenerations();
    QString componentName = mpContainer->getModelObjectNames().first();
    QString portName = mpContainer->getModelObject(componentName)->getPortListPtrs().first()->getPortName();
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

    qDebug() << "Found " << mConnectorList.size() << " vectors!";



    for(int f=0;f<mConnectorList.size();f++)
    {
//        GUIModelObject* tempObject1 = mpGUIConnectorList.at(f)->getStartPort()->getGuiModelObject();
//        GUIModelObject* tempObject2 = mpGUIConnectorList.at(f)->getEndPort()->getGuiModelObject();

//        if(!mpGUIModelObjects->contains(tempObject1))
//        {
//            mpGUIModelObjects->append(tempObject1);
//        }
//        if(!mpGUIModelObjects->contains(tempObject2))
//        {
//            mpGUIModelObjects->append(tempObject2);
//        }

//        mpGUIConnectorList.replace(f,new GUIConnector(mpGUIConnectorList.at(f)));
//        mpGraphicsScene->addItem(mpGUIConnectorList.at(f));
        //for(int l=0; l<mpConnectorList.at(f)->getNumberOfLines(); ++l)
        //{
            //ConnectorLine *pConnectorLine = mpConnectorList.at(f)->getLine(l);
            //QGraphicsLineItem *pLine = dynamic_cast<QGraphicsLineItem *>(pConnectorLine);
            //pLine->setPos(mpConnectorList.at(f)->x()+pConnectorLine->x(),
            //              mpConnectorList.at(f)->y()+pConnectorLine->y());
            //mpGraphicsScene->addItem(pLine);
        //}
//        Connector *dummyConnector = mpConnectorList.at(f)->createDummyCopy();
        AnimatedConnector *pAnimatedConnector = new AnimatedConnector(mConnectorList.at(f), this);
        mpGraphicsScene->addItem(pAnimatedConnector);
        mAnimatedConnectorList.append(pAnimatedConnector);

        //mpConnectorList.removeAt(f);
        //mpConnectorList.insert(f, dummyConnector);
    }


    for(int g=0;g<mModelObjectsList.size();g++)
    {
        mAnimatedComponentList.append(AnimationWidget::createComponent(mModelObjectsList.at(g),this));
    }

    for(int h=0;h<mAnimatedComponentList.size();h++)
    {
        mAnimatedComponentList.at(h)->draw();
    }



    connect( mpRewindButton, SIGNAL( clicked() ), this, SLOT( rewind() ) );
    connect( mpPlayButton, SIGNAL( clicked() ), this, SLOT( play() ) );
    connect( mpPauseButton, SIGNAL( clicked() ), this, SLOT( pause() ) );
  //  connect( mpPushButton_forward, SIGNAL( clicked() ), this, SLOT( forward() ) );
    connect( mpStopButton, SIGNAL( clicked() ), this, SLOT( stop() ) );
    connect(mpCloseButton, SIGNAL(pressed()), mpParent->mpProjectTabs->getCurrentTab(), SLOT(closeAnimation()));
}


AnimationWidget::~AnimationWidget()
{
//    //Make sure the ports in the real model "forgets" the dummy component before they are removed
//    for(int i=0; i<mpConnectorList.size(); ++i)
//    {
//        gpMainWindow->mpProjectTabs->getCurrentContainer()->removeSubConnector(mpConnectorList.at(i));
//        //mpConnectorList.at(i)->getStartPort()->removeConnection(mpConnectorList.at(i));
//        //mpConnectorList.at(i)->getEndPort()->removeConnection(mpConnectorList.at(i));
//        //delete(mpConnectorList.at(i));
//    }

}


// define all the slots here

void AnimationWidget::stop()
{
    simulationSpeed = 0.0;
    previousSimulationTime = 0.0;
}

void AnimationWidget::rewind()
{
    simulationSpeed = -1.0;
}

void AnimationWidget::pause()
{
    simulationSpeed = 0.0;
}

void AnimationWidget::play()
{
    simulationSpeed = 1.0;
}
//chagne
void AnimationWidget::forward()
{
    index = mpTimeValues->size()-1;
    previousSimulationTime = mpTimeValues->at(index);
  //  simulationSpeed = 0;
  //  currentSimulationTime = mpTimeValues->at(mpTimeValues->size()-1);
}

void AnimationWidget::changeSpeed(int newSpeed)
{
    simulationSpeed = double(newSpeed)/10.0;
    return;
}

void AnimationWidget::changeIndex(int newIndex)
{
    index = std::min(std::max(newIndex,0), mpTimeValues->size()-1);
    index = newIndex;
    previousSimulationTime = mpTimeValues->at(index);
    return;
}

void AnimationWidget::updateAnimation()
{
    //This code snippet converts the actual time into simulation time
    currentSimulationTime  = std::min(mpTimeValues->last(), std::max(0.0,previousSimulationTime + (simulationSpeed)*(WALL_CLOCK_INTERVAL_MILLISECONDS * .005))); // originally was .001
    previousSimulationTime = currentSimulationTime;


    //This is the index that points to where in the Data/Time Vector the simulation is currently in.
    index= std::min(double(mpTimeValues->size()-1), std::max(0.0, currentSimulationTime/mpTimeValues->last()*mpTimeValues->size()));

    mpTimeSlider->setValue(index);
    mpTextDisplay->setText(QString::number(mpTimeValues->at(index)));

    //This snippet iterates through the list of animated (or unanimated) objects and
    //invokes their update() method. If the object is unanimated, its update() method
    //is blank.
    if(currentSimulationTime>0)
    {
        for(int c=0;c<mAnimatedComponentList.size();c++)
       {
          mAnimatedComponentList.at(c)->update();
       }
    }

    if(index==mpTimeValues->size()-1)
    {
        simulationSpeed=0;
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

 void AnimationWidget::closeEvent(QCloseEvent *event)
 {
     this->stop();
     //delete mpParent->mpProjectTabs->getCurrentTab()->mpAnimationWidget;
     event->accept();
 }
