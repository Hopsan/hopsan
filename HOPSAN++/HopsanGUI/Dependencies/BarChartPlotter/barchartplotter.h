#ifndef BARCHARTPLOTTER_H
#define BARCHARTPLOTTER_H

#include "plotterbase.h"


namespace QSint
{


class BarChartPlotter : public PlotterBase
{
    Q_OBJECT
public:
    explicit BarChartPlotter(QWidget *parent = 0);


    enum BarChartType { Stacked, Columns };

    void setBarType(BarChartType type);
    inline BarChartType barType() const { return m_type; }


    void setBarSize(int min, int max = INT_MAX);
    inline int barSizeMin() const { return m_barsize_min; }
    inline int barSizeMax() const { return m_barsize_max; }

    void setBarScale(double scale);
    inline double barScale() const { return m_scale; }

    void setBarOpacity(double value);
    inline double barOpacity() const { return m_opacity; }

protected:
    virtual void drawContent(QPainter &p);

    virtual void drawBarItem(QPainter &p, QRect rect,
                             QPen &pen, QBrush &brush,
                             const QModelIndex &index,
                             double value);

    int m_barsize_min, m_barsize_max;
    double m_scale;
    double m_opacity;

    BarChartType m_type;
};


} // namespace

#endif // BARCHARTPLOTTER_H
