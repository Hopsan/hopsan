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
//! @file   QuickNavigationWidget.h
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2010-12-xx
//!
//! @brief Contains the quick navigation widget that is used to go back after entering into container objects
//!
//$Id$

#ifndef QUICKNAVIGATIONWIDGET_H
#define QUICKNAVIGATIONWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QButtonGroup>

//Forward Declarations
class GUIContainerObject;

class QuickNavigationWidget : public QWidget
{
    Q_OBJECT
public:
    QuickNavigationWidget(QWidget *parent = 0);
    void addOpenContainer(GUIContainerObject* pContainer);

signals:

public slots:
    void gotoContainerAndCloseSubcontainers(int id);

private:
    void refreshVisible();

    QVector<GUIContainerObject*> mContainerObjectPtrs;
    QVector<QPushButton*> mPushButtonPtrs;
    QButtonGroup *mpButtonGroup;
};

#endif // QUICKNAVIGATIONWIDGET_H
