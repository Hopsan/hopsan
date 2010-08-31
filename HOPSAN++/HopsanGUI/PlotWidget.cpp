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
 * HopsanGUI
 * Fluid and Mechatronic Systems, Department of Management and Engineering, Linköping University
 * Main Authors 2009-2010:  Robert Braun, Björn Eriksson, Peter Nordin
 * Contributors 2009-2010:  Mikael Axin, Alessandro Dell'Amico, Karl Pettersson, Ingo Staack
 */

//$Id$

#include <QDebug>
#include <QSpinBox>
#include <QColorDialog>
#include <QLabel>
#include <QCursor>
#include <QAction>

#include "PlotWidget.h"

#include "MainWindow.h"
#include "ProjectTabWidget.h"
#include "GUIObject.h"
#include "GUIPort.h"
#include "GraphicsView.h"
#include "GUISystem.h"

PlotWidget::PlotWidget(QVector<double> xarray, QVector<double> yarray, MainWindow *parent)
    : QMainWindow(parent)//QWidget(parent,Qt::Window)
{
    this->setAttribute(Qt::WA_DeleteOnClose);

    QWidget *centralwidget = new QWidget(this);

    mpParentMainWindow = parent;
    mpCurrentGUISystem = mpParentMainWindow->mpProjectTabs->getCurrentSystem();

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
    mpVariablePlot->setCurve(mpCurve);
    mpCurve->setRenderHint(QwtPlotItem::RenderAntialiased);
    mpVariablePlot->replot();
    mpCurve->setPen(QPen(QBrush(QColor("Blue")),2));


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
    btnZoom->setToolTip("Zoom");
    btnZoom->setIcon(QIcon("../../HopsanGUI/icons/Hopsan-Zoom.png"));
    btnZoom->setCheckable(true);
    btnZoom->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    toolBar->addWidget(btnZoom);

    btnPan = new QToolButton(toolBar);
    btnPan->setToolTip("Pan");
    btnPan->setIcon(QIcon("../../HopsanGUI/icons/Hopsan-Pan.png"));
    btnPan->setCheckable(true);
    btnPan->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    toolBar->addWidget(btnPan);

    btnSVG = new QToolButton(toolBar);
    btnSVG->setToolTip("Export to SVG");
    btnSVG->setIcon(QIcon("../../HopsanGUI/icons/Hopsan-SaveToSvg.png"));
    btnSVG->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    toolBar->addWidget(btnSVG);

    btnGNUPLOT = new QToolButton(toolBar);
    btnGNUPLOT->setToolTip("Export to GNUPLOT");
    btnGNUPLOT->setIcon(QIcon("../../HopsanGUI/icons/Hopsan-SaveToGnuPlot.png"));
    btnGNUPLOT->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    toolBar->addWidget(btnGNUPLOT);

    btnGrid = new QToolButton(toolBar);
    btnGrid->setToolTip("Show Grid");
    btnGrid->setIcon(QIcon("../../HopsanGUI/icons/Hopsan-Grid.png"));
    btnGrid->setCheckable(true);
    btnGrid->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    toolBar->addWidget(btnGrid);

    btnColor = new QToolButton(toolBar);
    btnColor->setToolTip("Select Line Color");
    btnColor->setIcon(QIcon("../../HopsanGUI/icons/Hopsan-LineColor.png"));
    btnColor->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    toolBar->addWidget(btnColor);

    btnBackgroundColor = new QToolButton(toolBar);
    btnBackgroundColor->setToolTip("Select Canvas Color");
    btnBackgroundColor->setIcon(QIcon("../../HopsanGUI/icons/Hopsan-BackgroundColor.png"));
    btnBackgroundColor->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    toolBar->addWidget(btnBackgroundColor);

    //btnSize = new QToolBar(tr("Size Spinbox"));
    QLabel *sizeLabel = new QLabel(tr("Line Width: "));
    sizeSpinBox = new QSpinBox(toolBar);
    //btnSize->set("Line Width");
    sizeSpinBox->setRange(1,10);
    sizeSpinBox->setSingleStep(1);
    sizeSpinBox->setSuffix(" pt");
    //btnSize->setOrientation(Qt::Vertical);
    //btnSize->addWidget(sizeLabel);
    //btnSize->addWidget(sizeSpinBox);
    toolBar->addWidget(sizeLabel);
    toolBar->addWidget(sizeSpinBox);

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
    //grid->hide();

    enableZoom(false);

    //Establish signal and slots connections
    connect(buttonbox, SIGNAL(rejected()), this, SLOT(close()));
    connect(btnZoom,SIGNAL(toggled(bool)),SLOT(enableZoom(bool)));
    connect(btnPan,SIGNAL(toggled(bool)),SLOT(enablePan(bool)));
    connect(btnSVG,SIGNAL(clicked()),SLOT(exportSVG()));
    connect(btnGNUPLOT,SIGNAL(clicked()),SLOT(exportGNUPLOT()));
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


void PlotWidget::exportGNUPLOT()
{
    QDir fileDialogSaveDir;
    QString modelFileName = QFileDialog::getSaveFileName(this, tr("Save Model File"),
                                                         fileDialogSaveDir.currentPath(),
                                                         tr("GNUPLOT File (*.GNUPLOT)"));
    QFile file(modelFileName);
    QFileInfo fileInfo(file);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qDebug() << "Failed to open file for writing: " + modelFileName;
        return;
    }
    QTextStream modelFile(&file);  //Create a QTextStream object to stream the content of file

    size_t size = mpVariablePlot->getCurve()->data().size();
    for(std::size_t i=0; i!=size; ++i)
    {
        modelFile << mpVariablePlot->getCurve()->data().x(i);
        modelFile << " ";
        modelFile << mpVariablePlot->getCurve()->data().y(i);
        modelFile << "\n";
    }
    file.close();
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
    this->setAcceptDrops(false);

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


void VariablePlot::setCurve(QwtPlotCurve *pCurve)
{
    mpCurve = pCurve;
}


QwtPlotCurve *VariablePlot::getCurve()
{
    return mpCurve;
}


VariableList::VariableList(MainWindow *parent)
        : QTreeWidget(parent)
{
    mpParentMainWindow = parent;
    mpCurrentSystem = mpParentMainWindow->mpProjectTabs->getCurrentSystem();

    this->setDragEnabled(true);
    this->setAcceptDrops(true);
    this->updateList();
    this->setHeaderHidden(true);
    this->setColumnCount(1);

    connect(mpParentMainWindow->mpProjectTabs, SIGNAL(currentChanged(int)), this, SLOT(updateList()));
    connect(mpParentMainWindow->simulateAction, SIGNAL(triggered()), this, SLOT(updateList()));
    connect(this,SIGNAL(itemDoubleClicked(QListWidgetItem*)),this,SLOT(createPlot(QListWidgetItem*)));
}

void VariableList::updateList()
{
    if(mpParentMainWindow->mpProjectTabs->count() == 0)     //Check so that project tabs are not empty (= program is closing)
    {
        return;
    }

    xMap.clear();
    yMap.clear();
    this->clear();
    mpCurrentSystem = mpParentMainWindow->mpProjectTabs->getCurrentTab()->mpSystem;
    QVector<double> y;
    QHash<QString, GUIObject *>::iterator it;
    QTreeWidgetItem *tempComponentItem;
    QTreeWidgetItem *tempParameterItem;
    bool colorize = false;
    for(it = mpCurrentSystem->mGUIObjectMap.begin(); it!=mpCurrentSystem->mGUIObjectMap.end(); ++it)
    {
        QColor backgroundColor;
        if(colorize)
        {
            backgroundColor = QColor("white");
            colorize = false;
        }
        else
        {
            backgroundColor = QColor("white");      //Used to be "beige"
            colorize = true;
        }

        tempComponentItem = new QTreeWidgetItem();
        tempComponentItem->setText(0, it.value()->getName());
        tempComponentItem->setBackgroundColor(0, backgroundColor);
        QFont tempFont;
        tempFont = tempComponentItem->font(0);
        tempFont.setBold(true);
        tempComponentItem->setFont(0, tempFont);
        this->addTopLevelItem(tempComponentItem);

        QList<GUIPort*> portListPtrs = it.value()->getPortListPtrs();
        QList<GUIPort*>::iterator itp;
        for(itp = portListPtrs.begin(); itp !=portListPtrs.end(); ++itp)
        {
           // qDebug() << "Writing plot stuff for " << it.value()->getName() << " " << (*itp)->getName();

            QVector<QString> parameterNames;
            QVector<QString> parameterUnits;
            mpParentMainWindow->mpProjectTabs->getCurrentTab()->mpSystem->mpCoreSystemAccess->getPlotDataNamesAndUnits((*itp)->getGUIComponentName(), (*itp)->getName(), parameterNames, parameterUnits);

            qDebug() << "guiComponentName: " << (*itp)->getGUIComponentName() << " portName: " << (*itp)->getName();
            QVector<double> time = QVector<double>::fromStdVector(mpParentMainWindow->mpProjectTabs->getCurrentTab()->mpSystem->mpCoreSystemAccess->getTimeVector((*itp)->getGUIComponentName(), (*itp)->getName()));
            qDebug() << "time.size: " << time.size();
            if(time.size() > 0)     //If time vector is greater than zero we have something to plot!
            {
                for(int i = 0; i!=parameterNames.size(); ++i)
                {
                    y.clear();
                    tempParameterItem = new QTreeWidgetItem();
                    tempParameterItem->setText(0, (*itp)->getName() + ", " + parameterNames[i] + ", [" + parameterUnits[i] + "]");
                    tempParameterItem->setBackgroundColor(0, backgroundColor);
                    //tempListWidget->setFlags(Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
                    tempComponentItem->addChild(tempParameterItem);
                    mpParentMainWindow->mpProjectTabs->getCurrentTab()->mpSystem->mpCoreSystemAccess->getPlotData((*itp)->getGUIComponentName(), (*itp)->getName(), parameterNames[i], y);
                    xMap.insert((*itp)->getGUIComponentName() + ", " + (*itp)->getName() + ", " + parameterNames[i] + ", [" + parameterUnits[i] + "]", time);
                    yMap.insert((*itp)->getGUIComponentName() + ", " + (*itp)->getName() + ", " + parameterNames[i] + ", [" + parameterUnits[i] + "]", y);
                    yLabelMap.insert((*itp)->getGUIComponentName() + ", " + (*itp)->getName() + ", " + parameterNames[i] + ", [" + parameterUnits[i] + "]", parameterNames[i]);
                }
            }
        }
    }
    this->sortItems(0, Qt::AscendingOrder);
}

void VariableList::createPlot(QTreeWidgetItem *item)
{
    //double n = map.value(item->text());
    //std::cout << n << std::endl;

//    QVector<double> xarray(2);
  //  QVector<double> yarray(2);

    QString lookupName;
    lookupName = QString(item->parent()->text(0) + ", " + item->text(0));

    QString title;
    QString xlabel;
    QString ylabel;

    title.append(lookupName);
    ylabel.append(yLabelMap.find(lookupName).value());
    xlabel.append("Time, [s]");

    PlotWidget *plotwidget = new PlotWidget(xMap.find(lookupName).value(),yMap.find(lookupName).value(),mpParentMainWindow);
    plotwidget->setWindowTitle("HOPSAN Plot Window");
    plotwidget->mpCurve->setTitle(title);
    plotwidget->mpVariablePlot->setAxisTitle(VariablePlot::yLeft, ylabel);
    plotwidget->mpVariablePlot->setAxisTitle(VariablePlot::xBottom, xlabel);
    plotwidget->mpVariablePlot->insertLegend(new QwtLegend(), QwtPlot::TopLegend);
    plotwidget->show();

    std::cout << lookupName.toStdString() << std::endl;
}

void VariableList::mousePressEvent(QMouseEvent *event)
{
    QTreeWidget::mousePressEvent(event);

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

    QTreeWidgetItem *item = this->currentItem();

    stream << item->text(0);

    *data = "Test";

    QString mimeType = "application/x-plotvariable";

    QDrag *drag = new QDrag(this);
    QMimeData *mimeData = new QMimeData;

    mimeData->setData(mimeType, *data);
    qDebug() << "mimeData = " << *data;
    drag->setMimeData(mimeData);
    //QCursor cursor;
    //drag->setHotSpot(cursor.pos());
    //drag->setHotSpot(QPoint(drag->pixmap().width()/2, drag->pixmap().height()));
    drag->exec();

    //Qt::DropAction dropAction = drag->exec(Qt::CopyAction | Qt::MoveAction);

}


SelectedVariableList::SelectedVariableList(MainWindow *parent)
        : VariableList(parent)
{
    mpParentMainWindow = parent;
    mpCurrentSystem = mpParentMainWindow->mpProjectTabs->getCurrentSystem();
    this->setAcceptDrops(true);
    this->setDragEnabled(true);

    xMap.clear();
    yMap.clear();
}

void SelectedVariableList::dragMoveEvent(QDragMoveEvent *event)
{
    qDebug() << "Dragging something...";
    if (event->mimeData()->hasFormat("application/x-plotvariable"))
    {
        event->acceptProposedAction();
    }
}

//
//void SelectedVariableList::dragMoveEvent(QDragMoveEvent *event)
//{
//    if (event->mimeData()->hasFormat("application/x-plotvariable"))
//    {
//      event->accept();
//    }
//    else
//    {
//        event->ignore();
//    }
//}


void SelectedVariableList::dropEvent(QDropEvent *event)
{
//    qDebug() << "dropEvent";
//    if (event->mimeData()->hasFormat("application/x-plotvariable"))
//    //if (event->mimeData()->hasText())
//    {
//        qDebug() << "True!";
//        QString datastr =  event->mimeData()->text();
//        //QTextStream stream(&datastr, QIODevice::ReadOnly);
//        QListWidgetItem *tempListWidget;
//        tempListWidget = new QListWidgetItem(datastr, this);

//        event->acceptProposedAction();
//   }
}

VariableListDialog::VariableListDialog(MainWindow *parent)
        : QWidget(parent)
{
    //this->setAcceptDrops(true);

    mpParentMainWindow = parent;

    //Create a grid
    QGridLayout *grid = new QGridLayout(this);

    //Create the plotvariables list
    VariableList *varList = new VariableList(mpParentMainWindow);
    SelectedVariableList *rightAxisList = new SelectedVariableList(mpParentMainWindow);
    SelectedVariableList *leftAxisList = new SelectedVariableList(mpParentMainWindow);
    rightAxisList->setMaximumHeight(100);
    rightAxisList->setObjectName("Right Axis");
    rightAxisList->setWindowTitle("Right Axis");
    rightAxisList->setSelectionMode(QAbstractItemView::SingleSelection);
    rightAxisList->setDragEnabled(true);
    rightAxisList->viewport()->setAcceptDrops(true);
    rightAxisList->setDropIndicatorShown(true);
    rightAxisList->setDragDropMode(QAbstractItemView::DragDrop);
    leftAxisList->setMaximumHeight(100);
    leftAxisList->setWindowTitle("Left Axis");

    plotButton = new QPushButton(tr("&Plot"));
    plotButton->setAutoDefault(true);

    grid->addWidget(varList,0,0,3,1);
    //grid->addWidget(rightAxisList,4,0,1,1);
    //grid->addWidget(leftAxisList,5,0,1,1);
    //grid->addWidget(plotButton,6,0);
}
