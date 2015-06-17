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

#ifndef EDITORWIDGET_H
#define EDITORWIDGET_H

#include <QLabel>

#include "Widgets/TextEditor.h"

class XmlHighlighter;
class CppHighlighter;
class Configuration;

class EditorWidget : public QWidget
{
    Q_OBJECT
public:
    enum HighlighterTypeEnum {XML, Cpp};

    explicit EditorWidget(Configuration *pConfiguration, QWidget *parent = 0);
    void setText(const QString &text, HighlighterTypeEnum type, bool editingEnabled=true);
    QString getText() const;
    void clear();

public slots:
    void update();

signals:
    void textChanged();

private:
    QLabel *mpNotEditableLabel;
    TextEditor *mpTextEdit;
    XmlHighlighter *mpXmlHighlighter;
    CppHighlighter *mpCppHighlighter;
    Configuration *mpConfiguration;
};

#endif // EDITORWIDGET_H
