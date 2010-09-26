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
#include "GuiUtilities.h"
#include "GUISystem.h"

#include "qwt_scale_engine.h"

#include "qwt_symbol.h"
#include "qwt_text_label.h"

PlotWindow::PlotWindow(QVector<double> xarray, QVector<double> yarray, VariableList *variableList, MainWindow *parent)
    : QMainWindow(parent)
{
    this->setAttribute(Qt::WA_DeleteOnClose);

    mpParentMainWindow = parent;
    mpCurrentGUISystem = mpParentMainWindow->mpProjectTabs->getCurrentSystem();
    mpVariableList = variableList;

    mHasSpecialXAxis = false;

    mHold = false;

        //Create the plot
    mpVariablePlot = new VariablePlot();
    mpVariablePlot->setAcceptDrops(false);

    nCurves = 0;
    mCurveColors << "Blue" << "Red" << "Green" << "Orange";

    mLeftAxisLogarithmic = false;
    mRightAxisLogarithmic = false;

        //Create mpToolBar and toolbutton
    mpToolBar = new QToolBar(this);
    mpToolBar->setAcceptDrops(false);

    mpZoomButton = new QToolButton(mpToolBar);
    mpZoomButton->setToolTip("Zoom");
    mpZoomButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-Zoom.png"));
    mpZoomButton->setCheckable(true);
    mpZoomButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    mpZoomButton->setAcceptDrops(false);
    mpToolBar->addWidget(mpZoomButton);

    mpPanButton = new QToolButton(mpToolBar);
    mpPanButton->setToolTip("Pan");
    mpPanButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-Pan.png"));
    mpPanButton->setCheckable(true);
    mpPanButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    mpPanButton->setAcceptDrops(false);
    mpToolBar->addWidget(mpPanButton);

    mpSVGButton = new QToolButton(mpToolBar);
    mpSVGButton->setToolTip("Export to SVG");
    mpSVGButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-SaveToSvg.png"));
    mpSVGButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    mpSVGButton->setAcceptDrops(false);
    mpToolBar->addWidget(mpSVGButton);

    mpGNUPLOTButton = new QToolButton(mpToolBar);
    mpGNUPLOTButton->setToolTip("Export to GNUPLOT");
    mpGNUPLOTButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-SaveToGnuPlot.png"));
    mpGNUPLOTButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    mpGNUPLOTButton->setAcceptDrops(false);
    mpToolBar->addWidget(mpGNUPLOTButton);

    mpGridButton = new QToolButton(mpToolBar);
    mpGridButton->setToolTip("Show Grid");
    mpGridButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-Grid.png"));
    mpGridButton->setCheckable(true);
    mpGridButton->setChecked(true);
    mpGridButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    mpGridButton->setAcceptDrops(false);
    mpToolBar->addSeparator();
    mpToolBar->addWidget(mpGridButton);

    mpColorButton = new QToolButton(mpToolBar);
    mpColorButton->setToolTip("Select Line Color");
    mpColorButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-LineColor.png"));
    mpColorButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    mpColorButton->setAcceptDrops(false);
    mpToolBar->addWidget(mpColorButton);

    mpBackgroundColorButton = new QToolButton(mpToolBar);
    mpBackgroundColorButton->setToolTip("Select Canvas Color");
    mpBackgroundColorButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-BackgroundColor.png"));
    mpBackgroundColorButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    mpBackgroundColorButton->setAcceptDrops(false);
    mpToolBar->addWidget(mpBackgroundColorButton);

    mpSizeLabel = new QLabel(tr("Line Width: "));
    mpSizeLabel->setAcceptDrops(false);
    mpSizeSpinBox = new QSpinBox(mpToolBar);
    mpSizeSpinBox->setAcceptDrops(false);
    mpSizeSpinBox->setRange(1,10);
    mpSizeSpinBox->setSingleStep(1);
    mpSizeSpinBox->setValue(2);
    mpSizeSpinBox->setSuffix(" pt");
    mpToolBar->addWidget(mpSizeLabel);
    mpToolBar->addWidget(mpSizeSpinBox);

    mpHoldCheckBox = new QCheckBox("Hold Plot Data");
    mpHoldCheckBox->setChecked(mHold);
    mpToolBar->addSeparator();
    mpToolBar->addWidget(mpHoldCheckBox);

    addToolBar(mpToolBar);

    //Zoom
    mpZoomer = new QwtPlotZoomer( QwtPlot::xBottom, QwtPlot::yLeft, mpVariablePlot->canvas());
    mpZoomer->setSelectionFlags(QwtPicker::DragSelection | QwtPicker::CornerToCorner);
    mpZoomer->setRubberBand(QwtPicker::RectRubberBand);
    mpZoomer->setRubberBandPen(QColor(Qt::green));
    mpZoomer->setTrackerMode(QwtPicker::ActiveOnly);
    mpZoomer->setTrackerPen(QColor(Qt::white));
    mpZoomer->setMousePattern(QwtEventPattern::MouseSelect2, Qt::RightButton, Qt::ControlModifier);
    mpZoomer->setMousePattern(QwtEventPattern::MouseSelect3, Qt::RightButton);

    //Panner
    mpPanner = new QwtPlotPanner(mpVariablePlot->canvas());
    mpPanner->setMouseButton(Qt::MidButton);

    //grid
    mpGrid = new QwtPlotGrid;
    mpGrid->enableXMin(true);
    mpGrid->enableYMin(true);
    mpGrid->setMajPen(QPen(Qt::black, 0, Qt::DotLine));
    mpGrid->setMinPen(QPen(Qt::gray, 0 , Qt::DotLine));
    mpGrid->attach(mpVariablePlot);
    //grid->hide();

        // Create and add curves to the plot
    tempCurve = new QwtPlotCurve();
    QwtArrayData data(xarray,yarray);
    tempCurve->setData(data);
    tempCurve->attach(mpVariablePlot);
    mpVariablePlot->setCurve(tempCurve);
    tempCurve->setRenderHint(QwtPlotItem::RenderAntialiased);
    mpVariablePlot->replot();
    tempCurve->setPen(QPen(QBrush(QColor(mCurveColors[nCurves])),mpSizeSpinBox->value()));
    mpCurves.append(tempCurve);
    ++nCurves;

        //Create the close button
    QDialogButtonBox *buttonbox = new QDialogButtonBox(QDialogButtonBox::Close);
    buttonbox->setAcceptDrops(false);

    this->setCentralWidget(mpVariablePlot);

    enableZoom(false);

    //Establish signal and slots connections
    connect(buttonbox, SIGNAL(rejected()), this, SLOT(close()));
    connect(mpZoomButton,SIGNAL(toggled(bool)),SLOT(enableZoom(bool)));
    connect(mpPanButton,SIGNAL(toggled(bool)),SLOT(enablePan(bool)));
    connect(mpSVGButton,SIGNAL(clicked()),SLOT(exportSVG()));
    connect(mpGNUPLOTButton,SIGNAL(clicked()),SLOT(exportGNUPLOT()));
    connect(mpGridButton,SIGNAL(toggled(bool)),SLOT(enableGrid(bool)));
    connect(mpSizeSpinBox,SIGNAL(valueChanged(int)),this, SLOT(setSize(int)));
    connect(mpColorButton,SIGNAL(clicked()),this,SLOT(setColor()));
    connect(mpBackgroundColorButton,SIGNAL(clicked()),this,SLOT(setBackgroundColor()));
    connect (mpHoldCheckBox, SIGNAL(toggled(bool)), this, SLOT(setHold(bool)));
    connect(this->mpParentMainWindow->mpProjectTabs->getCurrentTab(),SIGNAL(simulationFinished()),this,SLOT(checkNewValues()));

    resize(600,600);

    this->setAcceptDrops(true);



    //mpMarker = new QwtPlotMarker();
    mpMarkerSymbol = new QwtSymbol();
    mpMarkerSymbol->setBrush(QBrush(Qt::red, Qt::SolidPattern));
    mpMarkerSymbol->setStyle(QwtSymbol::Ellipse);
    mpMarkerSymbol->setSize(10,10);
//    mpMarker->setSymbol(*mpMarkerSymbol);
//    mpMarker->setXValue(0);
//    mpMarker->setYValue(0);
//    mpMarker->attach(mpVariablePlot);

    //mpLabels = new QwtText();
    //mpLabels->setText("(0.0, 0.0)");
    //mpLabels->setBackgroundBrush(QColor("yellow"));
    //mpLabels->setFont(QFont("Calibri", 12, QFont::Bold));
    //mpLabel = new QwtTextLabel(*mpLabelText, this);
    //mpLabel->setGeometry(0, 0, 70, 24);
    //mpLabel->adjustSize();
    //mpLabel->show();
}


void PlotWindow::setHold(bool value)
{
    mHold = value;
}


void PlotWindow::insertMarker(QwtPlotCurve *curve)
{
    if(mCurveToMarkerMap.contains(curve))
    {
        return;
    }
    QwtPlotMarker *tempMarker = new QwtPlotMarker();
    mpMarkerSymbol->setBrush(curve->pen().brush().color());
    tempMarker->setSymbol(*mpMarkerSymbol);

    QwtText tempLabel;
    QString xString;
    QString yString;
    xString.setNum(curve->x(0));
    yString.setNum(curve->y(0));
    tempLabel.setText("("+xString+", "+yString+")");
    tempLabel.setColor(curve->pen().brush().color());
    tempLabel.setBackgroundBrush(QBrush(QColor("lemonchiffon")));
    tempLabel.setFont(QFont("Calibri", 12, QFont::Bold));
    tempMarker->setLabel(tempLabel);

    mpMarkers.append(tempMarker);
    mCurveToMarkerMap.insert(curve, tempMarker);
    mMarkerToCurveMap.insert(tempMarker, curve);
    setActiveMarker(tempMarker);

    tempMarker->attach(mpVariablePlot);
    mpActiveMarker->setXValue(curve->x(0));
    double y_pos = mpVariablePlot->canvasMap(QwtPlot::yLeft).invTransform(mpVariablePlot->canvasMap(curve->yAxis()).xTransform(curve->y(0)));
    mpActiveMarker->setYValue(y_pos);
}


void PlotWindow::setActiveMarker(QwtPlotMarker *marker)
{
    this->mpActiveMarker = marker;
}


void PlotWindow::enableZoom(bool on)
{
    mpZoomer->setEnabled(on);
    //mpZoomer->zoom(0);

    mpPanner->setEnabled(on);
    mpPanner->setMouseButton(Qt::MidButton);

    disconnect(mpPanButton,SIGNAL(toggled(bool)),this,SLOT(enablePan(bool)));
    mpPanButton->setChecked(false);
    connect(mpPanButton,SIGNAL(toggled(bool)),this, SLOT(enablePan(bool)));
    mpPanner->setEnabled(false);
}


//! Slot that enables or disables
void PlotWindow::enablePan(bool on)
{
    mpPanner->setEnabled(on);
    mpPanner->setMouseButton(Qt::LeftButton);

    disconnect(mpZoomButton,SIGNAL(toggled(bool)),this,SLOT(enableZoom(bool)));
    mpZoomButton->setChecked(false);
    connect(mpZoomButton,SIGNAL(toggled(bool)),this,SLOT(enableZoom(bool)));
    mpZoomer->setEnabled(false);
}


//! Slot that turns plot grid on or off
void PlotWindow::enableGrid(bool on)
{
    if (on)
    {
        mpGrid->show();
    }
    else
    {
        mpGrid->hide();
    }

}


//! @Slot that exports current plot to .svg format
void PlotWindow::exportSVG()
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


//! Slot that exports a curve to GNUPLOT format
//! @todo Currently only the last added curve will be exported...
void PlotWindow::exportGNUPLOT()
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

void PlotWindow::setSize(int size)
{
    for(size_t i=0; i<mpCurves.size(); ++i)
    {
        mpCurves.at(i)->setPen(QPen(mpCurves.at(i)->pen().color(),size));
    }
}

void PlotWindow::setColor()
{
    QMenu menu;


    //QVector<*QAction> curves;
    //QAction *tempAction;

    for(int i=0; i<mpCurves.size(); ++i)
    {
        menu.addAction(mpCurves[i]->title().text());
    }

    QCursor *cursor;
    QAction *selectedAction = menu.exec(cursor->pos());

    if(selectedAction == 0)
    {
        return;
    }

    QwtPlotCurve *pSelectedCurve;

    for(int i=0; i<mpCurves.size(); ++i)
    {
        if (selectedAction->text() == mpCurves[i]->title().text())
        {
            pSelectedCurve = mpCurves[i];
        }
    }

    QColor color = QColorDialog::getColor(pSelectedCurve->pen().color(), this);
    if (color.isValid())
    {
        pSelectedCurve->setPen(QPen(color, pSelectedCurve->pen().width()));
        if(mCurveToMarkerMap.contains(pSelectedCurve))
        {
            QwtText tempLabel = mCurveToMarkerMap.value(pSelectedCurve)->label();
            tempLabel.setColor(color);
            mCurveToMarkerMap.value(pSelectedCurve)->setLabel(tempLabel);
            mpMarkerSymbol->setBrush(color);
            mCurveToMarkerMap.value(pSelectedCurve)->setSymbol(*mpMarkerSymbol);
        }
    }
}

void PlotWindow::setBackgroundColor()
{
    QColor color = QColorDialog::getColor(this->mpVariablePlot->canvasBackground(), this);
    if (color.isValid())
    {
        mpVariablePlot->setCanvasBackground(color);
        mpVariablePlot->replot();
    }
}


void PlotWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasText())
    {
        mpHoverRect = new QRubberBand(QRubberBand::Rectangle,this);
        mpHoverRect->setGeometry(0, 0, this->width(), this->height());
        mpHoverRect->setWindowOpacity(1);
        mpHoverRect->show();

        event->acceptProposedAction();
    }
}

void PlotWindow::mouseMoveEvent(QMouseEvent *event)
{

    if(mpActiveMarker != 0)
    {
        QwtPlotCurve *curve = mMarkerToCurveMap.value(mpActiveMarker);
        QCursor cursor;
        int correctionFactor = mpVariablePlot->canvas()->x()+5;
        int intX = this->mapFromGlobal(cursor.pos()).x() - correctionFactor;
        double x = mpVariablePlot->canvasMap(curve->xAxis()).invTransform(intX);
//        double x = mpVariablePlot->canvasMap(QwtPlot::xBottom).invTransform(intX);
        if(x < 0)
        {
            x = 0;
        }
        if(intX < 0)
        {
            intX = 0;
        }
        int xDataPos = x / curve->maxXValue() * curve->dataSize();
        if(xDataPos > curve->dataSize()-1)
        {
            xDataPos = curve->dataSize()-1;
        }
        double y = curve->y(std::max(0, xDataPos));
        double y_pos = mpVariablePlot->canvasMap(QwtPlot::yLeft).invTransform(mpVariablePlot->canvasMap(curve->yAxis()).xTransform(y));
        mpActiveMarker->setXValue(x);
        mpActiveMarker->setYValue(y_pos);

        QString xString;
        QString yString;
        xString.setNum(x);
        yString.setNum(y);

        QwtText tempLabel;
        tempLabel.setText("("+xString+", "+yString+")");
        tempLabel.setColor(curve->pen().brush().color());
        tempLabel.setBackgroundBrush(QBrush(QColor("lemonchiffon")));
        tempLabel.setFont(QFont("Calibri", 12, QFont::Bold));

        mpActiveMarker->setLabel(tempLabel);
        mpActiveMarker->setLabelAlignment(Qt::AlignTop);
        mpVariablePlot->replot();
    }
}


void PlotWindow::dragMoveEvent(QDragMoveEvent *event)
{
    QCursor cursor;
    if(this->mapFromGlobal(cursor.pos()).y() > this->height()/2 && mpCurves.size() >= 1)
    {
        mpHoverRect->setGeometry(mpVariablePlot->canvas()->x(), mpVariablePlot->canvas()->height()/2+mpVariablePlot->canvas()->y()+34, mpVariablePlot->canvas()->width(), mpVariablePlot->canvas()->height()/2);
    }
    else if(this->mapFromGlobal(cursor.pos()).x() < this->width()/2)
    {
        mpHoverRect->setGeometry(mpVariablePlot->canvas()->x(), mpVariablePlot->canvas()->y()+34, mpVariablePlot->canvas()->width()/2, mpVariablePlot->canvas()->height());
    }
    else
    {
        mpHoverRect->setGeometry(mpVariablePlot->canvas()->x() + mpVariablePlot->canvas()->width()/2, mpVariablePlot->canvas()->y()+34, mpVariablePlot->canvas()->width()/2, mpVariablePlot->canvas()->height());
    }
    QMainWindow::dragMoveEvent(event);
}


void PlotWindow::dragLeaveEvent(QDragLeaveEvent *event)
{
    delete(mpHoverRect);
    QMainWindow::dragLeaveEvent(event);
}


//! Defines what happens when something is dropped in a plot window
void PlotWindow::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasText())
    {
        delete(mpHoverRect);

        QString mimeText = event->mimeData()->text();
        QTextStream mimeStream;
        mimeStream.setString(&mimeText);

        QString discardedText;
        QString componentName;
        QString portName;
        QString dataName;
        QString dataUnit;

        if(mimeText.startsWith("HOPSANPLOTDATA"))
        {
            mimeStream >> discardedText;
            componentName = readName(mimeStream);
            portName = readName(mimeStream);
            dataName = readName(mimeStream);
            dataUnit = readName(mimeStream);

            QString title;
            QString xlabel;
            QString ylabel;

            title.append(QString(componentName + ", " + portName + ", " + dataName + " [" + dataUnit + "]"));
            ylabel.append(dataName + " [" + dataUnit + "]");
            xlabel.append("Time [s]");

            QVector<double> xVector = QVector<double>::fromStdVector(mpParentMainWindow->mpProjectTabs->getCurrentSystem()->mpCoreSystemAccess->getTimeVector(componentName, portName));
            QVector<double> yVector;
            mpParentMainWindow->mpProjectTabs->getCurrentSystem()->mpCoreSystemAccess->getPlotData(componentName, portName, dataName, yVector);

            QCursor cursor;
            if(this->mapFromGlobal(cursor.pos()).y() > this->height()/2 && mpCurves.size() >= 1)
            {
                this->changeXVector(yVector, ylabel);
            }
            else if(this->mapFromGlobal(cursor.pos()).x() < this->width()/2)
            {
                this->addPlotCurve(xVector, yVector, title, xlabel, ylabel, QwtPlot::yLeft);
            }
            else
            {
                this->addPlotCurve(xVector, yVector, title, xlabel, ylabel, QwtPlot::yRight);
            }
        }
    }
}

void PlotWindow::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu;

    QMenu *yAxisRightMenu;
    QMenu *yAxisLeftMenu;
    QMenu *insertMarkerMenu;
    QMenu *selectMarkerMenu;

    yAxisRightMenu = menu.addMenu(QString("Right Y Axis"));
    yAxisLeftMenu = menu.addMenu(QString("Left Y Axis"));
    insertMarkerMenu = menu.addMenu(QString("Insert Curve Marker"));
    selectMarkerMenu = menu.addMenu(QString("Change Active Marker"));

    QAction *setRightAxisLogarithmic;
    QAction *setLeftAxisLogarithmic;

    QMap <QAction *, QwtPlotCurve *> actionToCurveMap;
    QAction *tempAction;
    for(size_t i=0; i<mpCurves.size(); ++i)
    {
        tempAction = insertMarkerMenu->addAction(mpCurves[i]->title().text());
        actionToCurveMap.insert(tempAction, mpCurves[i]);
        if(mCurveToMarkerMap.contains(mpCurves[i]))
        {
           tempAction->setDisabled(true);
        }
    }

    QMap <QAction *, QwtPlotMarker *> actionToMarkerMap;
    if(mpMarkers.size() < 2)
    {
        selectMarkerMenu->setDisabled(true);    //Disable the select marker menu if there are less than two markers
    }
    else
    {
        for(size_t i=0; i<mpMarkers.size(); ++i)
        {
            tempAction = selectMarkerMenu->addAction(mMarkerToCurveMap.value(mpMarkers[i])->title().text());
            actionToMarkerMap.insert(tempAction, mpMarkers[i]);
            if(mpActiveMarker == mpMarkers[i])
            {
                QFont tempFont = tempAction->font();
                tempFont.setBold(true);
                tempAction->setFont(tempFont);
            }
        }
    }

    setRightAxisLogarithmic = yAxisRightMenu->addAction("Logarithmic Scale");
    setLeftAxisLogarithmic = yAxisLeftMenu->addAction("Logarithmic Scale");

    setRightAxisLogarithmic->setCheckable(true);
    setLeftAxisLogarithmic->setCheckable(true);
    setRightAxisLogarithmic->setChecked(mRightAxisLogarithmic);
    setLeftAxisLogarithmic->setChecked(mLeftAxisLogarithmic);

    QCursor *cursor;
    QAction *selectedAction = menu.exec(cursor->pos());

    if (selectedAction == setRightAxisLogarithmic)
    {
        mRightAxisLogarithmic = !mRightAxisLogarithmic;
        if(mRightAxisLogarithmic)
        {
            mpVariablePlot->setAxisScaleEngine(QwtPlot::yRight, new QwtLog10ScaleEngine);
        }
        else
        {
            mpVariablePlot->setAxisScaleEngine(QwtPlot::yRight, new QwtLinearScaleEngine);
        }
    }
    else if (selectedAction == setLeftAxisLogarithmic)
    {
        mLeftAxisLogarithmic = !mLeftAxisLogarithmic;
        if(mLeftAxisLogarithmic)
        {
            mpVariablePlot->setAxisScaleEngine(QwtPlot::yLeft, new QwtLog10ScaleEngine);
        }
        else
        {
            mpVariablePlot->setAxisScaleEngine(QwtPlot::yLeft, new QwtLinearScaleEngine);
        }
    }
    QMap<QAction *, QwtPlotCurve *>::iterator it;
    for(it = actionToCurveMap.begin(); it!=actionToCurveMap.end(); ++it)
    {
        if(selectedAction == it.key())
        {
            this->insertMarker(it.value());
        }
    }
    QMap<QAction *, QwtPlotMarker *>::iterator itm;
    for(itm = actionToMarkerMap.begin(); itm!=actionToMarkerMap.end(); ++itm)
    {
        if(selectedAction == itm.key())
        {
            this->setActiveMarker(itm.value());
        }
    }

}


//! Help function to add a new curve to an existing plot window
//! @param xarray is the vector for the x-axis
//! @param yarray is the vector for the y-axis
//! @param title is the title of the curve
//! @param xLabel is the label for the x-axis
//! @param yLabel is the label for the y-axis
//! @param axisY tells whether the right or left y-axis shall be used
void PlotWindow::addPlotCurve(QVector<double> xarray, QVector<double> yarray, QString title, QString xLabel, QString yLabel, QwtPlot::Axis axisY)
{

        // Create and add curves to the plot
    tempCurve = new QwtPlotCurve(title);
    if(!mHasSpecialXAxis)
    {
        tempCurve->setData(xarray, yarray);
        mpVariablePlot->setAxisTitle(VariablePlot::xBottom, xLabel);
    }
    else
    {
        QVector<double> tempXarray;
        for(size_t j=0; j<mpCurves.last()->data().size(); ++j)
        {
            tempXarray.append(mpCurves.last()->data().x(j));
        }
        tempCurve->setData(tempXarray, yarray);
    }
    tempCurve->attach(mpVariablePlot);
    mpVariablePlot->setCurve(tempCurve);
    mpVariablePlot->enableAxis(axisY, true);
    tempCurve->setRenderHint(QwtPlotItem::RenderAntialiased);
    mpVariablePlot->replot();
    int size = mpSizeSpinBox->value();
    tempCurve->setPen(QPen(QBrush(QColor(mCurveColors[nCurves])),size));
    tempCurve->setYAxis(axisY);
    mpCurves.append(tempCurve);

    ++nCurves;
    if(nCurves > mCurveColors.size()-1)
    {
        nCurves = 0;        //! @todo Ugly way to restart color loop when too many curves are added
    }

    mpVariablePlot->setAxisTitle(VariablePlot::yLeft, yLabel);
    mpVariablePlot->insertLegend(new QwtLegend(), QwtPlot::TopLegend);
}


void PlotWindow::changeXVector(QVector<double> xarray, QString xLabel)
{
    QVector<double> tempYarray;
    for(size_t i=0; i<mpCurves.size(); ++i)
    {
        for(size_t j=0; j<mpCurves.at(i)->data().size(); ++j)       //! @todo Figure out a less stupid way of replacing only the x values...
        {
            tempYarray.append(mpCurves.at(i)->data().y(j));
        }
        mpCurves.at(i)->setData(xarray, tempYarray);
        tempYarray.clear();
    }
    mpVariablePlot->setAxisTitle(VariablePlot::xBottom, xLabel);
    mHasSpecialXAxis = true;
}


void PlotWindow::checkNewValues()
{
    if(mHold)       //Do not update curves to new values if hold is checked
    {
        return;
    }
    for(int i=0; i<mpCurves.size(); ++i)
    {
        if(mpVariableList->mAvailableParameters.contains(mCurveParameters[i]))
        {
            QVector<double> xVector;
            xVector = QVector<double>::fromStdVector(mpParentMainWindow->mpProjectTabs->getCurrentTab()->mpSystem->mpCoreSystemAccess->getTimeVector(mCurveParameters[i][0], mCurveParameters[i][1]));
            QVector<double> yVector;
            mpParentMainWindow->mpProjectTabs->getCurrentTab()->mpSystem->mpCoreSystemAccess->getPlotData(mCurveParameters[i][0], mCurveParameters[i][1], mCurveParameters[i][2], yVector);
            mpCurves[i]->setData(xVector, yVector);
            mpVariablePlot->replot();
        }
    }
}


VariablePlot::VariablePlot(QWidget *parent)
        : QwtPlot(parent)
{
    setCanvasBackground(QColor(Qt::white));
    setAutoReplot(true);
}


void VariablePlot::setCurve(QwtPlotCurve *pCurve)
{
    mpCurve = pCurve;
}


QwtPlotCurve *VariablePlot::getCurve()
{
    return mpCurve;
}



ParameterItem::ParameterItem(QString componentName, QString portName, QString dataName, QString dataUnit, QTreeWidgetItem *parent)
        : QTreeWidgetItem(parent)
{
    mComponentName = componentName;
    mPortName = portName;
    mDataName = dataName;
    mDataUnit = dataUnit;
    this->setText(0, mComponentName + ", " + mDataName + ", [" + mDataUnit + "]");
}


QString ParameterItem::getComponentName()
{
    return mComponentName;
}

QString ParameterItem::getPortName()
{
    return mPortName;
}

QString ParameterItem::getDataName()
{
    return mDataName;
}

QString ParameterItem::getDataUnit()
{
    return mDataUnit;
}

VariableList::VariableList(MainWindow *parent)
        : QTreeWidget(parent)
{
    mpParentMainWindow = parent;
    mpCurrentSystem = mpParentMainWindow->mpProjectTabs->getCurrentSystem();

    this->setDragEnabled(true);
    this->setAcceptDrops(false);
    this->updateList();
    this->setHeaderHidden(true);
    this->setColumnCount(1);

    connect(mpParentMainWindow->mpProjectTabs, SIGNAL(currentChanged(int)), this, SLOT(updateList()));
    connect(mpParentMainWindow->mpProjectTabs, SIGNAL(tabCloseRequested(int)), this, SLOT(updateList()));
    connect(mpParentMainWindow->mpProjectTabs->getCurrentTab(), SIGNAL(simulationFinished()), this, SLOT(updateList()));
    connect(this,SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)),this,SLOT(createPlot(QTreeWidgetItem*)));
}


//! Updates the list of variables to the available components and parameters in the current tab.
void VariableList::updateList()
{
    //xMap.clear();
    //yMap.clear();
    mAvailableParameters.clear();
    this->clear();

    if(mpParentMainWindow->mpProjectTabs->count() == 0)     //Check so that at least one project tab exists
    {
        return;
    }

    mpCurrentSystem = mpParentMainWindow->mpProjectTabs->getCurrentTab()->mpSystem;
    QVector<double> y;
    QHash<QString, GUIObject *>::iterator it;
    QTreeWidgetItem *tempComponentItem;
    ParameterItem *tempParameterItem;
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
            QVector<QString> parameterNames;
            QVector<QString> parameterUnits;
            mpParentMainWindow->mpProjectTabs->getCurrentTab()->mpSystem->mpCoreSystemAccess->getPlotDataNamesAndUnits((*itp)->getGUIComponentName(), (*itp)->getName(), parameterNames, parameterUnits);

            QVector<double> time = QVector<double>::fromStdVector(mpParentMainWindow->mpProjectTabs->getCurrentTab()->mpSystem->mpCoreSystemAccess->getTimeVector((*itp)->getGUIComponentName(), (*itp)->getName()));

            if(time.size() > 0)     //If time vector is greater than zero we have something to plot!
            {
                for(int i = 0; i!=parameterNames.size(); ++i)
                {
                    y.clear();
                    tempParameterItem = new ParameterItem(it.value()->getName(), (*itp)->getName(), parameterNames[i], parameterUnits[i], tempComponentItem);
                    tempParameterItem->setBackgroundColor(0, backgroundColor);
                    tempComponentItem->addChild(tempParameterItem);
                    mpParentMainWindow->mpProjectTabs->getCurrentTab()->mpSystem->mpCoreSystemAccess->getPlotData((*itp)->getGUIComponentName(), (*itp)->getName(), parameterNames[i], y);
                    QStringList parameterDescription;
                    parameterDescription << (*itp)->getGUIComponentName() << (*itp)->getName() << parameterNames[i];
                    mAvailableParameters.append(parameterDescription);
                }
            }
        }
    }
    this->sortItems(0, Qt::AscendingOrder);
}


//! Helper function that creates a new plot window by using a QTreeWidgetItem in the plot variable tree.
//! @param *item is the tree widget item whos arrays will be looked up from the map and plotted
void VariableList::createPlot(QTreeWidgetItem *item)
{
    //! @todo This may be a problem if subsystem parameters should be displayed as lower levels in the tree, because subsystems will have a parent without being plotable...
    if(item->parent() != 0)     //Top level items cannot be plotted (they represent the components)
    {
        ParameterItem *tempItem = dynamic_cast<ParameterItem *>(item);
        createPlot(tempItem->getComponentName(), tempItem->getPortName(), tempItem->getDataName(), tempItem->getDataUnit());
    }
}


//! Creates a new plot window from specified component and parameter.
//! @param componentName is the name of the desired component
//! @param parameterName is a string containing name of port, data and unit in this format: "portName, dataName, [unitName]"
void VariableList::createPlot(QString componentName, QString portName, QString dataName, QString dataUnit)
{
    //! @todo Add some error handling if component or parameter does not exist!
    //QString lookupName;
    //lookupName = QString(componentName + ", " + portName + ", " + dataName + " [" + dataUnit + "]");

    QString title;
    QString xlabel;
    QString ylabel;

    title.append(QString(componentName + ", " + portName + ", " + dataName + " [" + dataUnit + "]"));
    //ylabel.append(yLabelMap.find(lookupName).value());
    ylabel.append(QString(dataName + " [" + dataUnit + "]"));
    xlabel.append("Time, [s]");    //! @todo Is it ok to assume time as the x-axis like this?

    QVector<double> xVector;
    xVector = QVector<double>::fromStdVector(mpParentMainWindow->mpProjectTabs->getCurrentTab()->mpSystem->mpCoreSystemAccess->getTimeVector(componentName, portName));
    QVector<double> yVector;
    mpParentMainWindow->mpProjectTabs->getCurrentTab()->mpSystem->mpCoreSystemAccess->getPlotData(componentName, portName, dataName, yVector);

    PlotWindow *plotWindow = new PlotWindow(xVector, yVector, this, mpParentMainWindow);
    plotWindow->setWindowTitle("Hopsan NG Plot Window");
    plotWindow->tempCurve->setTitle(title);
    QStringList parameterDescription;
    parameterDescription << componentName << portName << dataName;
    plotWindow->mCurveParameters.append(parameterDescription);
    plotWindow->mpVariablePlot->setAxisTitle(VariablePlot::yLeft, ylabel);
    plotWindow->mpVariablePlot->setAxisTitle(VariablePlot::xBottom, xlabel);
    plotWindow->mpVariablePlot->insertLegend(new QwtLegend(), QwtPlot::TopLegend);
    plotWindow->show();
}


//! Defines what happens when clicking in the variable list. Used to initiate drag operations.
void VariableList::mousePressEvent(QMouseEvent *event)
{
    QTreeWidget::mousePressEvent(event);

    if (event->button() == Qt::LeftButton)
        dragStartPosition = event->pos();
}


//! Defines what happens when mouse is moving in variable list. Used to handle drag operations.
void VariableList::mouseMoveEvent(QMouseEvent *event)
{

    if (!(event->buttons() & Qt::LeftButton))
    {
        return;
    }
    if ((event->pos() - dragStartPosition).manhattanLength() < QApplication::startDragDistance())
    {
        return;
    }

    //QTreeWidgetItem *listItem;
    //listItem = this->currentItem();
    ParameterItem *item;
    //item = dynamic_cast<ParameterItem *>(listItem);
    item = reinterpret_cast<ParameterItem *>(currentItem());

    if(item != 0)
    //if(true)
    {
        QString mimeText;
        mimeText = QString("HOPSANPLOTDATA " + addQuotes(item->getComponentName()) + " " + addQuotes(item->getPortName()) + " " + addQuotes(item->getDataName()) + " " + addQuotes(item->getDataUnit()));
        //mimeText = QString("Gorilla");

        QDrag *drag = new QDrag(this);
        QMimeData *mimeData = new QMimeData;

        mimeData->setText(mimeText);
        drag->setMimeData(mimeData);
        drag->exec();
    }
}


//! This is the main plot widget, which contains the tree with variables
VariableListDialog::VariableListDialog(MainWindow *parent)
        : QWidget(parent)
{
    mpParentMainWindow = parent;

    //Create a grid
    QGridLayout *grid = new QGridLayout(this);

    //Create the plot variables tree
    mpVariableList = new VariableList(mpParentMainWindow);
    grid->addWidget(mpVariableList,0,0,3,1);
}
