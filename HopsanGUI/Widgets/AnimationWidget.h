/*-----------------------------------------------------------------------------
 This source file is a part of Hopsan

 Copyright (c) 2009 to present year, Hopsan Group

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

 For license details and information about the Hopsan Group see the files
 GPLv3 and HOPSANGROUP in the Hopsan source code root directory

 For author and contributor information see the AUTHORS file
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

#include <QWidget>
#include <QMap>

// Forward declarations
class ContainerObject;
class CentralTabWidget;
class ModelWidget;
class GUIComponent;
class Connector;
class Port;
class AnimatedConnector;

class CentralTabWidget;
class GraphicsView;
class LibraryWidget;
class Configuration; // Might not need
class AnimatedComponent;

class AnimatedGraphicsView;
class MainWindow;
class ModelObject;
class LogDataHandler2;
class ModelObjectAnimationData;

class QGraphicsScene;
class QTextEdit;
class QPushButton;
class QTimer;
class QGraphicsScene;
class QGridLayout;
class QToolButton;
class QLabel;
class QGroupBox;
class QDoubleSpinBox;
class QLineEdit;
class QSlider;

class AnimationWidget : public QWidget
{
    Q_OBJECT
public:
    AnimationWidget(QWidget *parent = 0);
    ~AnimationWidget();

    void closeEvent(QCloseEvent *event);

    //Get functions
    QGraphicsScene* getGraphicsScene();
    LogDataHandler2 *getPlotDataPtr();

    int getIndex(); // returns the current position inside the time vector
    int getLastIndex();
    bool isRealTimeAnimation();
    double getLastAnimationTime();
    AnimatedComponent *getAnimatedComponent(QString name);

    //Public member pointers
    AnimatedGraphicsView *mpGraphicsView=nullptr;
    ContainerObject *mpContainer=nullptr;
    ModelObjectAnimationData *mpAnimationData=nullptr;

    //Maps that stores maximum and minimum values for simulation variables ("Pressure", "Velocity" etc)
    //! @todo These are not used, shall we use them or remove them?
    //! @todo If we use them, they shall not be public
    QMap<QString, double> mIntensityMaxMap;
    QMap<QString, double> mIntensityMinMap;
    QMap<QString, double> mFlowSpeedMap;

    //These three variables are for improving performance (perhaps the three maps above should be deleted)
    double mHydraulicIntensityMax;
    double mHydraulicIntensityMin;
    double mHydraulicSpeed;

private slots:
    void openPreferencesDialog();
    void stop();
    void rewind();
    void pause();
    void play();
    void playRT();
    void updateAnimationSpeed(const double speedScale);
    void updateAnimation();
    void updateMovables();
    void resetAllAnimationDataToDefault();

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

    //The sliders
    QSlider* mpTimeSlider;
    QDoubleSpinBox* mpSpeedSpinBox;

    //Time display widget
    QLineEdit* mpTimeDisplay;

    //Animation timer object
    QTimer *mpTimer;
    QTime *mpTime;
    int mLastTimeCheck;

    //Layout
    QGridLayout *mpLayout;

    //Copy of plot data object
    LogDataHandler2 *mpPlotData;

    //Copy of time values
    QVector<double> mTimeValues;

    //Lists of sub objects
    QList<ModelObject*> mModelObjectsList;
    QList<AnimatedComponent*> mAnimatedComponentList;
    QList<Connector*> mConnectorList;
    QList<AnimatedConnector*> mAnimatedConnectorList;

    //Private animation variables
    double mCurrentAnimationTime;
    double mLastAnimationTime;
    double mSimulationSpeed;
    double mTimeStep;
    int mIndex;
    bool mRealTime;
    int mFps;
    double mTotalTime;
    double mnSamples;

public slots:
    void changeSpeed(double newSpeed);
    void changeIndex(int newIndex);
};

#endif // AnimationWidget_H


