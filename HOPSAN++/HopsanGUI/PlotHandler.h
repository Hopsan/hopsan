#ifndef PLOTHANDLER_H
#define PLOTHANDLER_H

#include <QObject>
#include <QVector>
#include <QString>
#include <QMap>
#include <QColor>

// Forward Declarations
class ContainerObject;
class LogVariableData;
class PlotWindow;

class PlotHandler : public QObject
{
    Q_OBJECT
public:
    explicit PlotHandler(ContainerObject *pContainerObject = 0);

    void createPlotWindow(QString name="");
    PlotWindow *createNewPlotWindowOrGetCurrentOne(QString name="");
    PlotWindow *getPlotWindow(const QString name);

    void addPlotCurve(QString windowName, LogVariableData *pData, int axis, QColor curveColor=QColor());
    PlotWindow *createPlotWindow(LogVariableData *pData, QColor desiredColor=QColor(), QString name="");
    PlotWindow *createPlotWindow(QVector<double> xVector, QVector<double> yVector, int axis, QString componentName, QString portName, QString dataName, QString dataUnit, QString name="");
    
signals:
    
protected slots:
    void forgetPlotWindow(PlotWindow *pWindow);

private:
    ContainerObject *mpContainerObject;
    QMap<QString, PlotWindow*> mOpenPlotWindows;
    
};

#endif // PLOTHANDLER_H
