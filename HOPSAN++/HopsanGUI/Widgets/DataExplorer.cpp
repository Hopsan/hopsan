#include "DataExplorer.h"
#include "LogDataHandler.h"
#include "Configuration.h"

#include <QLabel>
#include <QLayout>
#include <QCheckBox>
#include <QPushButton>
#include <QFileDialog>


class GenerationItem : public QWidget
{
public:
    GenerationItem(const int genNum, QWidget *pParent) : QWidget(pParent)
    {
        mGenerationNumber = genNum;
        QHBoxLayout *pHLayout = new QHBoxLayout(this);
        pHLayout->addWidget(&mChosenCheckBox);
        pHLayout->addWidget(new QLabel(QString("Gen: %1").arg(genNum), this));
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
    QGridLayout *pMainLayout = new QGridLayout();

    // Create generations buttons
    QWidget *pGenerationsButtonWidget = new QWidget(this);
    QGridLayout *pGenerationButtonsLayout = new QGridLayout(pGenerationsButtonWidget);
    QPushButton *pImportGenerationButton = new QPushButton("Import", pGenerationsButtonWidget);
    QPushButton *pExportGenerationsButton = new QPushButton("Export", pGenerationsButtonWidget);
    pExportGenerationsButton->setEnabled(false);
    QPushButton *pDeleteGenerationsButton = new QPushButton("Remove", pGenerationsButtonWidget);
    pGenerationButtonsLayout->addWidget(pImportGenerationButton,0,0);
    pGenerationButtonsLayout->addWidget(pExportGenerationsButton,1,0);
    pGenerationButtonsLayout->addWidget(pDeleteGenerationsButton,1,1);
    pGenerationButtonsLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);

    connect(pImportGenerationButton, SIGNAL(clicked()), this, SLOT(openImportDataDialog()));
    connect(pDeleteGenerationsButton, SIGNAL(clicked()), this, SLOT(removeSelectedGenerations()));


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
    refreshGenerationList();

}

void DataExplorer::refresh()
{

}

void DataExplorer::openImportDataDialog()
{
    QString fileName = QFileDialog::getOpenFileName(this,tr("Choose Hopsan Data File"),
                                                    gConfig.getPlotDataDir(),
                                                    tr("Data Files (*.plo *.PLO *.csv)"));
    QFileInfo fi(fileName);
    if (mpLogDataHandler)
    {
        if (fi.suffix().toLower() == "plo")
        {
            mpLogDataHandler->importFromPlo(fileName);
        }
        else if (fi.suffix().toLower() == "csv")
        {
            mpLogDataHandler->importFromCsv(fileName);
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
        QList<int>::iterator it;
        for (it=gens.begin(); it!=gens.end(); ++it)
        {
            GenerationItem *pItem = new GenerationItem(*it, mpGenerationsListWidget);
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
    QVector<int> gens;
    QMap<int,GenerationItem*>::iterator it;
    for (it = mGenerationItemMap.begin(); it != mGenerationItemMap.end(); ++it)
    {
        if (it.value()->mChosenCheckBox.isChecked())
        {
            gens.append(it.key());
        }
    }

    // Since removeGeneration will also remove from mGenerationItemMap, we cant remove directly in for loop above
    for (int i=0; i<gens.size(); ++i)
    {
        removeGeneration(gens[i]);
    }
}

void DataExplorer::removeGeneration(int gen)
{
    if (mpLogDataHandler)
    {
        mpLogDataHandler->removeGeneration(gen);
        // We assume that the generation was removed, now delete it from the view (quicker the reloading the view)
        GenerationItem *pItem = mGenerationItemMap.value(gen);
        if (pItem)
        {
            pItem->deleteLater();
            mGenerationItemMap.remove(gen);
        }
    }
}
