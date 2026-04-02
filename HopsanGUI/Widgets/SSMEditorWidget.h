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
//! @file   SSMEditorWidget.h
//! @brief A widget for editing SSM parameter mappings
//! @author Robert Braun <robert.braun@liu.se>
//!
//$Id$

#ifndef SSMEDITORGWIDGET_H
#define SSMEDITORGWIDGET_H

#include <QWidget>
#include <QTableWidget>
#include <QTableWidgetItem>

// Forward declarations
class ssmParameterMappingHandle;
class QPushButton;

class SSMEditorWidget : public QWidget
{
    Q_OBJECT

public:
    SSMEditorWidget(ssmParameterMappingHandle *pSsm, QString fileName, QWidget *parent = nullptr);
    ~SSMEditorWidget();

    void loadSSMData();

protected slots:
    void addMapping();
    void removeMapping();

private:
    ssmParameterMappingHandle *mpSsm;
    QString mFileName;
    QTableWidget *mpTable;
    QPushButton *mpAddButton;
    QPushButton *mpRemoveButton;
};

#endif // SSMEDITORGWIDGET_H
