//$Id$

#ifndef GUICONNECTOR_H
#define GUICONNECTOR_H


#include <vector>
#include <QGraphicsWidget>
#include "GUIPort.h"
#include "GUIConnectorLine.h"


class GUIConnector : public QGraphicsWidget
{
    Q_OBJECT
public:
    GUIConnector(qreal x1, qreal y1, qreal x2, qreal y2, QPen passivePen, QPen activePen, QPen hoverPen, GraphicsView *parentView, QGraphicsItem *parent = 0);
    ~GUIConnector();
    QPointF startPos;
    QPointF endPos;
    void setStartPort(GUIPort *port);
    void setEndPort(GUIPort *port);
    GUIPort *getStartPort();
    GUIPort *getEndPort();
    void drawLine(QPointF startPos, QPointF endPos);
    void addLine();
    void addFixedLine(int length, int heigth, GUIConnectorLine::geometryType geometry);
    void removeLine(QPointF cursorPos);
    void setPen(QPen pen);
    int getNumberOfLines();
    int getLineNumber();
    GUIConnectorLine *getLine(int line);
    GUIConnectorLine *getOldLine();
    GUIConnectorLine *getLastLine();
    GUIConnectorLine *getThisLine();

public slots:
    void updatePos();
    void setActive();
    void setPassive();
    void setHovered();
    void setUnHovered();
    void deleteMe();
    //void deleteMeIfMeIsActive();
    void updateLine(int);
    void doSelect(bool lineSelected);

signals:
    void endPortConnected();

protected:
    virtual void SetEndPos(qreal x2, qreal y2);

private:
    std::vector<GUIConnectorLine*> mLines;
    GUIPort *mpStartPort;
    GUIPort *mpEndPort;
    GraphicsView *mpParentView;
    GUIConnectorLine *mpTempLine;
    QPen mPassivePen;
    QPen mActivePen;
    QPen mHoverPen;
    bool mIsActive;
    bool mEndPortConnected;
};

#endif // GUICONNECTOR_H
