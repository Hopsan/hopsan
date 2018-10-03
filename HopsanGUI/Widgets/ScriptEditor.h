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
//! @file   ScriptEditor.h
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2018-10-01
//!
//! @brief Contains the script editor class
//!


#ifndef SCRIPTEDITOR_H
#define SCRIPTEDITOR_H

#include <QWidget>
#include <QTextEdit>
#include <QFileInfo>

#include "common.h"

class ScriptEditor : public QWidget
{
    Q_OBJECT
public:
    explicit ScriptEditor(QFileInfo file, QWidget *parent = nullptr);

    QFileInfo getScriptFileInfo() const { return mScriptFileInfo; }
    bool isSaved() const { return mIsSaved; }

protected:
    void wheelEvent(QWheelEvent* event);

signals:

private:
    QTextEdit *mpEditor;
    QFileInfo mScriptFileInfo;
    bool mIsSaved = true;
    QString mSavedText;

public slots:
    void saveAs();
    void cut();
    void copy();
    void paste();
    void undo();
    void redo();
    void zoomIn();
    void zoomOut();
    void print();
    void save(SaveTargetEnumT saveAsFlag=ExistingFile);

private slots:
    void hasChanged();
};

#endif // SCRIPTEDITOR_H
