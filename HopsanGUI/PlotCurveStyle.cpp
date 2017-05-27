#include "PlotCurveStyle.h"

const std::array<QString, 16> PlotCurveStyle::symbol_names = {
    "None",
    "Cross",
    "Ellipse",
    "XCross",
    "Triangle",
    "Rectangle",
    "Diamond",
    "Down Triangle",
    "Up Triangle",
    "Right Triangle",
    "Left Triangle",
    "Hexagon",
    "Horizontal Line",
    "Vertical Line",
    "Star 1",
    "Star 2",
};

const std::array<QwtSymbol::Style, 16> PlotCurveStyle::symbol_enums = {
    QwtSymbol::NoSymbol,
    QwtSymbol::Cross,
    QwtSymbol::Ellipse,
    QwtSymbol::XCross,
    QwtSymbol::Triangle,
    QwtSymbol::Rect,
    QwtSymbol::Diamond,
    QwtSymbol::DTriangle,
    QwtSymbol::UTriangle,
    QwtSymbol::RTriangle,
    QwtSymbol::LTriangle,
    QwtSymbol::Hexagon,
    QwtSymbol::HLine,
    QwtSymbol::VLine,
    QwtSymbol::Star1,
    QwtSymbol::Star2,
};
