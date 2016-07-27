#ifndef PLOTCURVESTYLE_H
#define PLOTCURVESTYLE_H

#include <QColor>

class PlotCurveStyle
{
public:
    PlotCurveStyle() {}

    PlotCurveStyle(QColor color)
    {
        this->color = color;
        this->thickness = thickness;
        this->type = type;
        this->symbol = symbol;
    }

    PlotCurveStyle(QString colorStr)
    {
        this->color = QColor(colorStr);
        this->thickness = thickness;
        this->type = type;
        this->symbol = symbol;
    }

    QColor color = QColor();
    int thickness = 2;
    int type = 1;
    int symbol = 0;
};

#endif // PLOTCURVESTYLE_H
