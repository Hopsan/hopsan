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
//! @file   HelpPopUpWidget.h
//! @author Flumes <flumes@lists.iei.liu.se>
//! @date   2014-01-14
//!
//! @brief Contains the help popup widget
//!
//$Id$

#ifndef HELPPOPUPWIDGET_H
#define HELPPOPUPWIDGET_H

#include <QWidget>
#include <QLabel>

class HelpPopUpWidget : public QWidget
{
    Q_OBJECT
public:
    HelpPopUpWidget(QWidget *pParent);

public slots:
    void showHelpPopupMessage(const QString &rMessage);
    void openContextHelp();
    void openContextHelp(QString file);

private:
    QLabel *mpHelpText;
    QTimer *mpTimer;
};

#endif // HELPPOPUPWIDGET_H
