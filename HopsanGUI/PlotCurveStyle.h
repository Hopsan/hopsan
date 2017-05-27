#ifndef PLOTCURVESTYLE_H
#define PLOTCURVESTYLE_H

#include <QColor>
#include <QString>
#include <array>
#include <qwt/qwt_symbol.h>

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
