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

#ifndef AnimationWidget_H
#define AnimationWidget_H

#include <QTimer>
#include <QTextEdit>
#include <QList>
#include <QMap>
#include <QGraphicsRotation>
#include <QVector3D>
#include <QGraphicsRectItem>
#include <QTransform>
#include <QWidget>
#include <QObject>
#include <QGroupBox>
#include <QSlider>
#include <QDialog>
#include <QPushButton>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QtGui>

class ContainerObject;
class ProjectTabWidget;
class ProjectTab;
class GUIGroup;
class GUIComponent;
class Connector;
class Port;
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
    int getLastIndex();
    void closeEvent(QCloseEvent *event);
    GraphicsView *mpGraphicsView;


    //These are used for testing. We can see the numerical results of tests through these
    QLineEdit* mpTextDisplay;

    QMap<QString, double> mIntensityMaxMap;
    QMap<QString, double> mIntensityMinMap;

private slots:
    void stop();
    void rewind();
    void pause();
    void play();
    void updateAnimationSpeed();
    void updateAnimation();
    void updateMovables();

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
    int simulationSpeed;

    double mTimeStep;

    int numberOfPlotGenerations;
    double timeStep;
    int index;


public slots:
    void changeSpeed(int newSpeed);
    void changeIndex(int newIndex);
};

#endif // AnimationWidget_H


