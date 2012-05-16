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

class AnimatedGraphicsView;
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

    void closeEvent(QCloseEvent *event);

    //Get functions
    QVector<double> *getTimeValues();
    QGraphicsScene* getGraphicsScene();
    QList< QMap< QString, QMap< QString, QMap<QString, QPair<QVector<double>, QVector<double> > > > > >* getPlotDataPtr();
    int getNumberOfPlotGenerations();
    int getIndex(); // returns the current position inside the time vector
    int getLastIndex();
    bool isRealTimeAnimation();
    double getLastAnimationTime();
    AnimatedComponent *getAnimatedComponent(QString name);

    //Public member pointers
    AnimatedGraphicsView *mpGraphicsView;
    ContainerObject *mpContainer;

    //Maps that stores maximum and minimum values for simulation variables ("Pressure", "Velocity" etc)
    //! @todo These are not used, shall we use them or remove them?
    //! @todo If we use them, they shall not be public
    QMap<QString, double> mIntensityMaxMap;
    QMap<QString, double> mIntensityMinMap;
    QMap<QString, double> mFlowSpeedMap;

private slots:
    void openPreferencesDialog();
    void stop();
    void rewind();
    void pause();
    void play();
    void playRT();
    void updateAnimationSpeed(double speed);
    void updateAnimation();
    void updateMovables();

private:
    //Graphics scene
    QGraphicsScene *mpGraphicsScene;

    //Control panel group box
    QGroupBox* mpControlPanel;

    //The buttons
    QToolButton* mpSettingsButton;
    QToolButton* mpStopButton;
    QToolButton* mpRewindButton;
    QToolButton* mpPauseButton;
    QToolButton* mpPlayButton;
    QToolButton *mpPlayRealTimeButton;
    QToolButton* mpCloseButton;

    //Labels
    QLabel *mpTimeLabel;
    QLabel *mpSpeedLabel;

    //The sliders
    QSlider* mpTimeSlider;
    QSlider* mpSpeedSlider;

    //Time display widget
    QLineEdit* mpTimeDisplay;

    //Animation timer object
    QTimer *mpTimer;
    QTime *mpTime;
    int mLastTimeCheck;

    //Layout
    QGridLayout *mpLayout;

    //Copy of plot data object
    QList< QMap< QString, QMap< QString, QMap<QString, QPair<QVector<double>, QVector<double> > > > > > mpPlotData;

    //Copy of time values
    QVector<double>* mpTimeValues;

    //Lists of sub objects
    QList<ModelObject*> mModelObjectsList;
    QList<AnimatedComponent*> mAnimatedComponentList;
    QList<Connector*> mConnectorList;
    QList<AnimatedConnector*> mAnimatedConnectorList;

    //Private animation variables
    double mCurrentAnimationTime;
    double mLastAnimationTime;
    int mSimulationSpeed;
    double mTimeStep;
    int mnPlotGenerations;
    int mIndex;
    bool mRealTime;
    int mFps;
    int mSpeedSliderSensitivity;
    double mTotalTime;
    double mnSamples;

public slots:
    void changeSpeed(int newSpeed);
    void changeIndex(int newIndex);
};

#endif // AnimationWidget_H


