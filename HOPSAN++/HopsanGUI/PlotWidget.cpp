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
#include "GUIUtilities.h"
#include "GUISystem.h"

#include "qwt_scale_engine.h"
#include "qwt_symbol.h"
#include "qwt_text_label.h"
#include "qwt_double_rect.h"


//! @brief Constructor for the plot window, where plots are displayed.
//! @param xArray is the x-axis data for the initial plot curve
//! @param yArray is the y-axis data for the initial plot curve
//! @param plotParameterTree is a pointer to the parameter tree from where the plot window was created
//! @param parent is a pointer to the main window
PlotWindow::PlotWindow(QVector<double> xArray, QVector<double> yArray, PlotParameterTree *plotParameterTree, MainWindow *parent)
    : QMainWindow(parent)
{
    this->setAttribute(Qt::WA_DeleteOnClose);

    mpParentMainWindow = parent;
    mpCurrentGUISystem = mpParentMainWindow->mpProjectTabs->getCurrentSystem();
    mpPlotParameterTree = plotParameterTree;

    mHasSpecialXAxis = false;   //This becomes true when user creates an XY-plot

    mHold = false;      //Default settings for "Hold Plot" function

        //Create the actual plot widget
    mpVariablePlot = new QwtPlot();
    mpVariablePlot->setAcceptDrops(false);
    mpVariablePlot->setCanvasBackground(QColor(Qt::white));
    mpVariablePlot->setAutoReplot(true);
    mpVariablePlot->setAxisAutoScale(QwtPlot::yLeft);
    mpVariablePlot->setAxisAutoScale(QwtPlot::xBottom);

    nCurves = 0;

        //Color names for curve colors. A new curve will get next color in list.
        //When all colors are used, the first color is used again.
    mCurveColors << "Blue" << "Red" << "Green" << "Orange" << "Pink" << "Brown" << "Purple" << "Gray";

    mLeftAxisLogarithmic = false;
    mRightAxisLogarithmic = false;

        //Create the toolbar and toolbar buttons
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

    mpExportGNUPLOTButton = new QToolButton(mpToolBar);
    mpExportGNUPLOTButton->setToolTip("Export to GNUPLOT");
    mpExportGNUPLOTButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-SaveToGnuPlot.png"));
    mpExportGNUPLOTButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    mpExportGNUPLOTButton->setAcceptDrops(false);
    mpToolBar->addWidget(mpExportGNUPLOTButton);

    mpImportGNUPLOTButton = new QToolButton(mpToolBar);
    mpImportGNUPLOTButton->setToolTip("Import from GNUPLOT");
    mpImportGNUPLOTButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-LoadGnuPlot.png"));
    mpImportGNUPLOTButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    mpImportGNUPLOTButton->setAcceptDrops(false);
    mpToolBar->addWidget(mpImportGNUPLOTButton);

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

    mpPreviousButton = new QToolButton(mpToolBar);
    mpPreviousButton->setToolTip("Previous Generation");
    mpPreviousButton->setText("<-");
    mpPreviousButton->setAcceptDrops(false);
    mpPreviousButton->setDisabled(true);
    mpToolBar->addWidget(mpPreviousButton);

    mpGenerationLabel = new QLabel(mpToolBar);
    mpGenerationLabel->setText("1 (1)");
    QFont tempFont = mpGenerationLabel->font();
    tempFont.setBold(true);
    mpGenerationLabel->setFont(tempFont);
    mpGenerationLabel->setDisabled(false);
    mpToolBar->addWidget(mpGenerationLabel);

    mpNextButton = new QToolButton(mpToolBar);
    mpNextButton->setToolTip("Next Generation");
    mpNextButton->setText("->");
    mpNextButton->setAcceptDrops(false);
    mpNextButton->setDisabled(false);
    mpToolBar->addWidget(mpNextButton);

    addToolBar(mpToolBar);

        // Create and add curves to the plot
    mCurrentGeneration = 0;
    tempCurve = new QwtPlotCurve();
    QwtArrayData data(xArray,yArray);
    tempCurve->setData(data);
    tempCurve->attach(mpVariablePlot);
    tempCurve->setRenderHint(QwtPlotItem::RenderAntialiased);
    mpVariablePlot->replot();
    tempCurve->setPen(QPen(QBrush(QColor(mCurveColors[nCurves])),mpSizeSpinBox->value()));
    mpCurves.append(tempCurve);
    ++nCurves;

    QList< QVector<double> > tempList;
    tempList.append(xArray);
    mVectorX.append(tempList);
    tempList.first() = yArray;
    mVectorY.append(tempList);

        //Grid
    mpGrid = new QwtPlotGrid;
    mpGrid->enableXMin(true);
    mpGrid->enableYMin(true);
    mpGrid->setMajPen(QPen(Qt::black, 0, Qt::DotLine));
    mpGrid->setMinPen(QPen(Qt::gray, 0 , Qt::DotLine));
    mpGrid->attach(mpVariablePlot);

        //Panning Tool
    mpPanner = new QwtPlotPanner(mpVariablePlot->canvas());
    mpPanner->setMouseButton(Qt::MidButton);

        //Rubber Band Zoom
    mpZoomer = new QwtPlotZoomer( QwtPlot::xBottom, QwtPlot::yLeft, mpVariablePlot->canvas());
    QwtDoubleRect tempDoubleRect;       //The first added curve is used as reference rectangle
    tempDoubleRect.setX(mpCurves.first()->minXValue());
    tempDoubleRect.setY(mpCurves.first()->minYValue());
    tempDoubleRect.setWidth(mpCurves.first()->maxXValue()-mpCurves.first()->minXValue());
    tempDoubleRect.setHeight(mpCurves.first()->maxYValue()-mpCurves.first()->minYValue());
    mpZoomer->setZoomBase(tempDoubleRect);
    mpZoomer->setMaxStackDepth(10000);
    mpZoomer->setSelectionFlags(QwtPicker::DragSelection | QwtPicker::CornerToCorner);
    mpZoomer->setRubberBand(QwtPicker::RectRubberBand);
    mpZoomer->setRubberBandPen(QColor(Qt::green));
    mpZoomer->setTrackerMode(QwtPicker::ActiveOnly);
    mpZoomer->setTrackerPen(QColor(Qt::white));
    mpZoomer->setMousePattern(QwtEventPattern::MouseSelect2, Qt::RightButton, Qt::ControlModifier);
    mpZoomer->setMousePattern(QwtEventPattern::MouseSelect3, Qt::RightButton);

        //Wheel Zoom
    mpMagnifier = new QwtPlotMagnifier(mpVariablePlot->canvas());
    mpMagnifier->setZoomInKey(Qt::Key_Plus, Qt::ControlModifier);
    mpMagnifier->setWheelFactor(1.1);

        //Curve Marker
    mpMarkerSymbol = new QwtSymbol();
    mpMarkerSymbol->setBrush(QBrush(Qt::red, Qt::SolidPattern));
    mpMarkerSymbol->setStyle(QwtSymbol::Ellipse);
    mpMarkerSymbol->setSize(10,10);
    mpActiveMarker = 0;

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
    connect(mpExportGNUPLOTButton,SIGNAL(clicked()),SLOT(exportGNUPLOT()));
    connect(mpImportGNUPLOTButton,SIGNAL(clicked()),SLOT(importGNUPLOT()));
    connect(mpGridButton,SIGNAL(toggled(bool)),SLOT(enableGrid(bool)));
    connect(mpSizeSpinBox,SIGNAL(valueChanged(int)),this, SLOT(setLineWidth(int)));
    connect(mpColorButton,SIGNAL(clicked()),this,SLOT(setLineColor()));
    connect(mpBackgroundColorButton,SIGNAL(clicked()),this,SLOT(setBackgroundColor()));
    connect (mpHoldCheckBox, SIGNAL(toggled(bool)), this, SLOT(setHold(bool)));
    connect(mpPreviousButton,SIGNAL(clicked()),this,SLOT(stepBack()));
    connect(mpNextButton, SIGNAL(clicked()),this,SLOT(stepForward()));
    connect(this->mpParentMainWindow->mpProjectTabs->getCurrentTab(),SIGNAL(simulationFinished()),this,SLOT(checkNewValues()));

    //! @todo Maybe user should be allowed to change default plot window size, or someone will become annoyed...
    resize(600,600);    //Default window size

    this->setAcceptDrops(true);
}


//! @brief Slot that is used to change the "Hold Plot" setting
//! @param value is a boolean that tells whether it should be turned on or off
void PlotWindow::setHold(bool value)
{
    mHold = value;
}


//! @brief Slot that steps back one generation in plot history
void PlotWindow::stepBack()
{
    if(mCurrentGeneration != 0)
    {
        --mCurrentGeneration;
        mpPreviousButton->setDisabled(mCurrentGeneration == 0);
        mpNextButton->setDisabled(false);

        for(int i=0; i<mpCurves.size(); ++i)
        {
            mpCurves[i]->setData(mVectorX[mCurrentGeneration][i], mVectorY[mCurrentGeneration][i]);
        }
        mpVariablePlot->replot();
    }

    QString numStr1;
    QString numStr2;
    numStr1.setNum(mCurrentGeneration+1);
    numStr2.setNum(mVectorX.size());
    mpGenerationLabel->setText(numStr1 + " (" + numStr2 + ")");
}


//! @brief Slot that steps forward one generation in plot history
void PlotWindow::stepForward()
{
    if(mCurrentGeneration != mVectorX.size()-1)
    {
        ++mCurrentGeneration;
        mpNextButton->setDisabled(mCurrentGeneration == mVectorX.size()-1);
        mpPreviousButton->setDisabled(false);

        for(int i=0; i<mpCurves.size(); ++i)
        {
            mpCurves[i]->setData(mVectorX[mCurrentGeneration][i], mVectorY[mCurrentGeneration][i]);
        }
        mpVariablePlot->replot();
    }

    QString numStr1;
    QString numStr2;
    numStr1.setNum(mCurrentGeneration+1);
    numStr2.setNum(mVectorX.size());
    mpGenerationLabel->setText(numStr1 + " (" + numStr2 + ")");
}

//! @brief Inserts a curve marker at the specified curve
//! @param curve is a pointer to the specified curve
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


//! @brief Changes the active marker (the on that can be moved around)
//! @marker is a pointer to the marker that shall be activated
void PlotWindow::setActiveMarker(QwtPlotMarker *marker)
{
    this->mpActiveMarker = marker;
}


//! @brief Slot that enables or disables rubber band zooming
//! @on is true if it shall be enabled or false if it should be disabled
void PlotWindow::enableZoom(bool on)
{
    mpZoomer->setEnabled(on);

    //! @todo Figure out why panner is enabled if zoomer is. Is it needed for something?
    mpPanner->setEnabled(on);
    mpPanner->setMouseButton(Qt::MidButton);

    disconnect(mpPanButton,SIGNAL(toggled(bool)),this,SLOT(enablePan(bool)));
    mpPanButton->setChecked(false);
    connect(mpPanButton,SIGNAL(toggled(bool)),this, SLOT(enablePan(bool)));
    mpPanner->setEnabled(false);
}


//! @brief Slot that enables or disables panning tool
//! @param on is true/false if panning shall be enabled/disabled
void PlotWindow::enablePan(bool on)
{
    mpPanner->setEnabled(on);
    mpPanner->setMouseButton(Qt::LeftButton);

    disconnect(mpZoomButton,SIGNAL(toggled(bool)),this,SLOT(enableZoom(bool)));
    mpZoomButton->setChecked(false);
    connect(mpZoomButton,SIGNAL(toggled(bool)),this,SLOT(enableZoom(bool)));
    mpZoomer->setEnabled(false);
}


//! @brief Slot that turns plot grid on or off
//! @param on is true/false if it shall be turned on/off.
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


//! @brief Slot that exports current plot to .svg format
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


//! @brief Slot that exports a curve to GNUPLOT format
void PlotWindow::exportGNUPLOT()
{
    QMenu menu;

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

    QDir fileDialogSaveDir;
    QString modelFileName = QFileDialog::getSaveFileName(this, tr("Save Model File"),
                                                         fileDialogSaveDir.currentPath(),
                                                         tr("GNUPLOT File (*.GNUPLOT)"));
    QFile file(modelFileName);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qDebug() << "Failed to open file for writing: " + modelFileName;
        return;
    }
    QTextStream modelFile(&file);  //Create a QTextStream object to stream the content of file

    size_t size = pSelectedCurve->data().size();
    for(std::size_t i=0; i!=size; ++i)
    {
        modelFile << pSelectedCurve->data().x(i);
        modelFile << " ";
        modelFile << pSelectedCurve->data().y(i);
        if(i < size-1)      //Do not add line break on the last line in the file
        {
            modelFile << "\n";
        }
    }
    file.close();
}


//! @brief Slot that imports a curve from a file in GNUPLOT format
void PlotWindow::importGNUPLOT()
{
    QDir fileDialogOpenDir;
    QString modelFileName = QFileDialog::getOpenFileName(this, tr("Choose Data File"),
                                                         fileDialogOpenDir.currentPath());
    if (modelFileName.isEmpty())
    {
        return;
    }
    QFile file(modelFileName);
    if(!file.exists())
    {
        qDebug() << "Failed to open file, file not found: " + file.fileName();
        return;
    }
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return;
    }

    QVector<double> xArray;
    QVector<double> yArray;

    QTextStream textStreamFile(&file);

    double tempValue1;
    double tempValue2;
    while( !textStreamFile.atEnd() )
    {
        qDebug() << "Reading something!";
        textStreamFile >> tempValue1;
        xArray.append(tempValue1);
        textStreamFile >> tempValue2;
        yArray.append(tempValue2);
        qDebug() << "(" << tempValue1 << ", " << tempValue2 << ")";
    }
    file.close();
    this->addPlotCurve(xArray, yArray, file.fileName(), "X from GNUPLOT", "Y from GNUPLOT", QwtPlot::yLeft);
}


//! @brief Slot that changes line width of all plot lines
//! @param size is the desired line width in pixels
void PlotWindow::setLineWidth(int size)
{
    for(size_t i=0; i<mpCurves.size(); ++i)
    {
        mpCurves.at(i)->setPen(QPen(mpCurves.at(i)->pen().color(),size));
    }
}

//! @brief Slot that asks for a line and then opens color selection box to change the line color
void PlotWindow::setLineColor()
{
    QMenu menu;

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


//! @brief Slot that opens color selection box to change background color for the plot
void PlotWindow::setBackgroundColor()
{
    QColor color = QColorDialog::getColor(this->mpVariablePlot->canvasBackground(), this);
    if (color.isValid())
    {
        mpVariablePlot->setCanvasBackground(color);
        mpVariablePlot->replot();
    }
}


//! @brief Defines what happens when used drags something into the plot window
void PlotWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasText())
    {
            //Create the hover rectangle (size will be changed by dragMoveEvent)
        mpHoverRect = new QRubberBand(QRubberBand::Rectangle,this);
        mpHoverRect->setGeometry(0, 0, this->width(), this->height());
        mpHoverRect->setWindowOpacity(1);
        mpHoverRect->show();

        event->acceptProposedAction();
    }
}


//! @brief Defines what happens when mouse is moving over plot window with one mouse button pressed. Used to move plot markers.
void PlotWindow::mouseMoveEvent(QMouseEvent *event)
{
    if(mpActiveMarker != 0 && !mpZoomer->isEnabled() && !mpPanner->isEnabled())
    {
        QwtPlotCurve *curve = mMarkerToCurveMap.value(mpActiveMarker);
        QCursor cursor;
        int correctionFactor = mpVariablePlot->canvas()->x();
        int intX = this->mapFromGlobal(cursor.pos()).x() - correctionFactor;
        double x = mpVariablePlot->canvasMap(curve->xAxis()).invTransform(intX);
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
            x = xDataPos*curve->maxXValue() / curve->dataSize();
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


//! @brief Defines what happens when user is dragging something in the plot window.
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


//! @brief Defines what happens when user drags something out from the plot window.
void PlotWindow::dragLeaveEvent(QDragLeaveEvent *event)
{
    delete(mpHoverRect);
    QMainWindow::dragLeaveEvent(event);
}


//! @brief Defines what happens when user drops something in the plot window
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
                this->changeXVector(yVector, ylabel, componentName, portName, dataName);
            }
            else if(this->mapFromGlobal(cursor.pos()).x() < this->width()/2)
            {
                this->addPlotCurve(xVector, yVector, title, xlabel, ylabel, QwtPlot::yLeft);
                QStringList parameterDescription;
                parameterDescription << componentName << portName << dataName << dataUnit;
                mCurveParameters.append(parameterDescription);
            }
            else
            {
                this->addPlotCurve(xVector, yVector, title, xlabel, ylabel, QwtPlot::yRight);
                QStringList parameterDescription;
                parameterDescription << componentName << portName << dataName << dataUnit;
                mCurveParameters.append(parameterDescription);
            }
        }
    }
}


//! @brief Handles the right-click menu in the plot window
void PlotWindow::contextMenuEvent(QContextMenuEvent *event)
{
    if(this->mpZoomer->isEnabled())
    {
        return;
    }

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
//! @param xArray is the vector for the x-axis
//! @param yArray is the vector for the y-axis
//! @param title is the title of the curve
//! @param xLabel is the label for the x-axis
//! @param yLabel is the label for the y-axis
//! @param axisY tells whether the right or left y-axis shall be used
void PlotWindow::addPlotCurve(QVector<double> xArray, QVector<double> yArray, QString title, QString xLabel, QString yLabel, QwtPlot::Axis axisY)
{
    mVectorX[mCurrentGeneration].append(xArray);
    mVectorY[mCurrentGeneration].append(yArray);

        // Create and add curves to the plot
    tempCurve = new QwtPlotCurve(title);
    if(!mHasSpecialXAxis)
    {
        tempCurve->setData(xArray, yArray);
        mpVariablePlot->setAxisTitle(QwtPlot::xBottom, xLabel);
    }
    else
    {
        QVector<double> tempXArray;
        for(size_t j=0; j<mpCurves.last()->data().size(); ++j)
        {
            tempXArray.append(mpCurves.last()->data().x(j));
        }
        tempCurve->setData(tempXArray, yArray);
    }
    tempCurve->attach(mpVariablePlot);
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
        nCurves = 0;        // This restarts the color loop when too many curves are added
    }

    mpVariablePlot->setAxisTitle(QwtPlot::yLeft, yLabel);
    mpVariablePlot->insertLegend(new QwtLegend(), QwtPlot::TopLegend);
}


//! @brief Function the x-axis in the plot, and updates all curves corresponding to this. Used to create XY-plots.
//! @param xArray is the data for the x-axis
//! @param xLabel is the label text to be displayed on the x-axis
//! @param componentName is the name of the component where the port is located
//! @param portName is the name of the port where the parameter is located
//! @param dataName is the name of the parameter
void PlotWindow::changeXVector(QVector<double> xArray, QString xLabel, QString componentName, QString portName, QString dataName)
{
    QVector<double> tempYArray;
    for(size_t i=0; i<mpCurves.size(); ++i)
    {
            //Loop through each y-axis and append each value to a new array
        for(size_t j=0; j<mpCurves.at(i)->data().size(); ++j)       //! @todo Figure out a less stupid way of replacing only the x values...
        {
            tempYArray.append(mpCurves.at(i)->data().y(j));
        }
            //Change the curve data to the new x-data and the temporary y-array
        mpCurves.at(i)->setData(xArray, tempYArray);
        tempYArray.clear();
    }
    mpVariablePlot->setAxisTitle(QwtPlot::xBottom, xLabel);
    mHasSpecialXAxis = true;
    mSpecialXParameter.clear();
    mSpecialXParameter << componentName << portName << dataName;
}


//! @brief Slot that updates the values for the curve, if the component/port still exist in the model
void PlotWindow::checkNewValues()
{
    if(mHold)       //Do not update curves to new values if hold is checked
    {
        return;
    }

    QList< QVector<double> > tempList;
    mVectorX.append(tempList);
    mVectorY.append(tempList);

    mCurrentGeneration = mVectorX.size()-1;
    QString numStr1;
    QString numStr2;
    numStr1.setNum(mCurrentGeneration+1);
    numStr2.setNum(mVectorX.size());
    mpGenerationLabel->setText(numStr1 + " (" + numStr2 + ")");

    mpNextButton->setDisabled(true);
    mpPreviousButton->setDisabled(false);

    for(int i=0; i<mpCurves.size(); ++i)
    {
        if(mpPlotParameterTree->mAvailableParameters.contains(mCurveParameters[i]))
        {
            QVector<double> xVector;
            if(mHasSpecialXAxis && mpPlotParameterTree->mAvailableParameters.contains(mSpecialXParameter))
            {
                mpParentMainWindow->mpProjectTabs->getCurrentSystem()->mpCoreSystemAccess->getPlotData(mSpecialXParameter[0], mSpecialXParameter[1], mSpecialXParameter[2], xVector);
            }
            else
            {
                xVector = QVector<double>::fromStdVector(mpParentMainWindow->mpProjectTabs->getCurrentTab()->mpSystem->mpCoreSystemAccess->getTimeVector(mCurveParameters[i][0], mCurveParameters[i][1]));
            }
            QVector<double> yVector;
            mpParentMainWindow->mpProjectTabs->getCurrentSystem()->mpCoreSystemAccess->getPlotData(mCurveParameters[i][0], mCurveParameters[i][1], mCurveParameters[i][2], yVector);

            mpCurves[i]->setData(xVector, yVector);
            mpVariablePlot->replot();

            mVectorX[mCurrentGeneration].append(xVector);
            mVectorY[mCurrentGeneration].append(yVector);
        }
    }
}


//! @brief Constructor for the parameter items in the parameter tree
//! @param componentName Name of the component where the parameter is located
//! @param portName Name of the port where the parameter is located
//! @param dataName Name of the parameter
//! @param dataUnit Name of the unit of the parameter
//! @param parent Pointer to a tree widget item, not used
PlotParameterItem::PlotParameterItem(QString componentName, QString portName, QString dataName, QString dataUnit, QTreeWidgetItem *parent)
        : QTreeWidgetItem(parent)
{
    mComponentName = componentName;
    mPortName = portName;
    mDataName = dataName;
    mDataUnit = dataUnit;
    this->setText(0, mPortName + ", " + mDataName + ", [" + mDataUnit + "]");
}


//! @brief Returns the name of the component where the parameter is located
QString PlotParameterItem::getComponentName()
{
    return mComponentName;
}


//! @brief Returns the name of the port where the parameter is located
QString PlotParameterItem::getPortName()
{
    return mPortName;
}


//! @brief Returns the name of the parameter
QString PlotParameterItem::getDataName()
{
    return mDataName;
}


//! @brief Returns the name of the unit of the parameter
QString PlotParameterItem::getDataUnit()
{
    return mDataUnit;
}


//! @brief Constructor for the parameter tree widget
//! @param parent Pointer to the main window
PlotParameterTree::PlotParameterTree(MainWindow *parent)
        : QTreeWidget(parent)
{
    mpParentMainWindow = parent;
    mpCurrentSystem = mpParentMainWindow->mpProjectTabs->getCurrentSystem();
    mFavoriteParameters.clear();

    this->setDragEnabled(true);
    this->setAcceptDrops(false);
    this->updateList();
    this->setHeaderHidden(true);
    this->setColumnCount(1);

    connect(mpParentMainWindow->mpProjectTabs, SIGNAL(currentChanged(int)), this, SLOT(updateList()));
    connect(mpParentMainWindow->mpProjectTabs, SIGNAL(tabCloseRequested(int)), this, SLOT(updateList()));
    connect(mpParentMainWindow->mpProjectTabs, SIGNAL(newTabAdded()), this, SLOT(updateList()));
    connect(mpParentMainWindow->mpProjectTabs->getCurrentSystem(), SIGNAL(componentChanged()), this, SLOT(updateList()));
    connect(mpParentMainWindow->mpProjectTabs->getCurrentTab(), SIGNAL(simulationFinished()), this, SLOT(updateList()));
    connect(this,SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)),this,SLOT(createPlotWindow(QTreeWidgetItem*)));
}


//! @brief Updates the parameter tree to the available components and parameters in the current tab.
void PlotParameterTree::updateList()
{
    mAvailableParameters.clear();
    this->clear();

    if(mpParentMainWindow->mpProjectTabs->count() == 0)     //Check so that at least one project tab exists
    {
        return;
    }

    mpCurrentSystem = mpParentMainWindow->mpProjectTabs->getCurrentTab()->mpSystem;
    QTreeWidgetItem *tempComponentItem;     //Tree item for components
    PlotParameterItem *tempPlotParameterItem;       //Tree item for parameters - reimplemented so they can store information about the parameter

    QHash<QString, GUIObject *>::iterator it;
    for(it = mpCurrentSystem->mGUIObjectMap.begin(); it!=mpCurrentSystem->mGUIObjectMap.end(); ++it)
    {
        tempComponentItem = new QTreeWidgetItem();
        tempComponentItem->setText(0, it.value()->getName());
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
                    tempPlotParameterItem = new PlotParameterItem(it.value()->getName(), (*itp)->getName(), parameterNames[i], parameterUnits[i], tempComponentItem);
                    tempComponentItem->addChild(tempPlotParameterItem);
                    QStringList parameterDescription;
                    parameterDescription << (*itp)->getGUIComponentName() << (*itp)->getName() << parameterNames[i] << parameterUnits[i];
                    mAvailableParameters.append(parameterDescription);
                    if(mFavoriteParameters.contains(parameterDescription))
                    {
                        tempPlotParameterItem->setIcon(0, QIcon(QString(ICONPATH) + "Hopsan-Favorite.png"));
                    }
                }
            }
        }
    }

        //Append favorite plot variables to tree if they still exist
    for(size_t i=0; i<mFavoriteParameters.size(); ++i)
    {
        if(mAvailableParameters.contains(mFavoriteParameters.at(i)))
        {
            QString componentName = mFavoriteParameters.at(i).at(0);
            QString portName = mFavoriteParameters.at(i).at(1);
            QString dataName = mFavoriteParameters.at(i).at(2);
            QString dataUnit = mFavoriteParameters.at(i).at(3);

            tempPlotParameterItem = new PlotParameterItem(componentName, portName, dataName, dataUnit);
            tempPlotParameterItem->setText(0, tempPlotParameterItem->text(0).prepend(" " + componentName + ", "));
            tempPlotParameterItem->setIcon(0, QIcon(QString(ICONPATH) + "Hopsan-Favorite.png"));
            this->addTopLevelItem(tempPlotParameterItem);
        }
    }

        //Remove no longer existing favorite parameters
    for(size_t i=0; i<mFavoriteParameters.size(); ++i)
    {
        if(!mAvailableParameters.contains(mFavoriteParameters.at(i)))
        {
            mFavoriteParameters.removeAll(mFavoriteParameters.at(i));
        }
    }

        //Sort the tree widget
    this->sortItems(0, Qt::AscendingOrder);

        // This connection makes sure that the plot list is connected to the new tab, so that it will update if the new tab is simulated.
        // It must first be disconnected in case it was already connected, to avoid duplication of connection.
    disconnect(mpParentMainWindow->mpProjectTabs->getCurrentTab(), SIGNAL(simulationFinished()), this, SLOT(updateList()));
    connect(mpParentMainWindow->mpProjectTabs->getCurrentTab(), SIGNAL(simulationFinished()), this, SLOT(updateList()));
}


//! @brief Helper function that creates a new plot window by using a QTreeWidgetItem in the plot variable tree.
//! @param *item Pointer to the tree widget item whos arrays will be looked up from the map and plotted
void PlotParameterTree::createPlotWindow(QTreeWidgetItem *item)
{
    //! @todo This is a kind of dumb check; it assumes that component items have bold font and variables not.
    if(!item->font(0).bold())     //Top level items cannot be plotted (they represent the components)
    {
        //QTreeWidgetItem must be casted to a PlotParameterItem. This is a necessary because double click event can not know which kind of tree item is clicked.
        PlotParameterItem *tempItem = dynamic_cast<PlotParameterItem *>(item);
        createPlotWindow(tempItem->getComponentName(), tempItem->getPortName(), tempItem->getDataName(), tempItem->getDataUnit());
    }
}


//! @brief Creates a new plot window from specified component and parameter.
//! @param componentName Name of the component where the port with the parameter is located
//! @param portName Name of the port where the parameter is located
//! @param dataName Name of the parameter
//! @param dataUnit Name of the unit of the parameter
void PlotParameterTree::createPlotWindow(QString componentName, QString portName, QString dataName, QString dataUnit)
{
    //! @todo Add some error handling if component or parameter does not exist

    QString title;
    QString xlabel;
    QString ylabel;

    title.append(QString(componentName + ", " + portName + ", " + dataName + " [" + dataUnit + "]"));
    ylabel.append(QString(dataName + " [" + dataUnit + "]"));
    xlabel.append("Time, [s]");    //! @todo Maybe we shall not hard code time as the default x-axis, with seconds as default unit?

    QVector<double> xVector;
    xVector = QVector<double>::fromStdVector(mpParentMainWindow->mpProjectTabs->getCurrentTab()->mpSystem->mpCoreSystemAccess->getTimeVector(componentName, portName));
    QVector<double> yVector;
    mpParentMainWindow->mpProjectTabs->getCurrentTab()->mpSystem->mpCoreSystemAccess->getPlotData(componentName, portName, dataName, yVector);

    PlotWindow *plotWindow = new PlotWindow(xVector, yVector, this, mpParentMainWindow);
    plotWindow->setWindowTitle("Hopsan NG Plot Window");
    plotWindow->tempCurve->setTitle(title);
    QStringList parameterDescription;
    parameterDescription << componentName << portName << dataName << dataUnit;
    plotWindow->mCurveParameters.append(parameterDescription);
    plotWindow->mpVariablePlot->setAxisTitle(QwtPlot::yLeft, ylabel);
    plotWindow->mpVariablePlot->setAxisTitle(QwtPlot::xBottom, xlabel);
    plotWindow->mpVariablePlot->insertLegend(new QwtLegend(), QwtPlot::TopLegend);
    plotWindow->show();
}


//! @brief Defines what happens when clicking in the variable list. Used to initiate drag operations.
void PlotParameterTree::mousePressEvent(QMouseEvent *event)
{
    QTreeWidget::mousePressEvent(event);

    if (event->button() == Qt::LeftButton)
        dragStartPosition = event->pos();
}


//! @brief Defines what happens when mouse is moving in variable list. Used to handle drag operations.
void PlotParameterTree::mouseMoveEvent(QMouseEvent *event)
{

    if (!(event->buttons() & Qt::LeftButton))
    {
        return;
    }
    if ((event->pos() - dragStartPosition).manhattanLength() < QApplication::startDragDistance())
    {
        return;
    }

    PlotParameterItem *item;
    item = dynamic_cast<PlotParameterItem *>(currentItem());

    if(item != 0)
    {
        QString mimeText;
        mimeText = QString("HOPSANPLOTDATA " + addQuotes(item->getComponentName()) + " " + addQuotes(item->getPortName()) + " " + addQuotes(item->getDataName()) + " " + addQuotes(item->getDataUnit()));
        QDrag *drag = new QDrag(this);
        QMimeData *mimeData = new QMimeData;
        mimeData->setText(mimeText);
        drag->setMimeData(mimeData);
        drag->exec();
    }
}


//! @brief Defines the right-click menu in the parameter tree
void PlotParameterTree::contextMenuEvent(QContextMenuEvent *event)
{
    qDebug() << "contextMenuEvent()";

    PlotParameterItem *item;

    //! @todo Dumb check that assumes component tree items to be bold and parameter items to be not bold
    if(currentItem() != 0 && !currentItem()->font(0).bold())
    {
        qDebug() << "currentItem() is ok!";

        item = dynamic_cast<PlotParameterItem *>(currentItem());
        QStringList parameterDescription;
        parameterDescription << item->getComponentName() << item->getPortName() << item->getDataName() << item->getDataUnit();
        QMenu menu;
        if(!mFavoriteParameters.contains(parameterDescription))
        {
            QAction *addToFavoritesAction;
            addToFavoritesAction = menu.addAction(QString("Add Favorite Parameter"));

            QCursor *cursor;
            QAction *selectedAction = menu.exec(cursor->pos());

            if(selectedAction == addToFavoritesAction)
            {
               mFavoriteParameters.append(parameterDescription);
               this->updateList();
            }
        }
        else
        {
            QAction *removeFromFavoritesAction;
            removeFromFavoritesAction = menu.addAction(QString("Remove Favorite Parameter"));

            QCursor *cursor;
            QAction *selectedAction = menu.exec(cursor->pos());

            if(selectedAction == removeFromFavoritesAction)
            {
               mFavoriteParameters.removeAll(parameterDescription);
               this->updateList();
            }
        }
    }
}


//! @brief Constructor the main plot widget, which contains the tree with variables
//! @param parent Pointer to the main window
PlotWidget::PlotWidget(MainWindow *parent)
        : QWidget(parent)
{
    mpParentMainWindow = parent;

    mpPlotParameterTree = new PlotParameterTree(mpParentMainWindow);
    mpLayout = new QGridLayout(this);
    mpLayout->addWidget(mpPlotParameterTree,0,0,3,1);
}
