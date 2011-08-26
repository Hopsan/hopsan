#include "plotterbase.h"
#include "axisbase.h"


namespace QSint
{


PlotterBase::PlotterBase(QWidget *parent) :
    QWidget(parent)
{
    m_axisX = new AxisBase(Qt::Horizontal, this);
    m_axisY = new AxisBase(Qt::Vertical, this);

    setModel(0);
}


void PlotterBase::setBorderPen(const QPen &pen)
{
    m_pen = pen;
}

void PlotterBase::setBackground(const QBrush &brush)
{
    m_bg = brush;
}


QRect PlotterBase::dataRect()
{
    QRect p_rect(rect());

    if (m_axisX)
        p_rect.setBottom(p_rect.bottom() - m_axisX->offset());

    if (m_axisY)
        p_rect.setLeft(p_rect.left() + m_axisY->offset());

    return p_rect;
}


void PlotterBase::setModel(QAbstractItemModel *model)
{
    m_model = model;

    if (m_axisX)
        m_axisX->setModel(model);

    if (m_axisY)
        m_axisY->setModel(model);

    if (m_model)
    {
        connect(m_model, SIGNAL(dataChanged(const QModelIndex &,const QModelIndex &)),
                this, SLOT(repaint()));

        connect(m_model, SIGNAL(headerDataChanged(Qt::Orientation, int, int)),
                this, SLOT(repaint()));

        connect(m_model, SIGNAL(columnsInserted(const QModelIndex &, int, int)),
                this, SLOT(repaint()));

        connect(m_model, SIGNAL(columnsRemoved(const QModelIndex &, int, int)),
                this, SLOT(repaint()));

        connect(m_model, SIGNAL(rowsInserted(const QModelIndex &, int, int)),
                this, SLOT(repaint()));

        connect(m_model, SIGNAL(rowsRemoved(const QModelIndex &, int, int)),
                this, SLOT(repaint()));
    }
}


void PlotterBase::paintEvent(QPaintEvent *)
{
    QPainter p(this);

    drawBackground(p);

    drawAxis(p);

    drawContent(p);

    drawForeground(p);
}

void PlotterBase::drawBackground(QPainter &p)
{
    p.fillRect(rect(), m_bg);
}

void PlotterBase::drawForeground(QPainter &p)
{
    p.setOpacity(1);
    p.setPen(m_pen);
    p.setBrush(Qt::NoBrush);
    p.drawRect(rect().adjusted(0,0,-1,-1));
}

void PlotterBase::drawAxis(QPainter &p)
{
    if (m_axisX)
        m_axisX->draw(p);

    if (m_axisY)
        m_axisY->draw(p);
}


} // namespace
