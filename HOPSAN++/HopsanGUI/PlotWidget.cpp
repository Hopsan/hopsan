//$Id$

#include <QDebug>
#include <QSpinBox>
#include <QColorDialog>
#include <QLabel>
#include <QCursor>
#include <QAction>
#include <QTextStream>

#include "PlotWindow.h"
#include "PlotWidget.h"
#include "MainWindow.h"
#include "ProjectTabWidget.h"
#include "GUIObjects/GUIModelObject.h"
#include "GUIPort.h"
#include "GraphicsView.h"
#include "GUIObjects/GUISystem.h"
#include "Utilities/GUIUtilities.h"
#include "loadObjects.h"
#include "MessageWidget.h"


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
    //mpParentMainWindow = parent;
    mpCurrentSystem = gpMainWindow->mpProjectTabs->getCurrentSystem();
    mFavoriteParameters.clear();

    this->setDragEnabled(true);
    this->setAcceptDrops(false);
    this->updateList();
    this->setHeaderHidden(true);
    this->setColumnCount(1);

    connect(gpMainWindow->mpProjectTabs, SIGNAL(currentChanged(int)), this, SLOT(updateList()));
    connect(gpMainWindow->mpProjectTabs, SIGNAL(tabCloseRequested(int)), this, SLOT(updateList()));
    connect(gpMainWindow->mpProjectTabs, SIGNAL(newTabAdded()), this, SLOT(updateList()));
    connect(gpMainWindow->mpProjectTabs->getCurrentSystem(), SIGNAL(componentChanged()), this, SLOT(updateList()));
    connect(gpMainWindow->mpProjectTabs->getCurrentTab(), SIGNAL(simulationFinished()), this, SLOT(updateList()));
    connect(this,SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)),this,SLOT(createPlotWindow(QTreeWidgetItem*)));
}


//! @brief Updates the parameter tree to the available components and parameters in the current tab.
void PlotParameterTree::updateList()
{
    mAvailableParameters.clear();
    this->clear();

    if(gpMainWindow->mpProjectTabs->count() == 0)     //Check so that at least one project tab exists
    {
        return;
    }

    mpCurrentSystem = gpMainWindow->mpProjectTabs->getCurrentTab()->mpSystem;
    QTreeWidgetItem *tempComponentItem;     //Tree item for components
    PlotParameterItem *tempPlotParameterItem;       //Tree item for parameters - reimplemented so they can store information about the parameter

    GUISystem::GUIModelObjectMapT::iterator it;
    for(it = mpCurrentSystem->mGUIModelObjectMap.begin(); it!=mpCurrentSystem->mGUIModelObjectMap.end(); ++it)
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
            gpMainWindow->mpProjectTabs->getCurrentTab()->mpSystem->getCoreSystemAccessPtr()->getPlotDataNamesAndUnits((*itp)->getGuiModelObjectName(), (*itp)->getName(), parameterNames, parameterUnits);

            QVector<double> time = QVector<double>::fromStdVector(gpMainWindow->mpProjectTabs->getCurrentTab()->mpSystem->getCoreSystemAccessPtr()->getTimeVector((*itp)->getGuiModelObjectName(), (*itp)->getName()));

            if(time.size() > 0)     //If time vector is greater than zero we have something to plot!
            {
                for(int i = 0; i!=parameterNames.size(); ++i)
                {
                    tempPlotParameterItem = new PlotParameterItem(it.value()->getName(), (*itp)->getName(), parameterNames[i], parameterUnits[i], tempComponentItem);
                    tempComponentItem->addChild(tempPlotParameterItem);
                    QStringList parameterDescription;
                    parameterDescription << (*itp)->getGuiModelObjectName() << (*itp)->getName() << parameterNames[i] << parameterUnits[i];
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
    disconnect(gpMainWindow->mpProjectTabs->getCurrentTab(), SIGNAL(simulationFinished()), this, SLOT(updateList()));
    connect(gpMainWindow->mpProjectTabs->getCurrentTab(), SIGNAL(simulationFinished()), this, SLOT(updateList()));
}


//! @brief Helper function that creates a new plot window by using a QTreeWidgetItem in the plot variable tree.
//! @param *item Pointer to the tree widget item whos arrays will be looked up from the map and plotted
PlotWindow *PlotParameterTree::createPlotWindow(QTreeWidgetItem *item)
{
    //! @todo This is a kind of dumb check; it assumes that component items have bold font and variables not.
    if(!item->font(0).bold())     //Top level items cannot be plotted (they represent the components)
    {
        //QTreeWidgetItem must be casted to a PlotParameterItem. This is a necessary because double click event can not know which kind of tree item is clicked.
        PlotParameterItem *tempItem = dynamic_cast<PlotParameterItem *>(item);
        return createPlotWindow(tempItem->getComponentName(), tempItem->getPortName(), tempItem->getDataName(), tempItem->getDataUnit());
    }
}


//! @brief Creates a new plot window from specified component and parameter.
//! @param componentName Name of the component where the port with the parameter is located
//! @param portName Name of the port where the parameter is located
//! @param dataName Name of the parameter
//! @param dataUnit Name of the unit of the parameter
PlotWindow *PlotParameterTree::createPlotWindow(QString componentName, QString portName, QString dataName, QString dataUnit)
{
    QVector<double> xVector = QVector<double>::fromStdVector(gpMainWindow->mpProjectTabs->getCurrentTab()->mpSystem->getCoreSystemAccessPtr()->getTimeVector(componentName, portName));
    QVector<double> yVector;
    gpMainWindow->mpProjectTabs->getCurrentTab()->mpSystem->getCoreSystemAccessPtr()->getPlotData(componentName, portName, dataName, yVector);

    if((xVector.isEmpty()) || (yVector.isEmpty()))
        return 0;

    PlotWindow *plotWindow = new PlotWindow(this, gpMainWindow);
    plotWindow->show();
    plotWindow->addPlotCurve(xVector, yVector, componentName, portName, dataName, dataUnit, QwtPlot::yLeft);

    return plotWindow;
}


PlotWindow *PlotParameterTree::createPlotWindow(QVector<double> xVector, QVector<double> yVector, int axis, QString componentName, QString portName, QString dataName, QString dataUnit)
{
    PlotWindow *plotWindow = new PlotWindow(this, gpMainWindow);
    plotWindow->show();
    plotWindow->addPlotCurve(xVector, yVector, componentName, portName, dataName, dataUnit, axis);

    return plotWindow;
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
    //mpParentMainWindow = parent;

    mpPlotParameterTree = new PlotParameterTree(gpMainWindow);

    mpLoadButton = new QPushButton(tr("&Load Plot Window from XML"));
    mpLoadButton->setAutoDefault(false);
    mpLoadButton->setFixedHeight(30);
    QFont tempFont = mpLoadButton->font();
    tempFont.setBold(true);
    mpLoadButton->setFont(tempFont);

    mpLayout = new QGridLayout(this);
    mpLayout->addWidget(mpPlotParameterTree,0,0,1,1);
    mpLayout->addWidget(mpLoadButton, 1, 0, 1, 1);

    connect(mpLoadButton, SIGNAL(clicked()),this,SLOT(loadFromXml()));
}


//! Loads a plot window from a specified .hpw file. Loads the actual plot data from a .hmpf file.
void PlotWidget::loadFromXml()
{
    QDir fileDialogSaveDir;
    QString hpwFilePath;
    hpwFilePath = QFileDialog::getOpenFileName(this, tr("Plot Window File"),
                                               fileDialogSaveDir.currentPath() + QString(MODELPATH),
                                               tr("Hopsan Plot Window files (*.hpw)"));

    QFile file(hpwFilePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        gpMainWindow->mpMessageWidget->printGUIErrorMessage("Unable to read plot window file.");
        return;
    }
    QDomDocument domDocument;
    QString errorStr;
    int errorLine, errorColumn;
    if (!domDocument.setContent(&file, false, &errorStr, &errorLine, &errorColumn))
    {
        QMessageBox::information(window(), tr("Hopsan GUI"),
                                 tr("Parse error at line %1, column %2:\n%3")
                                 .arg(errorLine)
                                 .arg(errorColumn)
                                 .arg(errorStr));
    }
    else
    {
        QDomElement hpwRoot = domDocument.documentElement();
        if (hpwRoot.tagName() != "hopsanplot")
        {
            QMessageBox::information(window(), tr("Hopsan GUI"),
                                     "The file is not an Hopsan Plot Window file. Incorrect hpw root tag name: "
                                     + hpwRoot.tagName() + " != hopsanplot");
        }
        else
        {
            QString hmpfFileName = hpwRoot.firstChildElement("datafile").text();
            size_t datasize = parseDomValueNode(hpwRoot.firstChildElement("datasize"));

            QFile hmpfFile(hmpfFileName);
            if(!hmpfFile.exists())
            {
                qDebug() << "Failed to open file, file not found: " + hmpfFile.fileName();
                return;
            }
            if (!hmpfFile.open(QIODevice::ReadOnly | QIODevice::Text))
            {
                return;
            }

            QList< QVector<double> > xData;
            QList< QList < QVector<double> > > yData;
            QVector<double> tempVector;
            xData.append(tempVector);
            QList< QVector<double> > tempList;
            for(size_t i=0; i<datasize; ++i)
            {
                tempList.append(tempVector);
            }
            yData.append(tempList);


            QTextStream fileStream(&hmpfFile);
            QString line;
            QTextStream lineStream;
            size_t generation = 0;
            double value;
            while( !fileStream.atEnd() )
            {
                line = fileStream.readLine();
                if(line.startsWith("GENERATIONBREAK"))
                {
                    ++generation;
                    xData.append(tempVector);
                    yData.append(tempList);
                }
                else
                {
                    lineStream.setString(&line);
                    lineStream >> value;
                    xData[generation].append(value);
                    for(size_t ic=0; ic<datasize; ++ic)
                    {
                        lineStream >> value;
                        yData[generation][ic].append(value);
                    }
                }
            }
            hmpfFile.close();

            QStringList componentName;
            QStringList portName;
            QStringList dataName;
            QStringList dataUnit;
            QList<size_t> axis;
            QList<size_t> index;

                //Create plot window and curves from loaded data
            QDomElement curveElement = hpwRoot.firstChildElement("plotcurve");
            componentName.append(curveElement.firstChildElement("component").text());
            portName.append(curveElement.firstChildElement("port").text());
            dataName.append(curveElement.firstChildElement("dataname").text());
            dataUnit.append(curveElement.firstChildElement("unit").text());
            axis.append(parseDomValueNode(curveElement.firstChildElement("axis")));
            index.append(parseDomValueNode(curveElement.firstChildElement("index")));

                //Create the actual plot window (with first curve, first generation)
            PlotWindow *pPlotWindow = mpPlotParameterTree->createPlotWindow(xData[0], yData[0][index.first()], axis.first(), componentName.first(), portName.first(), dataName.first(), dataUnit.first());

            pPlotWindow->mpCurves.first()->setPen(QPen(QColor(curveElement.firstChildElement("linecolor").text()),
                                                  pPlotWindow->mpCurves.first()->pen().width()));

                //Add the remaining curves (first generation)
            curveElement = curveElement.nextSiblingElement("plotcurve");
            while(!curveElement.isNull())
            {
                componentName.append(curveElement.firstChildElement("component").text());
                portName.append(curveElement.firstChildElement("port").text());
                dataName.append(curveElement.firstChildElement("dataname").text());
                dataUnit.append(curveElement.firstChildElement("unit").text());
                axis.append(parseDomValueNode(curveElement.firstChildElement("axis")));
                index.append(parseDomValueNode(curveElement.firstChildElement("index")));
                pPlotWindow->addPlotCurve(xData[0], yData[0][index.last()], componentName.last(), portName.last(), dataName.last(), dataUnit.last(), axis.last());

                pPlotWindow->mpCurves.last()->setPen(QPen(QColor(curveElement.firstChildElement("linecolor").text()),
                                                     pPlotWindow->mpCurves.last()->pen().width()));

                curveElement = curveElement.nextSiblingElement("plotcurve");
            }

                //Add the remaining generations
            QList< QVector<double> > tempList2;
            for(size_t ig=1; ig<xData.size(); ++ig)
            {
                pPlotWindow->mVectorX.append(tempList2);
                pPlotWindow->mVectorY.append(tempList2);
                for(size_t ic=0; ic<index.size(); ++ic)
                {
                    pPlotWindow->mVectorX.last().append(xData[ig]);
                    pPlotWindow->mVectorY.last().append(yData[ig][index[ic]]);
                }
            }

                //Set current generation and enable discard button if more than one generation
            pPlotWindow->mpDiscardGenerationButton->setEnabled(xData.size() > 1);
            pPlotWindow->setGeneration(xData.size()-1);


                //Keep loading xml data
            pPlotWindow->setLineWidth(parseDomValueNode(hpwRoot.firstChildElement("linewidth")));
            pPlotWindow->mpVariablePlot->setCanvasBackground(hpwRoot.firstChildElement("backgroundcolor").text());
            pPlotWindow->enableGrid(parseDomBooleanNode(hpwRoot.firstChildElement("grid")));
            pPlotWindow->mpVariablePlot->replot();
        }
    }

    file.close();
}



