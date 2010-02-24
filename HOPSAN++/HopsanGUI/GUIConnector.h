//$Id$

#ifndef GUICONNECTOR_H
#define GUICONNECTOR_H

//#include <QCursor>
//#include <QBrush>
//#include <QGraphicsScene>
//#include <QGraphicsLineItem>
//#include <vector>
//#include <QGraphicsRectItem>
//#include <QCursor>
//#include <QBrush>
//#include <QGraphicsLineItem>
//#include <QGraphicsScene>
//#include "GUIConnector.h"
//#include "GUIPort.h"
#include <vector>
//#include "GUIConnectorLine.h"
#include <QGraphicsWidget>
#include <QGraphicsView>

class GUIPort;     //Forwarad declaration
class GUIConnectorLine; //Forward  declaration

class GUIConnector : public QGraphicsWidget
{
    Q_OBJECT
public:
    GUIConnector(qreal x1, qreal y1, qreal x2, qreal y2, qreal width, QColor color, QColor activecolor,
                 QColor hoverColor, QGraphicsView *parentView, QGraphicsItem *parent = 0);
    ~GUIConnector();
    QPointF startPos;
    QPointF endPos;
    void setStartPort(GUIPort *port);
    void setEndPort(GUIPort *port);
    GUIPort *getStartPort();
    GUIPort *getEndPort();
    void drawLine(QPointF startPos, QPointF endPos);
    void addLine();
    void removeLine(QPointF cursorPos);
    void setPen(QPen pen);
    int getNumberOfLines();
    void setStraigth(bool var);
    bool isStraigth();

public slots:
    void updatePos();
    void setActive();
    void setPassive();
    void setHovered();
    void setUnHovered();
    void deleteMe();
    void updateLine(int);

protected:
    virtual void SetEndPos(qreal x2, qreal y2);
    QVariant selectedEvent(GraphicsItemChange change, const QVariant &value);

private:
    std::vector<GUIConnectorLine*> mLines;
    GUIPort *mpStartPort;
    GUIPort *mpEndPort;
    QGraphicsView *mpParentView;
    GUIConnectorLine *mpTempLine;
    qreal mWidth;
    QColor mPrimaryColor;
    QColor mActiveColor;
    QColor mHoverColor;
    bool mStraigth;
    bool mIsActive;
    bool mEndPortConnected;
};

#endif // GUICONNECTOR_H
