/*-----------------------------------------------------------------------------

 Copyright 2017 Hopsan Group

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 The full license is available in the file GPLv3.
 For details about the 'Hopsan Group' or information about Authors and
 Contributors see the HOPSANGROUP and AUTHORS files that are located in
 the Hopsan source code root directory.

-----------------------------------------------------------------------------*/

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

