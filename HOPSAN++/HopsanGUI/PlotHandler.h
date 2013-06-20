#ifndef PLOTHANDLER_H
#define PLOTHANDLER_H

#include <QObject>
#include <QVector>
#include <QString>
#include <QMap>
#include <QColor>
#include <QSharedPointer>

// Forward Declarations
class ContainerObject;
class PlotWindow;
class LogVariableData;
typedef QSharedPointer<LogVariableData> SharedLogVariableDataPtrT;

class PlotHandler : public QObject
{
    Q_OBJECT
public:
    explicit PlotHandler(QObject *pParent=0);
    ~PlotHandler();

    PlotWindow *createNewPlotWindowOrGetCurrentOne(QString name="");
    PlotWindow *createNewOrReplacePlotwindow(const QString &rName="");
    PlotWindow *getPlotWindow(const QString name);

    void createPlotWindow(QString name="");
    PlotWindow *plotDataToWindow(PlotWindow *pPlotWindow, SharedLogVariableDataPtrT pData, int axis, QColor curveColor=QColor());
    PlotWindow *plotDataToWindow(PlotWindow *pPlotWindow, SharedLogVariableDataPtrT pDataX, SharedLogVariableDataPtrT pDataY, int axis, QColor curveColor=QColor());
    QString plotDataToWindow(QString windowName, SharedLogVariableDataPtrT pData, int axis, QColor curveColor=QColor());
    QString plotDataToWindow(QString windowName, SharedLogVariableDataPtrT pDataX, SharedLogVariableDataPtrT pDataY, int axis, QColor curveColor=QColor());

    void closeWindow(const QString &rWindowName);
    void closeAllOpenWindows();

    PlotWindow *createPlotWindow(QVector<double> xVector, QVector<double> yVector, int axis, QString componentName, QString portName, QString dataName, QString dataUnit, QString name="");
    
signals:
    
protected slots:
    void forgetPlotWindow(PlotWindow *pWindow);

private:
    QMap<QString, PlotWindow*> mOpenPlotWindows;
    
};

#endif // PLOTHANDLER_H
