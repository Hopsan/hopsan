#ifndef HVCWIDGET_H
#define HVCWIDGET_H

#include <QDialog>
#include <QLineEdit>
#include <QTreeWidget>

class FullNameVariableTreeWidget : public QTreeWidget
{
public:
    FullNameVariableTreeWidget(QWidget *pParent=0);
    void addFullNameVariable(const QString &rFullName);

protected:
    void mousePressEvent(QMouseEvent *event);
    void dropEvent(QDropEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);

private:
    void addFullNameVariable(const QString &rFullName, const QString &rRemaningName, QTreeWidgetItem *pParentItem);
    QString mTopLevelSystemName;

};

class HvcConfig
{
public:
    QString mFullVarName;
    QString mDataFile;
    int mDataColumn;
    double mTolerance;
};

class HVCWidget : public QDialog
{
    Q_OBJECT
public:
    explicit HVCWidget(QWidget *parent = 0);
    
signals:
    
public slots:
    void openHvcFile();
    void clearContents();
    void runHvcTest();

private:
    QString mModelFilePath;
    QList<HvcConfig> mDataConfigs;
    QLineEdit *mpHvcOpenPathEdit;
    FullNameVariableTreeWidget *mpAllVariablesTree;
    FullNameVariableTreeWidget *mpSelectedVariablesTree;

};

#endif // HVCWIDGET_H
