/*-----------------------------------------------------------------------------
 This source file is a part of Hopsan

 Copyright (c) 2009 to present year, Hopsan Group

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 For license details and information about the Hopsan Group see the files
 GPLv3 and HOPSANGROUP in the Hopsan source code root directory

 For author and contributor information see the AUTHORS file
-----------------------------------------------------------------------------*/

#include "global.h"
#include "DataExplorer.h"
#include "LogDataHandler2.h"
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
        pHLayout->addWidget(new QLabel(QString("Gen: %1  %2").arg(genNum+1).arg(rDescription), this));
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

void DataExplorer::setLogdataHandler(LogDataHandler2 *pLogDataHandler)
{
    if (mpLogDataHandler)
    {
        disconnect(mpLogDataHandler, 0, this, 0);
    }

    mpLogDataHandler = pLogDataHandler;
    if (mpLogDataHandler)
    {
        connect(mpLogDataHandler, SIGNAL(dataAdded()), this, SLOT(refreshGenerationList()));
        connect(mpLogDataHandler, SIGNAL(dataRemoved()), this, SLOT(refreshGenerationList()));
    }
    refreshGenerationList();

}

void DataExplorer::refresh()
{

}

void DataExplorer::openImportDataDialog()
{
    QString fileName = QFileDialog::getOpenFileName(this,tr("Choose Hopsan Data File"),
                                                    gpConfig->getStringSetting(CFG_PLOTDATADIR),
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
    QRadioButton *pHdf5Button = new QRadioButton("hdf5");
    pFormatButtons->addButton(pPLOv1Button);
    pFormatButtons->addButton(pPLOv2Button);
    pFormatButtons->addButton(pCSVButton);
    pFormatButtons->addButton(pHdf5Button);
    pPLOv1Button->setChecked(true);

    QGroupBox *pFormatGroupBox = new QGroupBox("Choose Export Format:", &exportOptions);
    QHBoxLayout *pFormatButtonLayout = new QHBoxLayout();
    pFormatButtonLayout->addWidget(pPLOv1Button);
    pFormatButtonLayout->addWidget(pPLOv2Button);
    pFormatButtonLayout->addWidget(pCSVButton);
    pFormatButtonLayout->addWidget(pHdf5Button);
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
        else if (pFormatButtons->checkedButton() == pHdf5Button)
        {
            suffixFilter = "*.h5";
        }
        else
        {
            suffixFilter = "*.csv *.CSV";
        }


        // Get save file name
        if ((pFilenameButtons->checkedButton() == pAppendGenButton) && (gens.size() > 1))
        {
            QString fileName = QFileDialog::getSaveFileName(this,tr("Choose Hopsan Data File Name"), gpConfig->getStringSetting(CFG_PLOTDATADIR), tr("Data Files")+QString(" (%1)").arg(suffixFilter));
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
                fNames.append(QFileDialog::getSaveFileName(this,QString("Choose File Name for Gen: %1").arg(gens[i]), gpConfig->getStringSetting(CFG_PLOTDATADIR),  tr("Data Files")+QString(" (%1)").arg(suffixFilter)));
            }
        }

        // Remember this output dir for later use
        if (!fNames.isEmpty())
        {
            QFileInfo exportPath(fNames.last());
            gpConfig->setStringSetting(CFG_PLOTDATADIR, exportPath.absolutePath());
        }

        QProgressDialog progress("Exporting Data", "Cancel", 0, fNames.size(), this);
        progress.setWindowModality(Qt::WindowModal);
        progress.show();
        progress.setValue(0);
        for (int i=0; i<fNames.size(); ++i)
        {
            // Abort if canceled
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
                else if (pFormatButtons->checkedButton() == pHdf5Button)
                {
                    mpLogDataHandler->exportGenerationToHDF5(file, g);
                }
                else
                {
                    mpLogDataHandler->exportGenerationToCSV(file, g);
                }
            }
            progress.setValue(i+1);
        }
        progress.setValue(progress.maximum());
    }
}

void DataExplorer::refreshGenerationList()
{
    if (mpGenerationsListWidget)
    {
        mGenerationItemMap.clear();
        mpGenerationsListWidget->deleteLater();
        mpGenerationsListWidget = nullptr;
    }

    if (mpLogDataHandler)
    {
        mpGenerationsListWidget = new QWidget(this);
        QVBoxLayout *pGenerationListLayout = new QVBoxLayout(mpGenerationsListWidget);
        pGenerationListLayout->addWidget(new QLabel("Generations", mpGenerationsListWidget),0,Qt::AlignHCenter);

        QList<int> gens = mpLogDataHandler->getGenerations();
        LogDataHandler2::ImportedGenerationsMapT importedGensFileMap = mpLogDataHandler->getImportFilesAndGenerations();
        for (int g : gens)
        {
            GenerationItem *pItem;
            if (importedGensFileMap.contains(g))
            {
                //! @todo maybe should check values in case of multiple files, but not sure that can even happen (it should not happen)
                pItem = new GenerationItem(g, importedGensFileMap.value(g), mpGenerationsListWidget);
            }
            else
            {
                pItem = new GenerationItem(g, "Simulated", mpGenerationsListWidget);
            }
            pGenerationListLayout->addWidget(pItem, 1, Qt::AlignTop);
            mGenerationItemMap.insert(g,pItem);
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
    progress.setValue(0);
    // Since removeGeneration will also remove from mGenerationItemMap, we cant remove directly in for loop above
    for (int i=0; i<gens.size(); ++i)
    {
        // Abort if canceled
        if (progress.wasCanceled())
            break;

        removeGeneration(gens[i]);
        progress.setValue(i+1);
    }
    progress.setValue(progress.maximum());
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
