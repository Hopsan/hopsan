//$Id$
#include "plotwidget.h"


PlotWidget::PlotWidget(QVector<double> xarray, QVector<double> yarray, QWidget *parent)
    : QMainWindow(parent)//QWidget(parent,Qt::Window)
{
    this->setAttribute(Qt::WA_DeleteOnClose);

    QWidget *centralwidget = new QWidget(this);

    //QGridLayout *grid = new QGridLayout(this);

    //Create the plot

    QString title = "Two Curves";
    //VariablePlot *varPlot = new VariablePlot(this);
    mpVariablePlot = new VariablePlot(centralwidget);

    // Create and add curves to the plot
    mpCurve = new QwtPlotCurve("Curve 1");
    QwtArrayData data(xarray,yarray);
    mpCurve->setData(data);
    mpCurve->attach(mpVariablePlot);
    mpCurve->setRenderHint(QwtPlotItem::RenderAntialiased);
    mpVariablePlot->replot();

    //Create the close button
    QDialogButtonBox *buttonbox = new QDialogButtonBox(QDialogButtonBox::Close);

    //Add the plot to the grid
//    grid->addWidget(varPlot,0,0);
//    grid->addWidget(buttonbox,1,0);
//
//    centralwidget->setLayout(grid);
    this->setCentralWidget(mpVariablePlot);

    //Create toolbar and toolbutton
    QToolBar *toolBar = new QToolBar(this);

    btnZoom = new QToolButton(toolBar);
    btnZoom->setText("Zoom");
    btnZoom->setIcon(QIcon("../../HopsanGUI/icons/zoom.png"));
    btnZoom->setCheckable(true);
    btnZoom->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    toolBar->addWidget(btnZoom);

    btnPan = new QToolButton(toolBar);
    btnPan->setText("Pan");
    btnPan->setIcon(QIcon("../../HopsanGUI/icons/pan.png"));
    btnPan->setCheckable(true);
    btnPan->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    toolBar->addWidget(btnPan);

    btnSVG = new QToolButton(toolBar);
    btnSVG->setText("svg");
    btnSVG->setIcon(QIcon("../../HopsanGUI/icons/save_svg.png"));
    btnSVG->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    toolBar->addWidget(btnSVG);

    btnGrid = new QToolButton(toolBar);
    btnGrid->setText("Grid");
    btnGrid->setIcon(QIcon("../../HopsanGUI/icons/grid.png"));
    btnGrid->setCheckable(true);
    btnGrid->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    toolBar->addWidget(btnGrid);

    addToolBar(toolBar);

    //Zoom
    zoomer = new QwtPlotZoomer( QwtPlot::xBottom, QwtPlot::yLeft, mpVariablePlot->canvas());
    zoomer->setSelectionFlags(QwtPicker::DragSelection | QwtPicker::CornerToCorner);
    zoomer->setRubberBand(QwtPicker::RectRubberBand);
    zoomer->setRubberBandPen(QColor(Qt::green));
    zoomer->setTrackerMode(QwtPicker::ActiveOnly);
    zoomer->setTrackerPen(QColor(Qt::white));
    zoomer->setMousePattern(QwtEventPattern::MouseSelect2, Qt::RightButton, Qt::ControlModifier);
    zoomer->setMousePattern(QwtEventPattern::MouseSelect3, Qt::RightButton);

    //Panner
    panner = new QwtPlotPanner(mpVariablePlot->canvas());
    panner->setMouseButton(Qt::MidButton);

    //grid
    grid = new QwtPlotGrid;
    grid->enableXMin(true);
    grid->enableYMin(true);
    grid->setMajPen(QPen(Qt::black, 0, Qt::DotLine));
    grid->setMinPen(QPen(Qt::gray, 0 , Qt::DotLine));
    grid->attach(mpVariablePlot);
    grid->hide();;

    enableZoom(false);

    //Establish signal and slots connections
    connect(buttonbox, SIGNAL(rejected()), this, SLOT(close()));
    connect(btnZoom,SIGNAL(toggled(bool)),SLOT(enableZoom(bool)));
    connect(btnPan,SIGNAL(toggled(bool)),SLOT(enablePan(bool)));
    connect(btnSVG,SIGNAL(clicked()),SLOT(exportSVG()));
    connect(btnGrid,SIGNAL(toggled(bool)),SLOT(enableGrid(bool)));


    resize(600,600);
}

void PlotWidget::enableZoom(bool on)
{
    zoomer->setEnabled(on);
    zoomer->zoom(0);

    panner->setEnabled(on);
    panner->setMouseButton(Qt::MidButton);

    btnPan->setChecked(false);
}

void PlotWidget::enablePan(bool on)
{
    panner->setEnabled(on);
    panner->setMouseButton(Qt::LeftButton);

    btnZoom->setChecked(false);
}

void PlotWidget::enableGrid(bool on)
{
    if (on)
    {
        grid->show();
    }
    else
    {
        grid->hide();
    }

}

void PlotWidget::exportSVG()
{
#ifdef QT_SVG_LIB
#ifndef QT_NO_FILEDIALOG
     QString fileName = QFileDialog::getSaveFileName(
        this, "Export File Name", QString(),
        "SVG Documents (*.svg)");
#endif
    if ( !fileName.isEmpty() )
    {
        QSvgGenerator generator;
        generator.setFileName(fileName);
        generator.setSize(QSize(800, 600));

        mpVariablePlot->print(generator);
    }
#endif
}


VariablePlot::VariablePlot(QWidget *parent)
        : QwtPlot(parent)
{
    this->setAcceptDrops(true);
    //Set color for plot background
    setCanvasBackground(QColor(Qt::white));

//    QwtPlotMarker *d_mrk1 = new QwtPlotMarker();
//    d_mrk1->setValue(0.0, 0.0);
//    d_mrk1->setLineStyle(QwtPlotMarker::VLine);
//    d_mrk1->setLabelAlignment(Qt::AlignRight | Qt::AlignBottom);
//    d_mrk1->setLinePen(QPen(Qt::green, 0, Qt::DashDotLine));
//    d_mrk1->attach(this);

    setAutoReplot(true);
}

void VariablePlot::dragMoveEvent(QDragMoveEvent *event)
{
    std::cout << "apa" << std::endl;
    if (event->mimeData()->hasFormat("application/x-plotvariable"))
    {
        event->accept();
    }
    else
    {
        event->ignore();
    }
}

void VariablePlot::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasFormat("application/x-plotvariable"))
    {
        QByteArray *data = new QByteArray;
        *data = event->mimeData()->data("application/x-plotvariable");

        QDataStream stream(data,QIODevice::ReadOnly);

        QString functionname;
        stream >> functionname;

        event->accept();

        std::cout << functionname.toStdString();

        delete data;

    }
}


VariableList::VariableList(QWidget *parent)
        : QListWidget(parent)
{
    //Populate the list
    //QListWidgetItem *sinus = new QListWidgetItem(tr("linear1"),this);
    //QListWidgetItem *cosinus = new QListWidgetItem(tr("linear2"),this);

    // Store values for two functions, only for testing

    map["linear1"] = 1.0;

    map["linear2"] = 2.0;

    connect(this,SIGNAL(itemDoubleClicked(QListWidgetItem*)),this,SLOT(createPlot(QListWidgetItem*)));
}

void VariableList::createPlot(QListWidgetItem *item)
{
    double n = map.value(item->text());
    std::cout << n << std::endl;

    QVector<double> xarray(2);
    QVector<double> yarray(2);
    xarray[0] = 0.0;
    xarray[1] = 10.0*n;
    yarray[0] = 0.0;
    yarray[1] = 10.0*n;

    PlotWidget *plotwidget = new PlotWidget(xarray,yarray,this);
    plotwidget->show();

    std::cout << item->text().toStdString() << std::endl;
}

void VariableList::mousePressEvent(QMouseEvent *event)
{
    QListWidget::mousePressEvent(event);

    if (event->button() == Qt::LeftButton)
        dragStartPosition = event->pos();
}

void VariableList::mouseMoveEvent(QMouseEvent *event)
{

    if (!(event->buttons() & Qt::LeftButton))
        return;
    if ((event->pos() - dragStartPosition).manhattanLength()
         < QApplication::startDragDistance())
        return;

    QByteArray *data = new QByteArray;
    QDataStream stream(data,QIODevice::WriteOnly);

    QListWidgetItem *item = this->currentItem();

    stream << item->text();

    QString mimeType = "application/x-plotvariable";

    QDrag *drag = new QDrag(this);
    QMimeData *mimeData = new QMimeData;

    mimeData->setData(mimeType, *data);
    drag->setMimeData(mimeData);

    drag->setHotSpot(QPoint(drag->pixmap().width()/2, drag->pixmap().height()));

    //Qt::DropAction dropAction = drag->exec(Qt::CopyAction | Qt::MoveAction);

}


VariableListDialog::VariableListDialog(QWidget *parent)
        : QWidget(parent)
{
    //Create a grid
    QGridLayout *grid = new QGridLayout(this);

    //Create the plotvariables list
    VariableList *varList = new VariableList(this);

    grid->addWidget(varList,0,0);
}
