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
//! @file   SSVEditorWidget.h
//! @brief A widget for editing SSV parameter sets
//! @author Robert Braun <robert.braun@liu.se>
//!
//$Id$

#ifndef SSVEDITORWIDGET_H
#define SSVEDITORWIDGET_H

#include <QWidget>
#include <QTableWidget>
#include <QTableWidgetItem>

// Forward declarations
class ssvParameterSetHandle;
class QPushButton;

class SSVEditorWidget : public QWidget
{
    Q_OBJECT

public:
    SSVEditorWidget(ssvParameterSetHandle *pSsv, QString fileName, QWidget *parent = nullptr);
    ~SSVEditorWidget();

    void loadSSVData();

protected slots:
    void addParameter();
    void removeParameter();

private:
    ssvParameterSetHandle *mpSsv;
    QString mFileName;
    QTableWidget *mpTable;
    QPushButton *mpAddButton;
    QPushButton *mpRemoveButton;
};

#endif // SSVEDITORWIDGET_H
