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

//$Id$

#ifndef FINDWIDGET_H
#define FINDWIDGET_H

#include <QDialog>
#include <QString>
#include <QPointer>
#include <QLineEdit>

// Forward declaration
class ContainerObject;

class FindHelper : public QWidget
{
    Q_OBJECT
public:
    FindHelper(QWidget *pParent);
private:
    QLineEdit *mpLineEdit;
private slots:
    void doFind();
signals:
    void find(QString name);
};


class FindWidget : public QDialog
{
    Q_OBJECT
public:
    explicit FindWidget(QWidget *parent = 0);
    void setContainer(ContainerObject *pContainer);

signals:

public slots:
    void findComponent(const QString &rName, const bool centerView=true);
    void findAlias(const QString &rName, const bool centerView=true);
    void findSystemParameter(const QStringList &rNames, const bool centerView=true);
    void findAny(const QString &rName);

private:
    void clearHighlights();
    QPointer<ContainerObject> mpContainer;

};

#endif // FINDWIDGET_H
