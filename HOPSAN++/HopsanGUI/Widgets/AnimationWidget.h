#ifndef AnimationWidget_H
#define AnimationWidget_H

#include <QWidget>
#include <QObject>
#include <QGroupBox>
#include <QSlider>
#include <QDialog>
#include "MainWindow.h"
#include <QPushButton>
#include <QGraphicsScene>
#include <QGraphicsView>
#include "GraphicsView.h"
#include "GUIObjects/GUISystem.h"
#include "GUIObjects/GUIContainerObject.h"
#include "GUIConnector.h"
#include "GUIObjects/GUIObject.h"
#include "GUIObjects/GUIModelObjectAppearance.h"
#include "GUIObjects/GUIComponent.h"
#include "GUIObjects/GUIGroup.h"
#include "MainWindow.h"
#include "Configuration.h"
#include "Widgets/LibraryWidget.h"
#include <QtGui>
#include "Widgets/ProjectTabWidget.h"
#include <QTimer>
#include <QTextEdit>
#include <QList>
#include <QMap>
#include <QGraphicsRotation>
#include <QVector3D>
#include <QGraphicsRectItem>
#include <QTransform>
#include "Widgets/LibraryWidget.h"
#include "Widgets/ProjectTabWidget.h"
#include "MainWindow.h"
#include "GUIObjects/AnimatedComponent.h"
//#include "GUIObjects/AnimatedPressureGauge.h"

#define WALL_CLOCK_INTERVAL_MILLISECONDS 1

class ContainerObject;
class ProjectTabWidget;
class ProjectTab;
class GUIGroup;
class GUIComponent;
class AnimatedConnector;

class ProjectTabWidget;
class GraphicsView;
class QGraphicsScene;
class LibraryWidget;
class Configuration; // Might not need
class AnimatedComponent;

class QGraphicsView;
class QGraphicsScene;
class QTextEdit;
class QPushButton;
class QTimer;
class MainWindow;
class ModelObject;

class AnimationWidget : public QWidget
{
    Q_OBJECT
public:
    AnimationWidget(MainWindow *parent = 0);
    ~AnimationWidget();

    AnimatedComponent* createComponent(ModelObject* unanimatedComponent, AnimationWidget* parent);
    QGraphicsScene* getScenePtr();
    QList< QMap< QString, QMap< QString, QMap<QString, QPair<QVector<double>, QVector<double> > > > > >* getPlotDataPtr();
    int getNumberOfPlotGenerations();
    int getIndex(); // returns the current position inside the time vector
    void closeEvent(QCloseEvent *event);
    GraphicsView *mpGraphicsView;


    //These are used for testing. We can see the numerical results of tests through these
    QLineEdit* mpTextDisplay;

private slots:
    void stop();
    void rewind();
    void pause();
    void play();
    void forward();
    void updateAnimation();

private:

    QGroupBox* mpControlPanel;
    //The buttons
    QToolButton* mpStopButton;
    QToolButton* mpRewindButton;
    QToolButton* mpPauseButton;
    QToolButton* mpPlayButton;
    QToolButton* mpCloseButton;

    //The sliders
    QSlider* mpTimeSlider;
    QSlider* mpSpeedSlider;

    MainWindow* mpParent;

    QList< QMap< QString, QMap< QString, QMap<QString, QPair<QVector<double>, QVector<double> > > > > > mpPlotData;
    QVector<double>* mpTimeValues;

    QGraphicsView *mpQGraphicsView;
    QGraphicsScene *mpGraphicsScene;

    ContainerObject *mpContainer;

    QTimer *mpTimer;

    QList<ModelObject*> mUnanimatedComponentList;
    QList<ModelObject*> mModelObjectsList;
    QList<AnimatedComponent*> mAnimatedComponentList;
    QList<Connector*> mConnectorList;
    QList<AnimatedConnector*> mAnimatedConnectorList;
    QList<Port*> mPortList;

    double currentSimulationTime;
    double previousSimulationTime;
    double simulationSpeed;

    int numberOfPlotGenerations;
    double timeStep;
    int index;


public slots:
    void changeSpeed(int newSpeed);
    void changeIndex(int newIndex);
};

#endif // AnimationWidget_H


