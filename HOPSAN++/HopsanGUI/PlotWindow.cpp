#include <QDebug>
#include <QSpinBox>
#include <QColorDialog>
#include <QLabel>
#include <QCursor>
#include <QAction>

#include <cstring>

#include "PlotWidget.h"
#include "PlotWindow.h"

#include "MainWindow.h"
#include "ProjectTabWidget.h"
//#include "GUIObject.h"
#include "GUIPort.h"
#include "GraphicsView.h"
#include "Utilities/GUIUtilities.h"
#include "GUIObjects/GUISystem.h"
#include "Configuration.h"
#include "loadObjects.h"

#include "qwt_scale_engine.h"
#include "qwt_symbol.h"
#include "qwt_text_label.h"
#include "qwt_double_rect.h"


//! @brief Constructor for the plot window, where plots are displayed.
//! @param plotParameterTree is a pointer to the parameter tree from where the plot window was created
//! @param parent is a pointer to the main window
PlotWindow::PlotWindow(PlotParameterTree *plotParameterTree, MainWindow *parent)
    : QMainWindow(parent)

{
    this->setAttribute(Qt::WA_DeleteOnClose);
    this->setWindowTitle("Hopsan NG Plot Window");
    this->setAcceptDrops(true);
    resize(700,600);    //! @todo Maybe user should be allowed to change default plot window size, or someone will become annoyed...

    //mpParentMainWindow = parent;
    mpCurrentGUISystem = gpMainWindow->mpProjectTabs->getCurrentSystem();
    mpPlotParameterTree = plotParameterTree;

        //Default settings
    mHasSpecialXAxis = false;
    mLeftAxisLogarithmic = false;
    mRightAxisLogarithmic = false;
    mAutoUpdate = true;

        //Initiate default values for left y-axis
    mCurrentUnitsLeft.insert("Value", gConfig.getDefaultUnit("Value"));
    mCurrentUnitsLeft.insert("Pressure", gConfig.getDefaultUnit("Pressure"));
    mCurrentUnitsLeft.insert("Flow", gConfig.getDefaultUnit("Flow"));
    mCurrentUnitsLeft.insert("Position", gConfig.getDefaultUnit("Position"));
    mCurrentUnitsLeft.insert("Velocity", gConfig.getDefaultUnit("Velocity"));
    mCurrentUnitsLeft.insert("Force", gConfig.getDefaultUnit("Force"));

        //Initiate default values for right y-axis
    mCurrentUnitsRight.insert("Value", gConfig.getDefaultUnit("Value"));
    mCurrentUnitsRight.insert("Pressure", gConfig.getDefaultUnit("Pressure"));
    mCurrentUnitsRight.insert("Flow", gConfig.getDefaultUnit("Flow"));
    mCurrentUnitsRight.insert("Position", gConfig.getDefaultUnit("Position"));
    mCurrentUnitsRight.insert("Velocity", gConfig.getDefaultUnit("Velocity"));
    mCurrentUnitsRight.insert("Force", gConfig.getDefaultUnit("Force"));

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

    mpSaveButton = new QToolButton(mpToolBar);
    mpSaveButton->setToolTip("Save Plot Window");
    mpSaveButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-Save.png"));
    mpSaveButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    mpSaveButton->setAcceptDrops(false);
    mpToolBar->addWidget(mpSaveButton);

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

    mpToolBar->addSeparator();

    mpAutoUpdateCheckBox = new QCheckBox("Auto Update");
    mpAutoUpdateCheckBox->setChecked(mAutoUpdate);
    mpToolBar->addWidget(mpAutoUpdateCheckBox);

    mpDiscardGenerationButton = new QToolButton(mpToolBar);
    mpDiscardGenerationButton->setToolTip("Discard Generation");
    mpDiscardGenerationButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-Discard.png"));
    mpDiscardGenerationButton->setAcceptDrops(false);
    mpDiscardGenerationButton->setDisabled(true);
    mpToolBar->addWidget(mpDiscardGenerationButton);

    mpPreviousButton = new QToolButton(mpToolBar);
    mpPreviousButton->setToolTip("Previous Generation");
    mpPreviousButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-StepLeft.png"));
    mpPreviousButton->setAcceptDrops(false);
    mpPreviousButton->setDisabled(true);
    mpToolBar->addWidget(mpPreviousButton);

    mpNextButton = new QToolButton(mpToolBar);
    mpNextButton->setToolTip("Next Generation");
    mpNextButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-StepRight.png"));
    mpNextButton->setAcceptDrops(false);
    mpNextButton->setDisabled(true);
    mpToolBar->addWidget(mpNextButton);

    mpGenerationLabel = new QLabel(mpToolBar);
    mpGenerationLabel->setText("Generation 1 (1)");
    QFont tempFont = mpGenerationLabel->font();
    tempFont.setBold(true);
    mpGenerationLabel->setFont(tempFont);
    mpGenerationLabel->setDisabled(false);
    mpToolBar->addWidget(mpGenerationLabel);

    addToolBar(mpToolBar);

    mpVariablePlot->insertLegend(new QwtLegend(), QwtPlot::TopLegend);
    mCurrentGeneration = 0;

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

        //Disables zoom function (activated by tool button, off by default)
    enableZoom(false);

        //Establish signal and slots connections
    connect(buttonbox, SIGNAL(rejected()), this, SLOT(close()));
    connect(mpZoomButton,SIGNAL(toggled(bool)),SLOT(enableZoom(bool)));
    connect(mpPanButton,SIGNAL(toggled(bool)),SLOT(enablePan(bool)));
    connect(mpSaveButton,SIGNAL(clicked()),this,SLOT(saveToXml()));
    connect(mpSVGButton,SIGNAL(clicked()),SLOT(exportSVG()));
    connect(mpExportGNUPLOTButton,SIGNAL(clicked()),SLOT(exportGNUPLOT()));
    connect(mpImportGNUPLOTButton,SIGNAL(clicked()),SLOT(importGNUPLOT()));
    connect(mpGridButton,SIGNAL(toggled(bool)),SLOT(enableGrid(bool)));
    connect(mpSizeSpinBox,SIGNAL(valueChanged(int)),this, SLOT(setLineWidth(int)));
    connect(mpColorButton,SIGNAL(clicked()),this,SLOT(setLineColor()));
    connect(mpBackgroundColorButton,SIGNAL(clicked()),this,SLOT(setBackgroundColor()));
    connect(mpAutoUpdateCheckBox, SIGNAL(toggled(bool)), this, SLOT(setAutoUpdate(bool)));
    connect(mpPreviousButton,SIGNAL(clicked()),this,SLOT(stepBack()));
    connect(mpNextButton, SIGNAL(clicked()),this,SLOT(stepForward()));
    connect(mpDiscardGenerationButton,SIGNAL(clicked()),this,SLOT(discardGeneration()));
    connect(gpMainWindow->mpProjectTabs->getCurrentTab(),SIGNAL(simulationFinished()),this,SLOT(checkNewValues()));
}


//! @brief Slot that is used to change the "Hold Plot" setting
//! @param value is a boolean that tells whether it should be turned on or off
void PlotWindow::setAutoUpdate(bool value)
{
    mAutoUpdate = value;
}


//! @brief Slot that steps back one generation in plot history
void PlotWindow::stepBack()
{
    if(mCurrentGeneration != 0)
    {
        setGeneration(mCurrentGeneration-1);
    }
}


//! @brief Slot that steps forward one generation in plot history
void PlotWindow::stepForward()
{
    if(mCurrentGeneration != mVectorX.size()-1)
    {
        setGeneration(mCurrentGeneration+1);
    }
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


//! @brief Changes plot variable generation in the plot window
//! @param gen Number of the desired generation
//! @todo Add a check that the generation exists
void PlotWindow::setGeneration(int gen)
{
    mCurrentGeneration = gen;

    mpPreviousButton->setDisabled(mCurrentGeneration == 0);
    mpNextButton->setDisabled(mCurrentGeneration == mVectorX.size()-1);

    for(int i=0; i<mpCurves.size(); ++i)
    {
        if((mVectorX[mCurrentGeneration].size()-1 < i) || (mVectorY[mCurrentGeneration][i].empty()))
        {
            mpCurves[i]->hide();
        }
        else
        {
            mpCurves[i]->show();
            mpCurves[i]->setData(mVectorX[mCurrentGeneration][i], mVectorY[mCurrentGeneration][i]);
        }
    }
    mpVariablePlot->replot();

    QString numStr1;
    QString numStr2;
    numStr1.setNum(mCurrentGeneration+1);
    numStr2.setNum(mVectorX.size());
    mpGenerationLabel->setText("Generation " + numStr1 + " (" + numStr2 + ")");
}


//! @brief Slot that removes the current generation from the plot window.
//! @todo There is no check that the number of generations is greater than one. The button shall always be disabled then anyway, but if this is called from outside it will cause problems.
void PlotWindow::discardGeneration()
{
    mVectorX.removeAt(mCurrentGeneration);
    mVectorY.removeAt(mCurrentGeneration);
    --mCurrentGeneration;
    if(mCurrentGeneration < 0)
    {
        mCurrentGeneration = 0;
    }
    setGeneration(mCurrentGeneration);

    mpDiscardGenerationButton->setEnabled(mVectorX.size() > 1);
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
    this->addPlotCurve(xArray, yArray, file.fileName(), "-", "-", "-", QwtPlot::yLeft);
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

            QVector<double> xVector = QVector<double>::fromStdVector(gpMainWindow->mpProjectTabs->getCurrentSystem()->getCoreSystemAccessPtr()->getTimeVector(componentName, portName));
            QVector<double> yVector;
            gpMainWindow->mpProjectTabs->getCurrentSystem()->getCoreSystemAccessPtr()->getPlotData(componentName, portName, dataName, yVector);

            QCursor cursor;
            if(this->mapFromGlobal(cursor.pos()).y() > this->height()/2 && mpCurves.size() >= 1)
            {
                this->changeXVector(yVector, componentName, portName, dataName, dataUnit);
            }
            else if(this->mapFromGlobal(cursor.pos()).x() < this->width()/2)
            {
                this->addPlotCurve(xVector, yVector, componentName, portName, dataName, dataUnit, QwtPlot::yLeft);
            }
            else
            {
                this->addPlotCurve(xVector, yVector, componentName, portName, dataName, dataUnit, QwtPlot::yRight);
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
    QMenu *changeUnitMenuLeft;
    QMenu *changeUnitMenuRight;
    QMenu *removeCurveMenu;

    yAxisLeftMenu = menu.addMenu(QString("Left Y Axis"));
    yAxisRightMenu = menu.addMenu(QString("Right Y Axis"));


        //Create menu for changing unit on left axis
    changeUnitMenuLeft = yAxisLeftMenu->addMenu(QString("Change Unit"));
    QString physicalQuantityLeft = QString(mpVariablePlot->axisTitle(QwtPlot::yLeft).text().toStdString().substr(0, mpVariablePlot->axisTitle(QwtPlot::yLeft).text().toStdString().find(' ')).c_str());
    QMap<QString, double>::iterator itul;
    for(itul=gConfig.getCustomUnits(physicalQuantityLeft).begin(); itul!=gConfig.getCustomUnits(physicalQuantityLeft).end(); ++itul)
    {
        QAction *tempAction = changeUnitMenuLeft->addAction(itul.key());
        std::string axisTitle = mpVariablePlot->axisTitle(QwtPlot::yLeft).text().toStdString();
        if(axisTitle.substr(axisTitle.find("[")+1, axisTitle.find("]")-axisTitle.find("[")-1) == itul.key().toStdString())
        {
           QFont tempFont = tempAction->font();
            tempFont.setBold(true);
            tempAction->setFont(tempFont);
        }
    }


        //Create menu for changing unit on right axis
    QString physicalQuantityRight;
    if(mpVariablePlot->axisEnabled(QwtPlot::yRight))
    {
        changeUnitMenuRight = yAxisRightMenu->addMenu(QString("Change Unit"));
        physicalQuantityRight = QString(mpVariablePlot->axisTitle(QwtPlot::yRight).text().toStdString().substr(0, mpVariablePlot->axisTitle(QwtPlot::yRight).text().toStdString().find(' ')).c_str());
        QMap<QString, double>::iterator itur;
        for(itur=gConfig.getCustomUnits(physicalQuantityRight).begin(); itur!=gConfig.getCustomUnits(physicalQuantityRight).end(); ++itur)
        {
            QAction *tempAction = changeUnitMenuRight->addAction(itur.key());
            std::string axisTitle = mpVariablePlot->axisTitle(QwtPlot::yRight).text().toStdString();
            if(axisTitle.substr(axisTitle.find("[")+1, axisTitle.find("]")-axisTitle.find("[")-1) == itur.key().toStdString())
            {
                QFont tempFont = tempAction->font();
                tempFont.setBold(true);
                tempAction->setFont(tempFont);
            }
        }
    }


        //Create actions for making axis logarithmic
    QAction *setRightAxisLogarithmic;
    QAction *setLeftAxisLogarithmic;
    setRightAxisLogarithmic = yAxisRightMenu->addAction("Logarithmic Scale");
    setLeftAxisLogarithmic = yAxisLeftMenu->addAction("Logarithmic Scale");
    setRightAxisLogarithmic->setCheckable(true);
    setLeftAxisLogarithmic->setCheckable(true);
    setRightAxisLogarithmic->setChecked(mRightAxisLogarithmic);
    setLeftAxisLogarithmic->setChecked(mLeftAxisLogarithmic);


        //Create menu for insereting curve markers
    insertMarkerMenu = menu.addMenu(QString("Insert Curve Marker"));
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


        //Create menu for selecting curve markers
    selectMarkerMenu = menu.addMenu(QString("Change Active Marker"));
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


        //Create menu for removing curves
    removeCurveMenu = menu.addMenu(QString("Remove Plot Curve"));
    for(size_t i=0; i<mpCurves.size(); ++i)
    {
        tempAction = removeCurveMenu->addAction(mpCurves[i]->title().text());
        actionToCurveMap.insert(tempAction, mpCurves[i]);
    }




    // ----- Wait for user to make a selection ----- //

    QCursor *cursor;
    QAction *selectedAction = menu.exec(cursor->pos());

    // ----- User has selected something -----  //



        // Check if user did not click on a menu item
    if(selectedAction == 0)
    {
        return;
    }


        // Change unit on left axis
    if((selectedAction->parentWidget() == changeUnitMenuLeft) && (gConfig.getCustomUnits(physicalQuantityLeft).contains(selectedAction->text())))
    {
        this->setUnit(QwtPlot::yLeft, physicalQuantityLeft, selectedAction->text());
    }


        // Change unit on right axis
    if((selectedAction->parentWidget() == changeUnitMenuRight) && (gConfig.getCustomUnits(physicalQuantityRight).contains(selectedAction->text())))
    {
        this->setUnit(QwtPlot::yRight, physicalQuantityRight, selectedAction->text());
    }


        //Make axis logarithmic
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


        //Insert curve marker
    QMap<QAction *, QwtPlotCurve *>::iterator it;
    if(selectedAction->parentWidget() == insertMarkerMenu)
    {
        for(it = actionToCurveMap.begin(); it!=actionToCurveMap.end(); ++it)
        {
            if(selectedAction == it.key())
            {
                this->insertMarker(it.value());
            }
        }
    }

        //Change active curve marker
    QMap<QAction *, QwtPlotMarker *>::iterator itm;
    for(itm = actionToMarkerMap.begin(); itm!=actionToMarkerMap.end(); ++itm)
    {
        if(selectedAction == itm.key())
        {
            this->setActiveMarker(itm.value());
        }
    }


        //Remove plot curve
    if(selectedAction->parentWidget() == removeCurveMenu)
    {
        for(it = actionToCurveMap.begin(); it!=actionToCurveMap.end(); ++it)
        {
            if(selectedAction == it.key())
            {
                it.value()->detach();
                size_t i;
                for(i=0; i<mpCurves.size(); ++i)
                {
                    if(mpCurves[i] == it.value())
                    {
                        break;
                    }
                }
                mpCurves.remove(i);
                delete(it.value());
            }
        }
        mpVariablePlot->replot();
    }
}


//! Changes physical unit on specified y axis
//! @param yAxis Number of the Y-axis (QwtPlot::yLeft or QwtPlot::yRight)
//! @param physicalQuantity Name of the physical quantity
//! @param selectedUnit Name of the new unit
void PlotWindow::setUnit(int yAxis, QString physicalQuantity, QString selectedUnit)
{
    double scale = gConfig.getCustomUnits(physicalQuantity).find(selectedUnit).value();

    for(size_t i=0; i<mpCurves.size(); ++i)
    {
        if(mpCurves.at(i)->yAxis() == yAxis)
        {
                //Change the curve data to the new x-data and the temporary y-array
            QVector<double> tempVectorY;
            for(size_t j=0; j<mVectorY[mCurrentGeneration][i].size(); ++j)
            {
                tempVectorY.append(mVectorY[mCurrentGeneration][i][j]*scale);
            }
            mpCurves.at(i)->setData(mVectorX[mCurrentGeneration][i], tempVectorY);
            //mCurveParameters[i][3] = selectedUnit;
        }
    }

    mpVariablePlot->setAxisTitle(yAxis, physicalQuantity + " [" + selectedUnit + "]");
}


//! @brief Help function to add a new curve to an existing plot window
//! @param xArray is the vector for the x-axis
//! @param yArray is the vector for the y-axis
//! @param componentName Name of the where parameter is located
//! @param portName Name of the port where the parameter is located
//! @param dataName Name of parameter
//! @param dataUnit Unit of the parameter
//! @param axisY tells whether the right or left y-axis shall be used
void PlotWindow::addPlotCurve(QVector<double> xArray, QVector<double> yArray, QString componentName, QString portName, QString dataName, QString dataUnit, int axisY)
{
    QString title = QString(componentName + ", " + portName + ", " + dataName + " [" + dataUnit + "]");
    QString xLabel = "Time [s]";

    bool firstCurve = false;
    if(mVectorX.isEmpty())
    {
        firstCurve = true;
        QList< QVector<double> > tempList;
        tempList.append(xArray);
        mVectorX.append(tempList);
        tempList.clear();
        tempList.append(yArray);
        mVectorY.append(tempList);
    }
    else
    {
        mVectorX[mCurrentGeneration].append(xArray);
        mVectorY[mCurrentGeneration].append(yArray);
    }

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

        //Change to default unit
    QString newUnit;
    if(axisY == QwtPlot::yLeft)
    {
        if(mCurrentUnitsLeft.contains(dataName))
            newUnit = mCurrentUnitsLeft.find(dataName).value();
    }
    else
    {
        if(mCurrentUnitsRight.contains(dataName))
            newUnit = mCurrentUnitsRight.find(dataName).value();
    }
    double scale = 1.0;
    scale = gConfig.getCustomUnits(dataName).find(newUnit).value();

    QVector<double> tempVectorY;
    for(size_t j=0; j<mVectorY[mCurrentGeneration].last().size(); ++j)
    {
        tempVectorY.append(mVectorY[mCurrentGeneration].last()[j]*scale);
    }
    tempCurve->setData(mVectorX[mCurrentGeneration].last(), tempVectorY);


    QString yLabel = QString(dataName + " [" + newUnit + "]");

    ++nCurves;
    if(nCurves > mCurveColors.size()-1)
    {
        nCurves = 0;        // This restarts the color loop when too many curves are added
    }

    mpVariablePlot->setAxisTitle(axisY, yLabel);
    mpVariablePlot->insertLegend(new QwtLegend(), QwtPlot::TopLegend);

    if(firstCurve)
    {
            //First curve, so set rubber band zoom base accordingly
        QwtDoubleRect tempDoubleRect;       //The first added curve is used as reference rectangle
        tempDoubleRect.setX(mpCurves.first()->minXValue());
        tempDoubleRect.setY(mpCurves.first()->minYValue());
        tempDoubleRect.setWidth(mpCurves.first()->maxXValue()-mpCurves.first()->minXValue());
        tempDoubleRect.setHeight(mpCurves.first()->maxYValue()-mpCurves.first()->minYValue());
        mpZoomer->setZoomBase(tempDoubleRect);
    }

    QStringList parameterDescription;
    parameterDescription << componentName << portName << dataName << dataUnit;
    mCurveParameters.append(parameterDescription);
}


//! @brief Function the x-axis in the plot, and updates all curves corresponding to this. Used to create XY-plots.
//! @param xArray is the data for the x-axis
//! @param componentName Name of the component where the parameter is located
//! @param portName Name of the port where the parameter is located
//! @param dataName Name of the parameter
//! @param dataUnit Unit of the parameter
void PlotWindow::changeXVector(QVector<double> xArray, QString componentName, QString portName, QString dataName, QString dataUnit)
{
    QString xLabel = QString(dataName + " [" + dataUnit + "]");
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
        mVectorX[mCurrentGeneration][i] = xArray;
    }
    mpVariablePlot->setAxisTitle(QwtPlot::xBottom, xLabel);
    mHasSpecialXAxis = true;
    mSpecialXParameter.clear();
    mSpecialXParameter << componentName << portName << dataName << dataUnit;
}


//! @brief Slot that updates the values for the curve, if the component/port still exist in the model
void PlotWindow::checkNewValues()
{
    if(!mAutoUpdate)       //Do not update curves to new values unless auto update is true
    {
        return;
    }

    QList< QVector<double> > tempList;
    mVectorX.append(tempList);
    mVectorY.append(tempList);

    for(int i=0; i<mpCurves.size(); ++i)
    {
        if(mpPlotParameterTree->mAvailableParameters.contains(mCurveParameters[i]))
        {
            QVector<double> xVector;
            if(mHasSpecialXAxis && mpPlotParameterTree->mAvailableParameters.contains(mSpecialXParameter))
            {
                gpMainWindow->mpProjectTabs->getCurrentSystem()->getCoreSystemAccessPtr()->getPlotData(mSpecialXParameter[0], mSpecialXParameter[1], mSpecialXParameter[2], xVector);
            }
            else
            {
                xVector = QVector<double>::fromStdVector(gpMainWindow->mpProjectTabs->getCurrentTab()->mpSystem->getCoreSystemAccessPtr()->getTimeVector(mCurveParameters[i][0], mCurveParameters[i][1]));
            }
            QVector<double> yVector;
            gpMainWindow->mpProjectTabs->getCurrentSystem()->getCoreSystemAccessPtr()->getPlotData(mCurveParameters[i][0], mCurveParameters[i][1], mCurveParameters[i][2], yVector);

            mVectorX.last().append(xVector);
            mVectorY.last().append(yVector);
        }
    }

    this->setGeneration(mVectorX.size()-1);

    mpDiscardGenerationButton->setEnabled(mVectorX.size() > 1);
}


//! @brief Reimplemented verseion of closeEvent function from QMainWindow. Used to warn the user if trying to close a plot window with old generations.
//! @todo The warning message won't appear if user discards all new generations and keeps only one old version. Could be difficult to solve though.
void PlotWindow::closeEvent(QCloseEvent *event)
{
    if(mVectorX.size() > 1)
    {
        QMessageBox msgBox(QMessageBox::Warning, tr("Warning"),
                           tr("Old plot data exist in plot window. Are you sure you want to close?"), 0, this);
        msgBox.addButton(tr("&Close"), QMessageBox::AcceptRole);
        msgBox.addButton(tr("&Cancel"), QMessageBox::RejectRole);
        if (msgBox.exec() == QMessageBox::RejectRole)
        {
            event->ignore();
        }
        else
        {
            event->accept();
            QMainWindow::close();
        }
    }
    else
    {
        event->accept();
        QMainWindow::close();
    }
}


//! Saves the plot window to a .hpw file. Saves the actual plot data to a corresponding .hmpf file.
void PlotWindow::saveToXml()
{
    QString hpwFileName = QFileDialog::getSaveFileName(
       this, "Export File Name", QString(),
       "Hopsan Plot Window file (*.hpw)");

    QString hmpfFileName = QString(hpwFileName.left(hpwFileName.size()-3) + "hmpf");

    if(!saveToHmpf(hmpfFileName))
    {
        return;
    }

        //Write to hopsanconfig.xml
    QDomDocument domDocument;
    QDomElement plotRoot = domDocument.createElement("hopsanplot");
    domDocument.appendChild(plotRoot);

    appendDomTextNode(plotRoot, "datafile", hmpfFileName);
    appendDomValueNode(plotRoot, "datasize", mpCurves.size());
    appendDomValueNode(plotRoot, "linewidth", mpCurves.first()->pen().width());
    appendDomTextNode(plotRoot, "backgroundcolor", mpVariablePlot->canvasBackground().name());
    appendDomBooleanNode(plotRoot, "grid", mpGrid->isVisible());

    QDomElement xDataElement = appendDomElement(plotRoot, "xdata");
    appendDomBooleanNode(xDataElement, "specialx", mHasSpecialXAxis);
    if(mHasSpecialXAxis)
    {
        appendDomTextNode(xDataElement, "component", mSpecialXParameter[0]);
        appendDomTextNode(xDataElement, "port", mSpecialXParameter[1]);
        appendDomTextNode(xDataElement, "dataname", mSpecialXParameter[2]);
        appendDomTextNode(xDataElement, "unit", mSpecialXParameter[3]);
    }

    for(size_t i=0; i<mCurveParameters.size(); ++i)
    {
        QDomElement curveElement = appendDomElement(plotRoot, "plotcurve");
        appendDomTextNode(curveElement, "component", mCurveParameters[i][0]);
        appendDomTextNode(curveElement, "port", mCurveParameters[i][1]);
        appendDomTextNode(curveElement, "dataname", mCurveParameters[i][2]);
        appendDomTextNode(curveElement, "unit", mCurveParameters[i][3]);
        appendDomValueNode(curveElement, "axis", mpCurves[i]->yAxis());
        appendDomValueNode(curveElement, "index", i);
        appendDomTextNode(curveElement, "linecolor", mpCurves[i]->pen().color().name());
    }

    appendRootXMLProcessingInstruction(domDocument);

    //Save to file
    const int IndentSize = 4;
    QFile xmlsettings(hpwFileName);
    if (!xmlsettings.open(QIODevice::WriteOnly | QIODevice::Text))  //open file
    {
        qDebug() << "Failed to open file for writing: " << hpwFileName;
        return;
    }
    QTextStream out(&xmlsettings);
    domDocument.save(out, IndentSize);
}


//! Saves the plot data for all curves and genereations to the specified .hmpf file
//! @param fileName Path and name of the target .hmpf file
bool PlotWindow::saveToHmpf(QString fileName)
{
    QFile file(fileName);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qDebug() << "Failed to open hpf file for writing: " + fileName;
        return false;
    }
    QTextStream hpfFile(&file);  //Create a QTextStream object to stream the content of file

    size_t nGenerations = mVectorX.size();
    size_t nCurves = mVectorX.back().size();

    for(size_t ig=0; ig<nGenerations; ++ig)
    {
        for(size_t id = 0; id<mVectorY.first().first().size(); ++id)
        {
            hpfFile << mVectorX[ig][0][id] << " ";
            for(size_t ic=0; ic<nCurves; ++ic)
            {
                hpfFile << mVectorY[ig][ic][id];
                if(ic!=nCurves-1)
                {
                    hpfFile << " ";
                }
                else
                {
                    hpfFile << "\n";
                }
            }
        }
        if(ig != nGenerations-1)
            hpfFile << "GENERATIONBREAK\n";
    }
    file.close();

    return true;
}
