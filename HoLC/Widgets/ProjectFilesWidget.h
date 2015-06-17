/*-----------------------------------------------------------------------------
 This source file is a part of Hopsan

 Copyright (c) 2009 to present year, Hopsan Group

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

 For license details and information about the Hopsan Group see the files
 GPLv3 and HOPSANGROUP in the Hopsan source code root directory

 For author and contributor information see the AUTHORS file
-----------------------------------------------------------------------------*/

#ifndef PROJECTFILESWIDGET_H
#define PROJECTFILESWIDGET_H

#include <QWidget>
#include <QTreeWidget>

#include "Handlers/FileHandler.h"

class ProjectFilesWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ProjectFilesWidget(QWidget *parent = 0);

    QTreeWidget *mpTreeWidget;

signals:
    void deleteRequested(QTreeWidgetItem*);

public slots:
    QTreeWidgetItem *addFile(const FileObject *pFile);
    void addAsterisk();
    void removeAsterisks();
    void removeItem(QTreeWidgetItem *pItem);
    void clear();
    void update();

protected:
    void keyPressEvent(QKeyEvent *event);

private:
    QTreeWidgetItem *mpProjectFilesTopLevelItem;
    QTreeWidgetItem *mpComponentFilesTopLevelItem;
    QTreeWidgetItem *mpAuxiliaryFilesTopLevelItem;
    QTreeWidgetItem *mpAppearanceFilesTopLevelItem;
};

#endif // PROJECTFILESTREE_H
