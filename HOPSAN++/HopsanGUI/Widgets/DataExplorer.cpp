#include "global.h"
#include "DataExplorer.h"
#include "LogDataHandler.h"
#include "Configuration.h"

#include <QLabel>
#include <QLayout>
#include <QCheckBox>
#include <QPushButton>
#include <QFileDialog>
#include <QButtonGroup>
#include <QRadioButton>
#include <QGroupBox>
#include <QProgressDialog>


class GenerationItem : public QWidget
{
public:
    GenerationItem(const int genNum, const QString &rDescription, QWidget *pParent) : QWidget(pParent)
    {
        mGenerationNumber = genNum;
        QHBoxLayout *pHLayout = new QHBoxLayout(this);
        pHLayout->addWidget(&mChosenCheckBox);
        pHLayout->addWidget(new QLabel(QString("Gen: %1  %2").arg(genNum).arg(rDescription), this));
        pHLayout->setSizeConstraint(QLayout::SetFixedSize);
    }

    int getGeneration() const
    {
        return mGenerationNumber;
    }

    QCheckBox mChosenCheckBox;

private:
    int mGenerationNumber;

};

DataExplorer::DataExplorer(QWidget *parent) :
    QDialog(parent)
{
    mpLogDataHandler = 0;
    mAllSelectedToggle = false;
    QGridLayout *pMainLayout = new QGridLayout();

    // Create generations buttons
    QWidget *pGenerationsButtonWidget = new QWidget(this);
    QGridLayout *pGenerationButtonsLayout = new QGridLayout(pGenerationsButtonWidget);
    QPushButton *pImportGenerationButton = new QPushButton("Import", pGenerationsButtonWidget);
    QPushButton *pExportGenerationsButton = new QPushButton("Export", pGenerationsButtonWidget);
    QPushButton *pDeleteGenerationsButton = new QPushButton("Remove", pGenerationsButtonWidget);
    QPushButton *pSelectAllGensButton = new QPushButton("(Un)Select all", pGenerationsButtonWidget);
    pGenerationButtonsLayout->addWidget(pImportGenerationButton,0,0);
    pGenerationButtonsLayout->addWidget(pExportGenerationsButton,1,0);
    pGenerationButtonsLayout->addWidget(pDeleteGenerationsButton,1,1);
    pGenerationButtonsLayout->addWidget(pSelectAllGensButton,2,0);
    pGenerationButtonsLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);

    connect(pExportGenerationsButton, SIGNAL(clicked()), this, SLOT(openExportDataDialog()));
    connect(pImportGenerationButton, SIGNAL(clicked()), this, SLOT(openImportDataDialog()));
    connect(pDeleteGenerationsButton, SIGNAL(clicked()), this, SLOT(removeSelectedGenerations()));
    connect(pSelectAllGensButton, SIGNAL(clicked()), this, SLOT(toggleSelectAllGenerations()));


    pMainLayout->addWidget(pGenerationsButtonWidget,0,0);

    mpGenerationsScrollArea = new QScrollArea(this);
    pMainLayout->addWidget(mpGenerationsScrollArea,1,0);
    mpGenerationsListWidget = 0;
    refreshGenerationList();


    this->setLayout(pMainLayout);

    this->resize(800, 600);
    this->setWindowTitle("Data Explorer");
}

void DataExplorer::setLogdataHandler(LogDataHandler *pLogDataHanlder)
{
    if (mpLogDataHandler)
    {
        disconnect(mpLogDataHandler, 0, this, 0);
    }

    mpLogDataHandler = pLogDataHanlder;
    connect(mpLogDataHandler, SIGNAL(newDataAvailable()), this, SLOT(refreshGenerationList()));
    connect(mpLogDataHandler, SIGNAL(dataRemoved()), this, SLOT(refreshGenerationList()));
    refreshGenerationList();

}

void DataExplorer::refresh()
{

}

void DataExplorer::openImportDataDialog()
{
    QString fileName = QFileDialog::getOpenFileName(this,tr("Choose Hopsan Data File"),
                                                    gpConfig->getPlotDataDir(),
                                                    tr("Data Files (*.plo *.PLO *.csv *.CSV)"));
    QFileInfo fi(fileName);
    if (mpLogDataHandler)
    {
        if (fi.suffix().toLower() == "plo")
        {
            mpLogDataHandler->importFromPlo(fileName);
        }
        else if (fi.suffix().toLower() == "csv")
        {
            mpLogDataHandler->importFromCSV_AutoFormat(fileName);
        }
    }
}

void DataExplorer::openExportDataDialog()
{
    QDialog exportOptions;

    QButtonGroup *pFormatButtons = new QButtonGroup(&exportOptions);
    QRadioButton *pPLOv1Button = new QRadioButton("PLO v1");
    QRadioButton *pPLOv2Button = new QRadioButton("PLO v2");
    QRadioButton *pCSVButton = new QRadioButton("csv");
    pFormatButtons->addButton(pPLOv1Button);
    pFormatButtons->addButton(pPLOv2Button);
    pFormatButtons->addButton(pCSVButton);
    pPLOv1Button->setChecked(true);

    QGroupBox *pFormatGroupBox = new QGroupBox("Choose Export Format:", &exportOptions);
    QHBoxLayout *pFormatButtonLayout = new QHBoxLayout();
    pFormatButtonLayout->addWidget(pPLOv1Button);
    pFormatButtonLayout->addWidget(pPLOv2Button);
    pFormatButtonLayout->addWidget(pCSVButton);
    pFormatGroupBox->setLayout(pFormatButtonLayout);


    QButtonGroup *pFilenameButtons = new QButtonGroup(&exportOptions);
    QRadioButton *pAppendGenButton = new QRadioButton("Append _gen to each selected generation");
    QRadioButton *pAskEveryGenButton = new QRadioButton("Ask for filename for each selected generation");
    pFilenameButtons->addButton(pAppendGenButton);
    pFilenameButtons->addButton(pAskEveryGenButton);
    pAppendGenButton->setChecked(true);

    QGroupBox *pFilenameGroupBox = new QGroupBox("Choose Multi-Generation Filename Option:", &exportOptions);
    QVBoxLayout *pFilenameButtonLayout = new QVBoxLayout();
    pFilenameButtonLayout->addWidget(pAppendGenButton);
    pFilenameButtonLayout->addWidget(pAskEveryGenButton);
    pFilenameGroupBox->setLayout(pFilenameButtonLayout);


    QGridLayout *pMainLayout = new QGridLayout(&exportOptions);
    pMainLayout->addWidget(pFormatGroupBox,0,0);
    pMainLayout->addWidget(pFilenameGroupBox,1,0);

    QPushButton *pExportButton = new QPushButton("Export", &exportOptions);
    QPushButton *pCancelButton = new QPushButton("Cancel", &exportOptions);

    pMainLayout->addWidget(pExportButton,2,1);
    pMainLayout->addWidget(pCancelButton,2,2);

    connect(pCancelButton, SIGNAL(clicked()), &exportOptions, SLOT(reject()));
    connect(pExportButton, SIGNAL(clicked()), &exportOptions, SLOT(accept()));

    if (exportOptions.exec() == QDialog::Accepted)
    {
        QVector<int> gens = gensFromSelected();
        QStringList fNames;
        QString suffixFilter;

        // Select suffix filter for file format
        if ( (pFormatButtons->checkedButton() == pPLOv1Button) || (pFormatButtons->checkedButton() == pPLOv2Button) )
        {
            suffixFilter = "*.plo *.PLO";
        }
        else
        {
            suffixFilter = "*.csv *.CSV";
        }

        // Get save file name
        if ((pFilenameButtons->checkedButton() == pAppendGenButton) && (gens.size() > 1))
        {
            QString fileName = QFileDialog::getSaveFileName(this,tr("Choose Hopsan Data File Name"), gpConfig->getPlotDataDir(), tr("Data Files")+QString(" (%1)").arg(suffixFilter));
            QFileInfo file(fileName);
            for (int i=0; i<gens.size(); ++i)
            {
                fNames.append(file.absolutePath()+"/"+file.baseName()+QString("_%1").arg(gens[i])+"."+file.suffix());
            }
        }
        else
        {
            for (int i=0; i<gens.size(); ++i)
            {
                fNames.append(QFileDialog::getSaveFileName(this,QString("Choose File Name for Gen: %1").arg(gens[i]), gpConfig->getPlotDataDir(),  tr("Data Files")+QString(" (%1)").arg(suffixFilter)));
            }
        }

        // Remember this output dir for later use
        if (!fNames.isEmpty())
        {
            QFileInfo exportPath(fNames.last());
            gpConfig->setPlotDataDir(exportPath.absolutePath());
        }

        QProgressDialog progress("Exporting Data", "Cancel", 0, fNames.size(), this);
        progress.setWindowModality(Qt::WindowModal);
        progress.show();
        for (int i=0; i<fNames.size(); ++i)
        {
            // Abort if canceld
            if (progress.wasCanceled())
                break;

            QString &file = fNames[i];
            if (!file.isEmpty())
            {
                const int g = gens[i];
                if (pFormatButtons->checkedButton() == pPLOv1Button)
                {
                    mpLogDataHandler->exportGenerationToPlo(file, g, 1);
                }
                else if (pFormatButtons->checkedButton() == pPLOv2Button)
                {
                    mpLogDataHandler->exportGenerationToPlo(file, g, 2);
                }
                else
                {
                    mpLogDataHandler->exportGenerationToCSV(file, g);
                }
            }
            progress.setValue(i+1);
        }
    }
}

void DataExplorer::refreshGenerationList()
{
    if (mpGenerationsListWidget)
    {
        mGenerationItemMap.clear();
        mpGenerationsListWidget->deleteLater();
    }

    if (mpLogDataHandler)
    {
        mpGenerationsListWidget = new QWidget(this);
        QVBoxLayout *pGenerationListLayout = new QVBoxLayout(mpGenerationsListWidget);
        pGenerationListLayout->addWidget(new QLabel("Generations", mpGenerationsListWidget),0,Qt::AlignHCenter);

        QList<int> gens = mpLogDataHandler->getGenerations();
        QMap<QString, int> importedGensFileMap = mpLogDataHandler->getImportFilesAndGenerations();
        QList<int> importedGens = importedGensFileMap.values();
        QList<int>::iterator it;
        for (it=gens.begin(); it!=gens.end(); ++it)
        {
            GenerationItem *pItem;
            if (importedGens.contains(*it))
            {
                pItem = new GenerationItem(*it, importedGensFileMap.key(*it), mpGenerationsListWidget);
            }
            else
            {
                pItem = new GenerationItem(*it, "", mpGenerationsListWidget);
            }
            pGenerationListLayout->addWidget(pItem, 1, Qt::AlignTop);
            mGenerationItemMap.insert(*it,pItem);
        }
        pGenerationListLayout->addStretch(2); // Pushes all items to top

        mpGenerationsListWidget->show();
        mpGenerationsScrollArea->setWidget(mpGenerationsListWidget);
    }
}

void DataExplorer::refreshDataList()
{

}

void DataExplorer::removeSelectedGenerations()
{
    QVector<int> gens = gensFromSelected();
    QProgressDialog progress("Removing generations", "Cancel", 0, gens.size(), this);
    progress.setWindowModality(Qt::WindowModal);
    progress.show();
    // Since removeGeneration will also remove from mGenerationItemMap, we cant remove directly in for loop above
    for (int i=0; i<gens.size(); ++i)
    {
        // Abort if canceld
        if (progress.wasCanceled())
            break;

        removeGeneration(gens[i]);
        progress.setValue(i+1);
    }
}

void DataExplorer::removeGeneration(int gen)
{
    if (mpLogDataHandler)
    {
        // Force removal of data generation
        mpLogDataHandler->removeGeneration(gen, true);
        // We assume that the generation was removed, now delete it from the view (quicker than reloading the view)
        GenerationItem *pItem = mGenerationItemMap.value(gen);
        if (pItem)
        {
            pItem->deleteLater();
            mGenerationItemMap.remove(gen);
        }
    }
}

void DataExplorer::toggleSelectAllGenerations()
{
    mAllSelectedToggle = !mAllSelectedToggle;
    QMap<int,GenerationItem*>::iterator it;
    for (it = mGenerationItemMap.begin(); it != mGenerationItemMap.end(); ++it)
    {
        it.value()->mChosenCheckBox.setChecked(mAllSelectedToggle);
    }
}

QVector<int> DataExplorer::gensFromSelected()
{
    QVector<int> gens;
    QMap<int,GenerationItem*>::iterator it;
    for (it = mGenerationItemMap.begin(); it != mGenerationItemMap.end(); ++it)
    {
        if (it.value()->mChosenCheckBox.isChecked())
        {
            gens.append(it.key());
        }
    }
    return gens;
}
