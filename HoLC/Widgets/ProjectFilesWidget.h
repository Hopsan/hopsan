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

#ifndef PROJECTFILESWIDGET_H
#define PROJECTFILESWIDGET_H

#include <QWidget>
#include <QTreeWidget>

#include "Handlers/FileHandler.h"

class ProjectFilesWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ProjectFilesWidget(QWidget *parent = nullptr);

    QTreeWidget *mpTreeWidget;

signals:
    void deleteRequested(QTreeWidgetItem*);

public slots:
    QTreeWidgetItem *addFile(const QSharedPointer<FileObject> pFile);
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
