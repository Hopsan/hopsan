//$Id$

#ifndef Configuration_H
#define Configuration_H

#include <QMap>
#include <QColor>
#include <QList>
#include <QFileInfo>

class MainWindow;

class Configuration
{

public:
    void saveToXml();
    void loadFromXml();

    bool getInvertWheel();
    bool getUseMulticore();
    size_t getNumberOfThreads();
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
    bool mInvertWheel;
    bool mUseMulticore;
    size_t mNumberOfThreads;
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

};

#endif // Configuration_H
