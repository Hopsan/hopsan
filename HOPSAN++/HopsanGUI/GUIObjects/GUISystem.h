//!
//! @file   GUISystem.h
//! @author Flumes <flumes@lists.iei.liu.se>
//! @date   2010-01-01
//!
//! @brief Contains the GUI System class, representing system components
//!
//$Id$

#ifndef GUISYSTEM_H
#define GUISYSTEM_H

#include <QGraphicsWidget>
#include <QObject>
#include <QString>
#include <QTextStream>
#include <QPoint>
#include <QFileInfo>

#include "GUIContainerObject.h"
#include "../common.h"

//Forward Declaration
class ProjectTab;


class GUISystem : public GUIContainerObject
{
    Q_OBJECT
public:
    GUISystem( QPoint position, qreal rotation, const GUIModelObjectAppearance* pAppearanceData, GUIContainerObject *pParentContainer, selectionStatus startSelected = DESELECTED, graphicsType gfxType = USERGRAPHICS);
    GUISystem(ProjectTab *parentProjectTab, QGraphicsItem *pParent);
    ~GUISystem();

    double getStartTime();
    double getTimeStep();
    double getStopTime();

    size_t getNumberOfLogSamples();
    void setNumberOfLogSamples(size_t nSamples);

    QString getTypeName();
    void setName(QString newName);
    //void setTypeCQS(QString typestring);
    QString getTypeCQS();

    void saveToDomElement(QDomElement &rDomElement);
    //void loadFromHMF(QString modelFilePath=QString());
    void loadFromDomElement(QDomElement &rDomElement);
    void setModelFileInfo(QFile &rFile);

    void saveToWrappedCode();

    QVector<QString> getParameterNames();

    CoreSystemAccess* getCoreSystemAccessPtr();
    GUIContainerObject *getParentContainerObject();

    enum { Type = GUISYSTEM };
    int type() const;

public slots:
    void updateStartTime();
    void updateTimeStep();
    void updateStopTime();
    void updateSimulationParametersInToolBar();

signals:

protected:
    QDomElement saveGuiDataToDomElement(QDomElement &rDomElement);
    void saveCoreDataToDomElement(QDomElement &rDomElement);

    void openPropertiesDialog();

private:
    void commonConstructorCode();

    double mStartTime;
    double mStopTime;
    double mTimeStep;
    size_t mNumberOfLogSamples;

    QString mLoadType;
    CoreSystemAccess *mpCoreSystemAccess;
};

#endif // GUISYSTEM_H
