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

#ifndef DATAEXPLORER_H
#define DATAEXPLORER_H

#include <QWidget>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QMap>
#include <QDialog>
#include <QPointer>

//Forward declarations
class LogDataHandler2;
class GenerationItem;

class DataExplorer : public QDialog
{
    Q_OBJECT
public:
    explicit DataExplorer(QWidget *parent = 0);

signals:

public slots:
    void setLogdataHandler(LogDataHandler2 *pLogDataHandler);
    void refresh();
    void openImportDataDialog();
    void openExportDataDialog();

private slots:
    void refreshGenerationList();
    void refreshDataList();
    void removeSelectedGenerations();
    void removeGeneration(int gen);
    void toggleSelectAllGenerations();

private:
    QVector<int> gensFromSelected();
    QPointer<LogDataHandler2> mpLogDataHandler;
    QWidget *mpGenerationsListWidget;
    QScrollArea *mpGenerationsScrollArea;
    QMap<int, GenerationItem*> mGenerationItemMap;
    bool mAllSelectedToggle;

};

#endif // DATAEXPLORER_H
