//$Id$

#ifndef GUICONNECTORLINE_H
#define GUICONNECTORLINE_H

//Should be as few includes as possible in h-files
#include <QPen>
//#include <QCursor>
//#include <QBrush>
//#include <QGraphicsScene>
#include <QGraphicsLineItem>
//#include <vector>
//#include <QGraphicsRectItem>
//#include <QCursor>
//#include <QBrush>
//#include <QGraphicsLineItem>
//#include <QGraphicsScene>
//#include "GUIConnector.h"
//#include "GUIPort.h"
//#include <vector>
//#include "GUIConnectorLine.h"
//#include "GUIConnector.h"
//#include <QGraphicsLineItem>
//#include <QGraphicsItem>
#include <QGraphicsSceneMoveEvent>

class GUIConnector;     //Forward declaration

class GUIConnectorLine : public QObject, public QGraphicsLineItem
{
    Q_OBJECT
public:
    GUIConnectorLine(qreal x1, qreal y1, qreal x2, qreal y2, QPen primaryPen, QPen activePen, int lineNumber, QGraphicsItem *parent = 0);
    ~GUIConnectorLine();
    void setActive(bool isActive);

signals:
    void lineClicked();
    void lineMoved(int);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void moveEvent(QGraphicsSceneMoveEvent *event);

private:
    bool mIsActive;
    QPen mPrimaryPen;
    QPen mActivePen;
    int mLineNumber;

};

#endif // GUICONNECTOR_H
