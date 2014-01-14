/*-----------------------------------------------------------------------------
 This source file is part of Hopsan NG

 Copyright (c) 2011
    Mikael Axin, Robert Braun, Alessandro Dell'Amico, Björn Eriksson,
    Peter Nordin, Karl Pettersson, Petter Krus, Ingo Staack

 This file is provided "as is", with no guarantee or warranty for the
 functionality or reliability of the contents. All contents in this file is
 the original work of the copyright holders at the Division of Fluid and
 Mechatronic Systems (Flumes) at Linköping University. Modifying, using or
 redistributing any part of this file is prohibited without explicit
 permission from the copyright holders.
-----------------------------------------------------------------------------*/

//!
//! @file   HelpPopUpWidget.h
//! @author Flumes <flumes@lists.iei.liu.se>
//! @date   2014-01-14
//!
//! @brief Contains the help popup widget
//!
//$Id: PlotWindow.h 6422 2014-01-13 16:10:52Z petno25 $

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

private:
    QLabel *mpHelpText;
    QTimer *mpTimer;
};

#endif // HELPPOPUPWIDGET_H
