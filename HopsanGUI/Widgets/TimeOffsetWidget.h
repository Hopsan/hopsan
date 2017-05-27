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
//! @file   TimeOffsetWidget.h
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   20150917
//!
//! @brief Contains the plot offset widget
//!
//$Id$

#ifndef TIMEOFFSETWIDGET_H
#define TIMEOFFSETWIDGET_H

#include <QObject>
#include <QWidget>
#include <QLineEdit>

class LogDataHandler2;

class TimeOffsetWidget : public QWidget
{
    Q_OBJECT
public:
    TimeOffsetWidget(int generation, LogDataHandler2 *pLogDataHandler, QWidget *pParent=0);

signals:
    void valuesChanged();

private slots:
    void setOffset(const QString &rOffset);
    void zeroOffset();

private:
    LogDataHandler2 *mpLogDataHandler;
    int mGeneration;
    QLineEdit *mpOffsetLineEdit=0;
    QString mTimeUnit;
};

#endif // TIMEOFFSETWIDGET_H
