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
 * Main Authors 2009-2010:  Robert Braun, Björn Eriksson, Peter Nordin
 * Contributors 2009-2010:  Mikael Axin, Alessandro Dell'Amico, Karl Pettersson, Ingo Staack
 * Fluid and Mechatronic Systems, Department of Management and Engineering, Linköping University
 * Based on: HopsanGUI
 */

#include "treewidget.h"
#include <iostream>

TreeWidget::TreeWidget(QWidget *parent)
        : QTreeWidget(parent)
{
    //this->setDragEnabled(true);

    this->connect(this,SIGNAL(itemActivated(QTreeWidgetItem*,int)),SLOT(showList(QTreeWidgetItem*,int)));
}

TreeWidget::~TreeWidget()
{
}


/*void TreeWidget::startDrag(Qt::DropActions)
{

    data = new QByteArray;
    stream = new QDataStream( data, QIODevice::WriteOnly);
    mimeData = new QMimeData;
    //drag = new QDrag(this);

    QTreeWidgetItem *item;
    item = this->currentItem();

    if (item->parent() == 0)
        return;

    *stream << item->text(1);

    QString mimeType = "application/x-text";
    mimeData->setData( mimeType, *data);

    drag->setMimeData(mimeData);
    drag->exec(Qt::CopyAction);

    delete data;
    delete stream;
    delete mimeData;
    //delete drag;
    delete item;
}
*/


/*void TreeWidget::mousePressEvent(QMouseEvent *event)
{
    QTreeWidget::mousePressEvent(event);

    if (event->button() == Qt::LeftButton)
        dragStartPosition = event->pos();
}


void TreeWidget::mouseMoveEvent(QMouseEvent *event)
{

    if (!(event->buttons() & Qt::LeftButton))
        return;
    if ((event->pos() - dragStartPosition).manhattanLength()
         < QApplication::startDragDistance())
        return;

    QByteArray *data = new QByteArray;
    QDataStream stream(data,QIODevice::WriteOnly);

    QTreeWidgetItem *item = this->currentItem();//this->currentItem();
    if (item->parent() == 0)
            return;

    stream << item->text(0) << item->data(0,Qt::UserRole).toString();

    QString mimeType = "application/x-text";

    QDrag *drag = new QDrag(this);
    QMimeData *mimeData = new QMimeData;

    mimeData->setData(mimeType, *data);
    drag->setMimeData(mimeData);

    Qt::DropAction dropAction = drag->exec(Qt::CopyAction | Qt::MoveAction);

    //delete data;
    //delete mimeData;
    //delete item;
}*/

void TreeWidget::showList(QTreeWidgetItem *item,int num)
{
    QTreeWidgetItemIterator itemiterator(this);
    while (*itemiterator) {
        ((TreeWidgetItem*)(itemiterator.operator *()))->getList()->hide();
        ++itemiterator;
    }

    ((TreeWidgetItem*)item)->getList()->show();

}
