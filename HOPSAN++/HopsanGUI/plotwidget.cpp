/*
 * This file is part of OpenModelica.
 *
 * Copyright (c) 1998-CurrentYear, Linköping University,
 * Department of Computer and Information Science,
 * SE-58183 Linköping, Sweden.
 *
 * All rights reserved.
 *
 * THIS PROGRAM IS PROVIDED UNDER THE TERMS OF GPL VERSION 3 
 * AND THIS OSMC PUBLIC LICENSE (OSMC-PL). 
 * ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS PROGRAM CONSTITUTES RECIPIENT'S  
 * ACCEPTANCE OF THE OSMC PUBLIC LICENSE.
 *
 * The OpenModelica software and the Open Source Modelica
 * Consortium (OSMC) Public License (OSMC-PL) are obtained
 * from Linköping University, either from the above address,
 * from the URLs: http://www.ida.liu.se/projects/OpenModelica or  
 * http://www.openmodelica.org, and in the OpenModelica distribution. 
 * GNU version 3 is obtained from: http://www.gnu.org/copyleft/gpl.html.
 *
 * This program is distributed WITHOUT ANY WARRANTY; without
 * even the implied warranty of  MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE, EXCEPT AS EXPRESSLY SET FORTH
 * IN THE BY RECIPIENT SELECTED SUBSIDIARY LICENSE CONDITIONS
 * OF OSMC-PL.
 *
 * See the full OSMC Public License conditions for more details.
 *
 */

/*
 * Main Authors 2009-2010:  Robert Braun, Björn Eriksson, Peter Nordin
 * Contributors 2009-2010:  Mikael Axin, Alessandro Dell'Amico, Karl Pettersson, Ingo Staack
 * Fluid and Mechatronic Systems, Department of Management and Engineering, Linköping University
 * Based on: HopsanGUI
 */

//$Id$
#include "plotwidget.h"
#include <QDebug>
#include <QSpinBox>
#include <QColorDialog>
#include <QLabel>


PlotWidget::PlotWidget(QVector<double> xarray, QVector<double> yarray, MainWindow *parent)
    : QMainWindow(parent)//QWidget(parent,Qt::Window)
{
    this->setAttribute(Qt::WA_DeleteOnClose);

    QWidget *centralwidget = new QWidget(this);

    mpParentMainWindow = parent;
    mpCurrentGraphicsView = mpParentMainWindow->mpProjectTabs->getCurrentTab()->mpGraphicsView;

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

    btnSize = new QToolBar(tr("Size Spinbox"));
    QLabel *sizeLabel = new QLabel(tr("Line Width"));
    sizeSpinBox = new QSpinBox(toolBar);
    //btnSize->set("Line Width");
    sizeSpinBox->setRange(1,10);
    sizeSpinBox->setSingleStep(1);
    sizeSpinBox->setSuffix(" pt");
    btnSize->setOrientation(Qt::Vertical);
    btnSize->addWidget(sizeLabel);
    btnSize->addWidget(sizeSpinBox);
    toolBar->addWidget(btnSize);

    btnColor = new QToolButton(toolBar);
    btnColor->setText("Line Color");
    btnColor->setIcon(QIcon("../../HopsanGUI/icons/palette.png"));
    btnColor->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    toolBar->addWidget(btnColor);

    btnBackgroundColor = new QToolButton(toolBar);
    btnBackgroundColor->setText("Canvas Color");
    btnBackgroundColor->setIcon(QIcon("../../HopsanGUI/icons/palette.png"));
    btnBackgroundColor->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    toolBar->addWidget(btnBackgroundColor);

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
    grid->hide();

    enableZoom(false);

    //Establish signal and slots connections
    connect(buttonbox, SIGNAL(rejected()), this, SLOT(close()));
    connect(btnZoom,SIGNAL(toggled(bool)),SLOT(enableZoom(bool)));
    connect(btnPan,SIGNAL(toggled(bool)),SLOT(enablePan(bool)));
    connect(btnSVG,SIGNAL(clicked()),SLOT(exportSVG()));
    connect(btnGrid,SIGNAL(toggled(bool)),SLOT(enableGrid(bool)));
    connect(sizeSpinBox,SIGNAL(valueChanged(int)),this, SLOT(setSize(int)));
    connect(btnColor,SIGNAL(clicked()),this,SLOT(setColor()));
    connect(btnBackgroundColor,SIGNAL(clicked()),this,SLOT(setBackgroundColor()));

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

void PlotWidget::setSize(int size)
{
    mpCurve->setPen(QPen(mpCurve->pen().color(),size));
}

void PlotWidget::setColor()
{
    QColor color = QColorDialog::getColor(Qt::black, this);
    if (color.isValid())
    {
        mpCurve->setPen(QPen(color, mpCurve->pen().width()));
        //colorLabel->setText(color.name());
        //colorLabel->setPalette(QPalette(color));
        //colorLabel->setAutoFillBackground(true);
    }
}

void PlotWidget::setBackgroundColor()
{
    QColor color = QColorDialog::getColor(Qt::white, this);
    if (color.isValid())
    {
        mpVariablePlot->setCanvasBackground(color);
        mpVariablePlot->replot();
    }
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


VariableList::VariableList(MainWindow *parent)
        : QListWidget(parent)
{
    qDebug() << "dsada";
    mpParentMainWindow = parent;
    mpCurrentView = mpParentMainWindow->mpProjectTabs->getCurrentTab()->mpGraphicsView;
    qDebug() << "erwq";
    //if ((!(this->mpCorePort->isConnected())) || (this->mpCorePort->getTimeVectorPtr()->empty()))

    xMap.clear();
    yMap.clear();
    QVector<double> y;
    qDebug() << "dsfsdaga";
    QMap<QString, GUIObject *>::iterator it;
    QListWidgetItem *tempListWidget;
    qDebug() << "wqer";
    for(it = mpCurrentView->mGUIObjectMap.begin(); it!=mpCurrentView->mGUIObjectMap.end(); ++it)
    {
        //qDebug() << "Gorilla";
        QList<GUIPort*>::iterator itp;
        //qDebug() << "Blaj";
        for(itp = it.value()->mPortListPtrs.begin(); itp !=it.value()->mPortListPtrs.end(); ++itp)
        {
            qDebug() << "Writing plot stuff for " << it.value()->getName() << " " << (*itp)->getName();
            // qDebug() << "tjo";
            y.clear();
            //qDebug() << "Bamse";
           // qDebug() << "Varmkorv";

            //qDebug() << "Julafton";

            //*****Core Interaction*****
//            if ((*itp)->mpCorePort->getNodeType() =="NodeHydraulic")
//            {
//                size_t dataLength = (*itp)->mpCorePort->getTimeVectorPtr()->size();
//                QVector<double> time = QVector<double>::fromStdVector(*((*itp)->mpCorePort->getTimeVectorPtr()));
//                //qDebug() << "hoj";
//                tempListWidget = new QListWidgetItem(it.key() + ", " +(*itp)->getName() + ", Flow", this);
//                //qDebug() << "hoj 1";
//                for (size_t i = 0; i<dataLength-1; ++i) //Denna loop ar inte klok
//                {
//                    y.append(((*itp)->mpCorePort->getDataVectorPtr()->at(i)).at(0));
//                }
//                //qDebug() << "hoj 2";
//                xMap.insert(it.key() + ", " + (*itp)->getName() + ", Flow", time);
//                yMap.insert(it.key() + ", " + (*itp)->getName() + ", Flow", y);
//
//                y.clear();
//
//                tempListWidget = new QListWidgetItem(it.key() + ", " +(*itp)->getName() + ", Pressure", this);
//                for (size_t i = 0; i<dataLength-1; ++i) //Denna loop ar inte klok
//                {
//                    y.append(((*itp)->mpCorePort->getDataVectorPtr()->at(i)).at(1));
//                }
//                xMap.insert(it.key() + ", " + (*itp)->getName() + ", Pressure", time);
//                yMap.insert(it.key() + ", " + (*itp)->getName() + ", Pressure", y);
//            }
//            if ((*itp)->mpCorePort->getNodeType() =="NodeMechanic")
//            {
//                size_t dataLength = (*itp)->mpCorePort->getTimeVectorPtr()->size();
//                QVector<double> time = QVector<double>::fromStdVector(*((*itp)->mpCorePort->getTimeVectorPtr()));
//                //qDebug() << "hej";
//                tempListWidget = new QListWidgetItem(it.key() + ", " +(*itp)->getName() + ", Velocity", this);
//                for (size_t i = 0; i<dataLength-1; ++i) //Denna loop ar inte klok
//                {
//                    y.append(((*itp)->mpCorePort->getDataVectorPtr()->at(i)).at(0));
//                }
//                xMap.insert(it.key() + ", " + (*itp)->getName() + ", Velocity", time);
//                yMap.insert(it.key() + ", " + (*itp)->getName() + ", Velocity", y);
//
//                y.clear();
//
//                tempListWidget = new QListWidgetItem(it.key() + ", " +(*itp)->getName() + ", Force", this);
//                for (size_t i = 0; i<dataLength-1; ++i) //Denna loop ar inte klok
//                {
//                    y.append(((*itp)->mpCorePort->getDataVectorPtr()->at(i)).at(1));
//                }
//                xMap.insert(it.key() + ", " + (*itp)->getName() + ", Force", time);
//                yMap.insert(it.key() + ", " + (*itp)->getName() + ", Force", y);
//
//                y.clear();
//
//                tempListWidget = new QListWidgetItem(it.key() + ", " +(*itp)->getName() + ", Position", this);
//                for (size_t i = 0; i<dataLength-1; ++i) //Denna loop ar inte klok
//                {
//                    y.append(((*itp)->mpCorePort->getDataVectorPtr()->at(i)).at(2));
//                }
//                xMap.insert(it.key() + ", " + (*itp)->getName() + ", Position", time);
//                yMap.insert(it.key() + ", " + (*itp)->getName() + ", Position", y);
//            }
//            if ((*itp)->mpCorePort->getNodeType() =="NodeSignal")
//            {
//                if((*itp)->mpCorePort->isConnected())
//                {
//                    //qDebug() << "size = " << (*itp)->mpCorePort->getTimeVectorPtr()->size();
//                    size_t dataLength = (*itp)->mpCorePort->getTimeVectorPtr()->size();
//                    QVector<double> time = QVector<double>::fromStdVector(*((*itp)->mpCorePort->getTimeVectorPtr()));
//                    //qDebug() << "haj";
//                    tempListWidget = new QListWidgetItem(it.key() + ", " +(*itp)->getName() + ", Signal", this);
//                    //qDebug() << "haj 1";
//                    for (size_t i = 0; i<dataLength-1; ++i) //Denna loop ar inte klok
//                    {
//                        y.append(((*itp)->mpCorePort->getDataVectorPtr()->at(i)).at(0));
//                    }
//                    //qDebug() << "haj 2";
//                    xMap.insert(it.key() + ", " + (*itp)->getName() + ", Signal", time);
//                    //qDebug() << "haj 3";
//                    yMap.insert(it.key() + ", " + (*itp)->getName() + ", Signal", y);
//                    //qDebug() << "haj 4";
//                }
//            }
            //**************************
        }
    }

    //Populate the list
    //QListWidgetItem *sinus = new QListWidgetItem(tr("linear1"),this);
    //QListWidgetItem *cosinus = new QListWidgetItem(tr("linear2"),this);

    // Store values for two functions, only for testing

    //map["linear1"] = 1.0;

    //map["linear2"] = 2.0;

    qDebug() << "Orangutang";

    connect(this,SIGNAL(itemDoubleClicked(QListWidgetItem*)),this,SLOT(createPlot(QListWidgetItem*)));
}

void VariableList::createPlot(QListWidgetItem *item)
{
    //double n = map.value(item->text());
    //std::cout << n << std::endl;

//    QVector<double> xarray(2);
  //  QVector<double> yarray(2);

    PlotWidget *plotwidget = new PlotWidget(xMap.find(item->text()).value(),yMap.find(item->text()).value(),this->mpParentMainWindow);
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


VariableListDialog::VariableListDialog(MainWindow *parent)
        : QWidget(parent)
{
    mpParentMainWindow = parent;

    //Create a grid
    QGridLayout *grid = new QGridLayout(this);

    //Create the plotvariables list
    VariableList *varList = new VariableList(mpParentMainWindow);

    grid->addWidget(varList,0,0);
}
