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

#ifndef PLOTCURVESTYLE_H
#define PLOTCURVESTYLE_H

#include <QColor>
#include <QString>
#include <array>
#include "qwt_symbol.h"

class PlotCurveStyle
{
public:
    static const std::array<QString, 16> symbol_names;
    static const std::array<QwtSymbol::Style, 16> symbol_enums;
    static QString toScriptFriendlySymbolName(const QString &symbol_name)
    {
        // Script friendly names are all lowercase with white spaces removed
        return symbol_name.toLower().replace(' ', "");
    }

    static QString toSymbolName(const QwtSymbol::Style enum_value)
    {
        for (size_t i=0; i<symbol_enums.size(); ++i) {
            if (enum_value == symbol_enums[i]) {
                return symbol_names[i];
            }
        }
        return symbol_names.front();
    }

    static int toSymbolIndex(const QwtSymbol::Style enum_value)
    {
        for (size_t i=0; i<symbol_enums.size(); ++i) {
            if (enum_value == symbol_enums[i]) {
                return i;
            }
        }
        return 0;
    }

    static QwtSymbol::Style toSymbolEnum(const QString &symbol_name)
    {
        for (size_t i=0; i<symbol_names.size(); ++i) {
            if (symbol_name == symbol_names[i]) {
                return symbol_enums[i];
            }
        }

        for (size_t i=0; i<symbol_names.size(); ++i) {
            // Also try to convert from "script friendly names"
            if (symbol_name == toScriptFriendlySymbolName(symbol_names[i])) {
                return symbol_enums[i];
            }
        }

        return QwtSymbol::NoSymbol;
    }

    static int toLineStyleIndex(const Qt::PenStyle enum_value) {
        // We ignore NoPen unless thats what we want
        if (enum_value != Qt::PenStyle::NoPen) {
            return static_cast<int>(enum_value) - 1;
        }
        return static_cast<int>(enum_value);
        //! @todo Solve LineStyle the same way as symbols above, encapsulate it in this class
    }


    PlotCurveStyle() {}

    PlotCurveStyle(QColor color)
    {
        this->color = color;
    }

    PlotCurveStyle(QString colorStr)
    {
        this->color = QColor(colorStr);
    }

    QColor color = QColor();
    int thickness = 2;
    Qt::PenStyle line_style = Qt::SolidLine;
    QwtSymbol::Style symbol = QwtSymbol::NoSymbol;
};

#endif // PLOTCURVESTYLE_H
