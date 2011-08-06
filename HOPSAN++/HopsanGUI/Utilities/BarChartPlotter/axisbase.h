#ifndef AXISBASE_H
#define AXISBASE_H

#include <QtGui>


namespace QSint
{


class PlotterBase;

class AxisBase : public QObject
{
    Q_OBJECT
public:
    explicit AxisBase(Qt::Orientation orient, PlotterBase *parent);


    enum AxisType { AxisData, AxisTime, AxisModel };

    void setType(AxisType type);
    inline AxisType type() const { return m_type; }


    void setRanges(double min, double max);
    inline double rangeMininum() const { return m_min; }
    inline double rangeMaximum() const { return m_max; }

    void setTicks(double minor, double major);
    inline double minorTicks() const { return m_minor; }
    inline double majorTicks() const { return m_major; }

    void setOffset(int offset);
    inline int offset() const { return m_offset; }


    void setFont(const QFont &font);
    inline const QFont& font() const { return m_font; }

    void setTextColor(const QColor &color);
    inline const QColor& textColor() const { return m_textColor; }

    void setPen(const QPen &pen);
    inline const QPen& pen() const { return m_pen; }

    void setMinorTicksPen(const QPen &pen);
    inline const QPen& minorTicksPen() const { return m_minorPen; }

    void setMajorTicksPen(const QPen &pen);
    inline const QPen& majorTicksPen() const { return m_majorPen; }

    void setMinorGridPen(const QPen &pen);
    inline const QPen& minorGridPen() const { return m_minorGridPen; }

    void setMajorGridPen(const QPen &pen);
    inline const QPen& majorGridPen() const { return m_majorGridPen; }


    void setModel(QAbstractItemModel *model);
    inline QAbstractItemModel* model() const { return m_model; }


    virtual void calculatePoints(int &p_start, int &p_end);

    virtual int toView(double value);


    virtual void draw(QPainter &p);

protected:
    virtual void drawAxisData(QPainter &p);
    virtual void drawAxisModel(QPainter &p);

    Qt::Orientation m_orient;

    QAbstractItemModel *m_model;

    double m_min, m_max;
    double m_minor, m_major;
    int m_offset;

    QFont m_font;

    QPen m_pen, m_minorPen, m_majorPen, m_minorGridPen, m_majorGridPen;
    QColor m_textColor;

    AxisType m_type;
};


} // namespace

#endif // AXISBASE_H
