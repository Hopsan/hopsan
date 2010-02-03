#ifndef COMPONENTGUICLASS_H
#define COMPONENTGUICLASS_H

#include <QGraphicsSvgItem>
#include <QWidget>

class ComponentGuiClass : public QGraphicsSvgItem
{
    Q_OBJECT
public:
    ComponentGuiClass(const QString &fileName, QGraphicsItem *parent = 0);
    ~ComponentGuiClass();

    QWidget *widget;
};

#endif // COMPONENTGUICLASS_H
