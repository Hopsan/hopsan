#include "axisbase.h"
#include "plotterbase.h"


namespace QSint
{


AxisBase::AxisBase(Qt::Orientation orient, PlotterBase *parent) :
    QObject(parent),
    m_orient(orient)
{
    setTicks(0, 10);
    setRanges(0, 100);
    setOffset(30);
    setType(AxisData);

    setMinorGridPen(Qt::NoPen);
    setMajorGridPen(Qt::NoPen);

    setModel(0);
}


void AxisBase::setType(AxisBase::AxisType type)
{
    m_type = type;
}

void AxisBase::setModel(QAbstractItemModel *model)
{
    m_model = model;
}


void AxisBase::setRanges(double min, double max)
{
    m_min = min;
    m_max = qMax(max, m_min);
}

void AxisBase::setTicks(double minor, double major)
{
    m_minor = qMax(0.0, minor);
    m_major = qMax(m_minor, major);
}

void AxisBase::setOffset(int offset)
{
    m_offset = qMax(0, offset);
}

void AxisBase::setFont(const QFont &font)
{
    m_font = font;
}

void AxisBase::setTextColor(const QColor &color)
{
    m_textColor = color;
}

void AxisBase::setPen(const QPen &pen)
{
    m_pen = pen;
}

void AxisBase::setMinorTicksPen(const QPen &pen)
{
    m_minorPen = pen;
}

void AxisBase::setMajorTicksPen(const QPen &pen)
{
    m_majorPen = pen;
}

void AxisBase::setMinorGridPen(const QPen &pen)
{
    m_minorGridPen = pen;
}

void AxisBase::setMajorGridPen(const QPen &pen)
{
    m_majorGridPen = pen;
}


void AxisBase::calculatePoints(int &p_start, int &p_end)
{
    PlotterBase *plotter = (PlotterBase*)parent();
    QRect rect(plotter->dataRect());

    switch (m_orient)
    {
        case Qt::Vertical:
        {
            p_start = rect.top() + 1;
            p_end = rect.bottom();
            if (p_end <= p_start) {
                p_start = 0;
                p_end = rect.height();
            }
            break;
        }

        case Qt::Horizontal:
        {
            p_start = rect.left();
            p_end = rect.right() - 1;
            if (p_end <= p_start) {
                p_start = 0;
                p_end = rect.width();
            }
            break;
        }
    }
}

int AxisBase::toView(double value)
{
    int p_start, p_end;
    calculatePoints(p_start, p_end);

    // add 5% to ensure that everything fits
    double p10 = (m_max - m_min) * 0.05;

    double d = (value - m_min) / (m_max - m_min + p10);

    switch (m_orient)
    {
        case Qt::Vertical:
            return p_end - d * (p_end - p_start);

        case Qt::Horizontal:
            return d * (p_end - p_start) + p_start;
    }

    return 0;
}


void AxisBase::draw(QPainter &p)
{
    switch (m_type)
    {
        case AxisModel:
            drawAxisModel(p);
            break;

        default:
            drawAxisData(p);
            break;
    }
}

void AxisBase::drawAxisData(QPainter &p)
{
    PlotterBase *plotter = (PlotterBase*)parent();
    QRect rect(plotter->contentsRect());

    double start = m_min;
    double end = m_max;

    int p_start, p_end;
    calculatePoints(p_start, p_end);

    switch (m_orient)
    {
        case Qt::Vertical:
        {
            p.setPen(m_pen);
            p.drawLine(m_offset+2, p_start, m_offset+2, p_end);

            if (m_minor > 1e-100)
            {
                int prevTick = INT_MAX/2;

                for (double i = start; i <= end; i += m_minor)
                {
                    int p_d = toView(i);

                    if (p_d < prevTick-1)
                    {
                        prevTick = p_d;
                        p.setPen(m_minorPen);
                        p.drawLine(m_offset+1, p_d, m_offset+3, p_d);

                        if (m_minorGridPen != Qt::NoPen)
                        {
                            p.setPen(m_minorGridPen);
                            p.drawLine(m_offset+2, p_d, rect.right(), p_d);
                        }
                    }
                }
            }

            if (m_major > 1e-100)
            {
                QRect prevRect;
                int prevTick = INT_MAX/2;

                for (double i = start; i <= end; i += m_major)
                {
                    int p_d = toView(i);

                    if (p_d < prevTick-1)
                    {
                        prevTick = p_d;
                        p.setPen(m_majorPen);
                        p.drawLine(m_offset+0, p_d, m_offset+4, p_d);

                        if (m_majorGridPen != Qt::NoPen)
                        {
                            p.setPen(m_majorGridPen);
                            p.drawLine(m_offset+2, p_d, rect.right(), p_d);
                        }
                    }

                    QString text(QString::number(i));
                    QFontMetrics fm(m_font);
                    QRect textRect(fm.boundingRect(text));

                    int h = textRect.height();
                    QRect drawRect(0, p_d - h/2, m_offset, h);

                    // skip paining the text
                    if (prevRect.isValid() && prevRect.intersects(drawRect))
                        continue;
                    prevRect = drawRect;

                    p.setPen(QPen(m_textColor));
                    p.drawText(drawRect, Qt::AlignRight | Qt::AlignVCenter, text);
                }
            }

            break;
        }

        case Qt::Horizontal:
        {
            p.setPen(m_pen);
            p.drawLine(p_start, rect.height()-m_offset, p_end, rect.height()-m_offset);

            if (m_minor > 1e-100)
            {
                int prevTick = -INT_MAX;

                for (double i = start; i <= end; i += m_minor)
                {
                    int p_d = toView(i);

                    if (p_d > prevTick+1)
                    {
                        prevTick = p_d;
                        p.setPen(m_minorPen);
                        p.drawLine(p_d, rect.height()-m_offset+1, p_d, rect.height()-m_offset+3);

                        if (m_minorGridPen != Qt::NoPen)
                        {
                            p.setPen(m_minorGridPen);
                            p.drawLine(p_d, rect.top(), p_d, rect.height()-m_offset);
                        }
                    }
                }
            }

            if (m_major > 1e-100)
            {
                QRect prevRect;
                int prevTick = -INT_MAX;

                for (double i = start; i <= end; i += m_major)
                {
                    int p_d = toView(i);

                    if (p_d > prevTick+1)
                    {
                        prevTick = p_d;
                        p.setPen(m_majorPen);
                        p.drawLine(p_d, rect.height()-m_offset+0, p_d, rect.height()-m_offset+4);

                        if (m_majorGridPen != Qt::NoPen)
                        {
                            p.setPen(m_majorGridPen);
                            p.drawLine(p_d, rect.top(), p_d, rect.height()-m_offset);
                        }
                    }

                    QString text(QString::number(i));
                    QFontMetrics fm(m_font);
                    QRect textRect(fm.boundingRect(text));

                    int w = textRect.width();
                    QRect drawRect(p_d - w/2, rect.height()-m_offset+3, w, m_offset);

                    // skip paining the text
                    if (prevRect.isValid() && prevRect.intersects(drawRect))
                        continue;
                    prevRect = drawRect;

                    p.setPen(QPen(m_textColor));
                    p.drawText(drawRect, Qt::AlignCenter, text);
                }
            }

            break;

        }
    }
}

void AxisBase::drawAxisModel(QPainter &p)
{
    PlotterBase *plotter = (PlotterBase*)parent();
    QRect rect(plotter->contentsRect());

    int p_start, p_end;
    calculatePoints(p_start, p_end);

    switch (m_orient)
    {
        case Qt::Vertical:
        {
            p.setPen(m_pen);
            p.drawLine(m_offset+2, p_start, m_offset+2, p_end);

            if (m_model)
            {
            }

            break;
        }

        case Qt::Horizontal:
        {
            p.setPen(m_pen);
            p.drawLine(p_start, rect.height()-m_offset, p_end, rect.height()-m_offset);

            if (m_model)
            {
                QRect prevRect;

                int count = m_model->columnCount();

                int p_offs = (p_end - p_start) / count;
                int p_line_d = p_start + p_offs;

                for (int i = 0; i < count; i++)
                {
                    double d = (double)i / (double)count;
                    int p_d = d * (p_end - p_start) + p_start + p_offs/2;

                    p.setPen(m_majorPen);
                    p.drawLine(p_d, rect.height()-m_offset+0, p_d, rect.height()-m_offset+4);

                    if (m_majorGridPen != Qt::NoPen)
                    {
                        p.setPen(m_majorGridPen);
                        p.drawLine(p_line_d, rect.top(), p_line_d, rect.height()-m_offset);
                        p_line_d += p_offs;
                    }

                    QString text(m_model->headerData(i, m_orient).toString());
                    QFontMetrics fm(m_font);
                    QRect textRect(fm.boundingRect(text));

                    int w = textRect.width() + 4;
                    QRect drawRect(p_d - w/2, rect.height()-m_offset+3, w, m_offset);

                    // skip paining the text
                    if (prevRect.isValid() && prevRect.intersects(drawRect))
                        continue;
                    prevRect = drawRect;

                    p.setPen(QPen(m_textColor));
                    p.drawText(drawRect, Qt::AlignCenter, text);
                }
            }

            break;
        }

    }

}


} // namespace
