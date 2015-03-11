//!
//! @file   GraphicsViewPort.h
//! @author Flumes <peter.nordin@liu.se>
//! @date   2015-03-11
//!
//! @brief Contains the GraphicsViewPort class
//!
//$Id$

#ifndef GRAPHICSVIEWPORT_H
#define GRAPHICSVIEWPORT_H

#include <QPointF>

class GraphicsViewPort
{
public:
    GraphicsViewPort() : mCenter(10,10), mZoom(1.0) {}
    GraphicsViewPort(double x, double y, double zoom) : mCenter(x,y), mZoom(zoom) {}
    GraphicsViewPort(QPointF center, double zoom) : mCenter(center), mZoom(zoom) {}
    QPointF mCenter;
    double mZoom;
};

#endif // GRAPHICSVIEWPORT_H

