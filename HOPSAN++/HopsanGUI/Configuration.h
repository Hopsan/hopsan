//!
//! @file   Configuration.h
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-11-15
//!
//! @brief Contains a class for the configuration object
//!
//$Id$

#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <QMap>
#include <QColor>
#include <QList>
#include <QFileInfo>
#include <QPen>
#include <QPalette>
#include <QFont>

#include "common.h"

class MainWindow;

class Configuration
{

public:
    void saveToXml();
    void loadFromXml();
    void loadDefaultsFromXml();

    bool getShowWelcomeDialog();
    bool getInvertWheel();
    bool getUseMulticore();
    size_t getNumberOfThreads();
    bool getToggleNamesButtonCheckedLastSession();
    bool getTogglePortsButtonCheckedLastSession();
    int getProgressBarStep();
    bool getEnableProgressBar();
    QColor getBackgroundColor();
    bool getAntiAliasing();
    QStringList getUserLibs();
    bool getSnapping();
    QStringList getRecentModels();
    QStringList getLastSessionModels();
    QString getDefaultUnit(QString key);
    QMap<QString, double> getCustomUnits(QString key);
    QPen getPen(QString type, graphicsType gfxType, QString situation);
    QPalette getPalette();
    QFont getFont();
    QString getStyleSheet();

    void setShowWelcomeDialog(bool value);
    void setInvertWheel(bool value);
    void setUseMultiCore(bool value);
    void setNumberOfThreads(size_t value);
    void setProgressBarStep(int value);
    void setEnableProgressBar(bool value);
    void setBackgroundColor(QColor value);
    void setAntiAliasing(bool value);
    void addUserLib(QString value);
    void removeUserLib(QString value);
    bool hasUserLib(QString value);
    void setSnapping(bool value);
    void addRecentModel(QString value);
    void addLastSessionModel(QString value);
    void clearLastSessionModels();
    void setDefaultUnit(QString key, QString value);
    void addCustomUnit(QString dataname, QString unitname, double scale);

private:
    bool mShowWelcomeDialog;
    bool mInvertWheel;
    bool mUseMulticore;
    size_t mNumberOfThreads;
    bool mToggleNamesButtonCheckedLastSession;
    bool mTogglePortsButtonCheckedLastSession;
    int mProgressBarStep;
    bool mEnableProgressBar;
    QColor mBackgroundColor;
    bool mAntiAliasing;
    QStringList mUserLibs;
    bool mSnapping;
    QStringList mRecentModels;
    QStringList mLastSessionModels;
    QMap<QString, QString> mDefaultUnits;
    QMap< QString, QMap<QString, double> > mCustomUnits;
    QPalette mPalette;
    QFont mFont;
    QString mStyleSheet;

    QMap < QString, QMap< QString, QMap<QString, QPen> > > mPenStyles;
};

#endif // CONFIGURATION_H
