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
//! @file   GUIPortAppearance.h
//! @author Flumes <flumes@lists.iei.liu.se>
//! @date   2010-01-01
//!
//! @brief Contains the GUIPortAppearance class
//!
//$Id$

#ifndef GUIPORTAPPEARANCE_H
#define GUIPORTAPPEARANCE_H

#include <QString>
#include <QHash>
#include <QSharedPointer>

class PortAppearance
{
public:
    PortAppearance();
    void selectPortIcon(QString cqstype, QString porttype, QString nodetype);

    double x,y,rot;
    QString mMainIconPath;
    QString mCQSOverlayPath;
    QString mMultiPortOverlayPath;
    bool mEnabled;
    bool mAutoPlaced;
    bool mPoseModified;
};

typedef QSharedPointer<PortAppearance> SharedPortAppearanceT;
typedef QHash<QString, SharedPortAppearanceT> PortAppearanceMapT;


#endif // GUIPORTAPPEARANCE_H
