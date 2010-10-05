/*
 * This file is part of OpenModelica.
 *
 * Copyright (c) 1998-CurrentYear, Linköping University,
 * Department of Computer and Information Science,
 * SE-58183 Linköping, Sweden.
 *
 * All rights reserved.
 *
 * THIS PROGRAM IS PROVIDED UNDER THE TERMS OF GPL VERSION 3 
 * AND THIS OSMC PUBLIC LICENSE (OSMC-PL). 
 * ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS PROGRAM CONSTITUTES RECIPIENT'S  
 * ACCEPTANCE OF THE OSMC PUBLIC LICENSE.
 *
 * The OpenModelica software and the Open Source Modelica
 * Consortium (OSMC) Public License (OSMC-PL) are obtained
 * from Linköping University, either from the above address,
 * from the URLs: http://www.ida.liu.se/projects/OpenModelica or  
 * http://www.openmodelica.org, and in the OpenModelica distribution. 
 * GNU version 3 is obtained from: http://www.gnu.org/copyleft/gpl.html.
 *
 * This program is distributed WITHOUT ANY WARRANTY; without
 * even the implied warranty of  MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE, EXCEPT AS EXPRESSLY SET FORTH
 * IN THE BY RECIPIENT SELECTED SUBSIDIARY LICENSE CONDITIONS
 * OF OSMC-PL.
 *
 * See the full OSMC Public License conditions for more details.
 *
 */

/*
 * HopsanGUI
 * Fluid and Mechatronic Systems, Department of Management and Engineering, Linköping University
 * Main Authors 2009-2010:  Robert Braun, Björn Eriksson, Peter Nordin
 * Contributors 2009-2010:  Mikael Axin, Alessandro Dell'Amico, Karl Pettersson, Ingo Staack
 */

#ifndef GLOBALPARAMETERSWIDGET_H
#define GLOBALPARAMETERSWIDGET_H

#include <QList>
#include <QStringList>
#include <QTableWidget>
#include <QPushButton>
#include <QDialog>
#include <QTableWidget>
#include <QObject>
#include <QGridLayout.h>
#include <qwidget.h>
#include <qlabel.h>

    //Forward Declarations
class GUIObject;
class GraphicsView;
class GUIConnector;
class MainWindow;
class GUISystem;


class GlobalParametersWidget : public QWidget
{
    Q_OBJECT
public:
    GlobalParametersWidget(MainWindow *parent = 0);

    MainWindow *mpParentMainWindow;

    QTableWidget *mpGlobalParametersTable;
    QPushButton *mpAddButton;
    QPushButton *mpRemoveButton;
    QPushButton *mpCloseButton;

    QGridLayout *mpGridLayout;

    QMap<QString,double> mGlobalParametersMap;

public slots:
    void setParameter(QString name, double value);

private slots:
    void openParameterDialog();
    void addParameter();
    void update();

private:
    QLabel *mpNameLabel;
    QLineEdit *mpNameBox;
    QLabel *mpValueLabel;
    QLineEdit *mpValueBox;
    QPushButton *mpAddInDialogButton;
    QPushButton *mpDoneInDialogButton;
    QList< QPair<QString, double> > mContents;
};


#endif // GLOBALPARAMETERSWIDGET_H
