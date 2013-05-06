#include "HVCWidget.h"
#include "Utilities/XMLUtilities.h"
#include "MainWindow.h"
#include "ProjectTabWidget.h"

#include "LogDataHandler.h"
#include "PlotHandler.h"
#include "GUIObjects/GUISystem.h"

#include <QCheckBox>
#include <QLabel>
#include <QFileDialog>
#include <QToolButton>

#include <QHBoxLayout>
#include <QGridLayout>
#include <QPushButton>


HVCWidget::HVCWidget(QWidget *parent) :
    QDialog(parent)
{
    mpHvcOpenPathEdit = new QLineEdit();
    QPushButton *pBrowseButton = new QPushButton();

    QHBoxLayout *pOpenFileHLayout = new QHBoxLayout();
    pOpenFileHLayout->addWidget(mpHvcOpenPathEdit);
    pOpenFileHLayout->addWidget(pBrowseButton);

    QCheckBox *pFoundModelCheckBox = new QCheckBox();
    QCheckBox *pFoundDataCheckBox = new QCheckBox();
    QHBoxLayout *pOpenStatusHLayout = new QHBoxLayout();
    pOpenStatusHLayout->addWidget(new QLabel("Found model: "));
    pOpenStatusHLayout->addWidget(pFoundModelCheckBox);
    pOpenStatusHLayout->addWidget(new QLabel("Found data: "));
    pOpenStatusHLayout->addWidget(pFoundDataCheckBox);


    mpAllVariablesTree = new FullNameVariableTreeWidget();
    mpAllVariablesTree->setDragDropMode(QAbstractItemView::DragOnly);
    mpAllVariablesTree->setDragEnabled(true);
    mpSelectedVariablesTree = new FullNameVariableTreeWidget();
    QHBoxLayout *pVariablesHLayout = new QHBoxLayout();

    pVariablesHLayout->addWidget(mpAllVariablesTree);
    pVariablesHLayout->addSpacing(10);
    pVariablesHLayout->addWidget(mpSelectedVariablesTree);

    QHBoxLayout *pButtonLayout = new QHBoxLayout();
    QPushButton *pSaveButton = new QPushButton("Save hvc");
    QPushButton *pCloseButton = new QPushButton("Close");
    QPushButton *pCompareButton = new QPushButton("Compare");
    pButtonLayout->addWidget(pCompareButton);
    pButtonLayout->addWidget(pSaveButton);
    pButtonLayout->addWidget(pCloseButton);

    QVBoxLayout *pMainLayout = new QVBoxLayout();
    pMainLayout->addLayout(pOpenFileHLayout);
    pMainLayout->addLayout(pOpenStatusHLayout);
    pMainLayout->addLayout(pVariablesHLayout);
    pMainLayout->addLayout(pButtonLayout);

    this->setLayout(pMainLayout);

    connect(pCloseButton, SIGNAL(clicked()), this, SLOT(close()));
    connect(pBrowseButton, SIGNAL(clicked()), this, SLOT(openHvcFile()));

    this->resize(800, 600);

}

void HVCWidget::openHvcFile()
{
    QFile hvcFile;
    hvcFile.setFileName(QFileDialog::getOpenFileName(this, "Select a HVC file"));
    if (hvcFile.exists())
    {
        // Clear
        clearContents();

        QFileInfo hvcFileInfo = QFileInfo(hvcFile);
        QDomDocument xmlDocument;
        QDomElement rootElement = loadXMLDomDocument(hvcFile, xmlDocument, "hopsanvalidationconfiguration");
        if (rootElement.isNull())
        {
            //! @todo some error message
            return;
        }

        QFile hmfFile, dataFile;
        if (rootElement.attribute("hvcversion") == "0.1")
        {
            // Load old format
            QDomElement xmlValidation = rootElement.firstChildElement("validation");
            QString modelFilePath = xmlValidation.firstChildElement("modelfile").text();
            if (modelFilePath.isEmpty())
            {
                // Use same name as hvc file instead
                modelFilePath = hvcFileInfo.absolutePath()+"/"+hvcFileInfo.baseName()+".hmf";
            }
            else
            {
                // Make sure absoulte, the loaded modelpath should be relative to the hvc file
                modelFilePath = hvcFileInfo.absolutePath()+"/"+modelFilePath;
            }
            hmfFile.setFileName(modelFilePath);

            if (hmfFile.exists())
            {
                mModelFilePath = modelFilePath;
                QDomElement xmlComponent = xmlValidation.firstChildElement("component");
                while (!xmlComponent.isNull())
                {
                    QDomElement xmlPort = xmlComponent.firstChildElement("port");
                    while (!xmlPort.isNull())
                    {
                        QString dataFilePath = xmlComponent.firstChildElement("csvfile").text();
                        if (dataFilePath.isEmpty())
                        {
                            // Use same name as hvc file instead
                            dataFilePath = hvcFileInfo.absolutePath()+hvcFileInfo.baseName()+".csv";
                        }
                        dataFile.setFileName(dataFilePath);

                        QDomElement xmlVariable = xmlPort.firstChildElement("variable");
                        while (!xmlVariable.isNull())
                        {
                            // check if we should override data file
                            if (!xmlComponent.firstChildElement("csvfile").isNull())
                            {
                                QString dataFilePath = xmlComponent.firstChildElement("csvfile").text();
                                if (dataFilePath.isEmpty())
                                {
                                    // Use same name as hvc file instead
                                    dataFilePath = hvcFileInfo.absolutePath()+hvcFileInfo.baseName()+".csv";
                                }
                                dataFile.setFileName(dataFilePath);
                            }

                            HvcConfig conf;
                            conf.mDataColumn = parseDomIntegerNode(xmlVariable.firstChildElement("column"));
                            conf.mTolerance = parseDomValueNode(xmlVariable.firstChildElement("tolerance"));
                            conf.mFullVarName = xmlComponent.attribute("name")+"#"+xmlPort.attribute("name")+"#"+xmlVariable.attribute("name");
                            conf.mDataFile = dataFile.fileName();
                            mDataConfigs.append(conf);

                            // Populate the tree
                            mpAllVariablesTree->addFullNameVariable(conf.mFullVarName);

                            // Next variable to check
                            xmlVariable = xmlVariable.nextSiblingElement("variable");
                        }
                        xmlPort = xmlPort.nextSiblingElement("port");
                    }
                    xmlComponent = xmlComponent.nextSiblingElement("component");
                }
            }
            else
            {
                //! @todo some error
            }
        }
        else
        {
            // Load new format
            //! @todo

        }


        // Show file in line edit
        mpHvcOpenPathEdit->setText(hvcFile.fileName());

        // Find model and any data files, update status

        // Populate tree

    }
}

void HVCWidget::clearContents()
{
    mpHvcOpenPathEdit->clear();
    mpAllVariablesTree->clear();
    mpSelectedVariablesTree->clear();
    mModelFilePath.clear();
}

void HVCWidget::runHvcTest()
{
    // Run each test
    for (int t=0; t<mDataConfigs.size(); ++t)
    {
        // First load the model
        gpMainWindow->mpProjectTabs->loadModel(mModelFilePath, true);
        // Switch to that tab

        // Simulate the system
        gpMainWindow->mpProjectTabs->getCurrentTab()->simulate_blocking();

        LogDataHandler *pLogDataHandler = gpMainWindow->mpProjectTabs->getCurrentTopLevelSystem()->getLogDataHandler();

        QVector<int> columns;
        QStringList names;
        columns.append(mDataConfigs[t].mDataColumn);
        names.append(mDataConfigs[t].mFullVarName+"_valid");

        pLogDataHandler->importTimeVariablesFromCSVColumns(mDataConfigs[t].mDataFile, columns, names, 0);

        QString windowName = QString("Validation Plot %1").arg(t);
        gpPlotHandler->createPlotWindow(windowName);
        gpPlotHandler->plotDataToWindow(windowName, pLogDataHandler->getPlotData(mDataConfigs[t].mFullVarName,-1), 0);
        gpPlotHandler->plotDataToWindow(windowName, pLogDataHandler->getPlotData(mDataConfigs[t].mFullVarName+"_valid",-1), 0);
    }
}


void FullNameVariableTreeWidget::addFullNameVariable(const QString &rFullName, const QString &rRemaningName, QTreeWidgetItem *pParentItem)
{
    if (!rFullName.isEmpty())
    {
        bool isLeaf=false;
        QStringList systems = rRemaningName.split("$");
        QString name;

        // If we have a system part
        if (systems.size() > 1)
        {
            name = systems.first();
        }
        // Else we have a comp::port::var
        else
        {
            QStringList compportvar = rRemaningName.split("#");
            if (compportvar.size()==1)
            {
                isLeaf = true;
            }
            name = compportvar.first();
        }


        QTreeWidgetItem* pLevelItem=0;
        // Add a new level in the tree
        if (pParentItem)
        {
            QTreeWidgetItem* pFound=0;
            for (int c=0; c<pParentItem->childCount(); ++c)
            {
                if (pParentItem->child(c)->text(0) == name)
                {
                    // We assume there will never be duplicates due to uniqe name requirements
                    pFound = pParentItem->child(c);
                    break;
                }
            }

            if (pFound)
            {
                pLevelItem = pFound;
            }
            else
            {
                // Add new item
                pLevelItem = new QTreeWidgetItem(QStringList(name));
                pParentItem->addChild(pLevelItem);
            }
        }
        else
        {
            QList<QTreeWidgetItem*> found = this->findItems(name, 0);
            if (found.isEmpty())
            {
                // Add new top level item
                pLevelItem = new QTreeWidgetItem(QStringList(name));
                addTopLevelItem(pLevelItem);
            }
            else
            {
                // We assume there will never be duplicates due to uniqe name requirements
                pLevelItem = found.first();
            }

        }

        if (isLeaf)
        {
            pLevelItem->setData(0,Qt::UserRole,rFullName);
        }
        else
        {
            // Recurse
            QString shorterRemaningName = rRemaningName;
            shorterRemaningName.remove(0, name.size()+1);
            addFullNameVariable(rFullName, shorterRemaningName, pLevelItem);
        }
    }
}


FullNameVariableTreeWidget::FullNameVariableTreeWidget(QWidget *pParent) : QTreeWidget(pParent)
{
    //Nothing
}

void FullNameVariableTreeWidget::addFullNameVariable(const QString &rFullName)
{
    addFullNameVariable(rFullName, rFullName, 0);
}
