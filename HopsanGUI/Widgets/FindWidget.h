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

//$Id$

#ifndef FINDWIDGET_H
#define FINDWIDGET_H

#include <QWidget>
#include <QString>
#include <QPointer>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QCheckBox>

// Forward declaration
class ContainerObject;

class FindWidget : public QWidget
{
    Q_OBJECT
public:
    explicit FindWidget(QWidget *parent = 0);
    void setContainer(ContainerObject *pContainer);

signals:

public slots:
    void find();
    void findComponent(const QString &rName, const bool centerView=true, Qt::CaseSensitivity caseSensitivity=Qt::CaseInsensitive, bool wildcard=true);
    void findAlias(const QString &rName, const bool centerView=true, Qt::CaseSensitivity caseSensitivity=Qt::CaseInsensitive, bool wildcard=true);
    void findSystemParameter(const QString &rName, const bool centerView=true, Qt::CaseSensitivity caseSensitivity=Qt::CaseInsensitive, bool wildcard=true);
    void findSystemParameter(const QStringList &rNames, const bool centerView=true, Qt::CaseSensitivity caseSensitivity=Qt::CaseInsensitive, bool wildcard=true);
    void findAny(const QString &rName);

public slots:
    virtual void setVisible(bool visible);
    virtual void keyPressEvent(QKeyEvent *event);

private:
    void clearHighlights();
    QLineEdit* mpFindLineEdit;
    QComboBox* mpFindWhatComboBox;
    QPushButton* mpFindButton;
    QCheckBox* mpCaseSensitivityCheckBox;
    QCheckBox* mpWildcardCheckBox;

    QPointer<ContainerObject> mpContainer;

};

#endif // FINDWIDGET_H
