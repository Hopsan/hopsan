#ifndef GUISYSTEM_H
#define GUISYSTEM_H

#include <QGraphicsWidget>
#include <QObject>
#include <QString>
#include <QTextStream>
#include <QPoint>
#include <QFileInfo>

#include "GUIContainerObject.h"
#include "common.h"

//Forward Declaration
class ProjectTab;


class GUISystem : public GUIContainerObject
{
    Q_OBJECT
public:
    GUISystem( QPoint position, qreal rotation, const GUIModelObjectAppearance* pAppearanceData, GUIContainerObject *system, selectionStatus startSelected = DESELECTED, graphicsType gfxType = USERGRAPHICS, QGraphicsItem *parent = 0);
    GUISystem(ProjectTab *parentProjectTab, QGraphicsItem *parent);
    ~GUISystem();

    double getStartTime();
    double getTimeStep();
    double getStopTime();

    size_t getNumberOfLogSamples();
    void setNumberOfLogSamples(size_t nSamples);

    void updateExternalPortPositions();

    QString getTypeName();
    void setName(QString newName);
    void setTypeCQS(QString typestring);
    QString getTypeCQS();

    void saveToTextStream(QTextStream &rStream, QString prepend);
    void saveToDomElement(QDomElement &rDomElement);
    void loadFromHMF(QString modelFilePath=QString());
    void loadFromDomElement(QDomElement &rDomElement);

    QVector<QString> getParameterNames();

    CoreSystemAccess* getCoreSystemAccessPtr();

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

    void openParameterDialog();

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
