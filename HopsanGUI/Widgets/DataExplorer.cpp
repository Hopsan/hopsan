/*-----------------------------------------------------------------------------

 Copyright 2017 Hopsan Group

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

 The full license is available in the file GPLv3.
 For details about the 'Hopsan Group' or information about Authors and
 Contributors see the HOPSANGROUP and AUTHORS files that are located in
 the Hopsan source code root directory.

-----------------------------------------------------------------------------*/

//$Id$

#include "global.h"
#include "DataExplorer.h"
#include "LogDataHandler2.h"
#include "Configuration.h"

#include <QObject>
#include <QLabel>
#include <QLayout>
#include <QCheckBox>
#include <QPushButton>
#include <QFileDialog>
#include <QButtonGroup>
#include <QRadioButton>
#include <QGroupBox>
#include <QProgressDialog>
#include <QWidget>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QMap>
#include <QPointer>

namespace {

class GenerationItem : public QWidget
{
public:
    GenerationItem(const int genNum, const QString &rDescription, QWidget *pParent) : QWidget(pParent)
    {
        auto *pHLayout = new QHBoxLayout(this);
        pHLayout->addWidget(&mChosenCheckBox);
        pHLayout->addWidget(new QLabel(QString("Gen: %1  %2").arg(genNum+1).arg(rDescription), this));
        pHLayout->setSizeConstraint(QLayout::SetFixedSize);
    }

    QCheckBox mChosenCheckBox;
};

}

class DataExplorerPrivates : public QObject
{
    Q_OBJECT
public:
    DataExplorerPrivates(QWidget* parent) : mpParentWidget(parent)
    {
        // Create generations buttons
        mpGenButtonsWidget = new QWidget(mpParentWidget);
        auto pGenButtonsLayout = new QGridLayout(mpGenButtonsWidget);
        mpImportGenButton = new QPushButton("Import", mpGenButtonsWidget);
        mpExportGenButton = new QPushButton("Export", mpGenButtonsWidget);
        mpDeleteGenButton = new QPushButton("Remove", mpGenButtonsWidget);
        auto pSelectAllGensButton = new QPushButton("(Un)Select all", mpGenButtonsWidget);
        pGenButtonsLayout->addWidget(mpImportGenButton,0,0);
        pGenButtonsLayout->addWidget(mpExportGenButton,1,0);
        pGenButtonsLayout->addWidget(mpDeleteGenButton,1,1);
        pGenButtonsLayout->addWidget(pSelectAllGensButton,2,0);
        pGenButtonsLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);

        connect(mpExportGenButton, SIGNAL(clicked()), this, SLOT(openExportDataDialog()));
        connect(mpImportGenButton, SIGNAL(clicked()), this, SLOT(openImportDataDialog()));
        connect(mpDeleteGenButton, SIGNAL(clicked()), this, SLOT(removeSelectedGenerations()));
        connect(pSelectAllGensButton, SIGNAL(clicked()), this, SLOT(toggleSelectAllGenerations()));

        mpGenerationsScrollArea = new QScrollArea(mpParentWidget);

        // This will initialize button states (to disabled)
        setLogdataHandler(nullptr);
    }

    QVector<int> gensFromSelected()
    {
        QVector<int> gens;
        for (auto it = mGenerationItemMap.begin(); it != mGenerationItemMap.end(); ++it) {
            if (it.value()->mChosenCheckBox.isChecked()) {
                gens.append(it.key());
            }
        }
        return gens;
    }

    void setLogdataHandler(LogDataHandler2 *pLogDataHandler)
    {
        if (mpLogDataHandler) {
            disconnect(mpLogDataHandler, nullptr, this, nullptr);
        }
        mpLogDataHandler = pLogDataHandler;

        const bool haveLogDataHandler = mpLogDataHandler!=nullptr;
        if (haveLogDataHandler) {
            connect(mpLogDataHandler, SIGNAL(dataAdded()), this, SLOT(refreshGenerationList()));
            connect(mpLogDataHandler, SIGNAL(dataRemoved()), this, SLOT(refreshGenerationList()));
        }
        mpDeleteGenButton->setEnabled(haveLogDataHandler);
        mpImportGenButton->setEnabled(haveLogDataHandler);
        mpExportGenButton->setEnabled(haveLogDataHandler);

        refreshGenerationList();
    }

public slots:
    void refreshGenerationList()
    {
        for (auto& value : mGenerationItemMap) {
            value->deleteLater();
        }
        mGenerationItemMap.clear();
        if (mpGenerationsListWidget) {
            mpGenerationsListWidget->deleteLater();
            mpGenerationsListWidget = nullptr;
        }

        if (mpLogDataHandler) {
            mpGenerationsListWidget = new QWidget(mpParentWidget);
            auto *pGenerationListLayout = new QVBoxLayout(mpGenerationsListWidget);
            pGenerationListLayout->addWidget(new QLabel("Generations", mpGenerationsListWidget),0,Qt::AlignHCenter);

            QList<int> gens = mpLogDataHandler->getGenerations();
            LogDataHandler2::ImportedGenerationsMapT importedGensFileMap = mpLogDataHandler->getImportFilesAndGenerations();
            for (const int g : gens) {
                GenerationItem *pItem;
                if (importedGensFileMap.contains(g)) {
                    //! @todo maybe should check values in case of multiple files, but not sure that can even happen (it should not happen)
                    pItem = new GenerationItem(g, importedGensFileMap.value(g), mpGenerationsListWidget);
                }
                else {
                    pItem = new GenerationItem(g, "Simulated", mpGenerationsListWidget);
                }
                pGenerationListLayout->addWidget(pItem, 1, Qt::AlignTop);
                mGenerationItemMap.insert(g, pItem);
            }
            // Pushes all items to top
            pGenerationListLayout->addStretch(2);

            mpGenerationsScrollArea->setWidget(mpGenerationsListWidget);
            mpGenerationsListWidget->show();
        }
    }

    void removeSelectedGenerations()
    {
        QVector<int> gens = gensFromSelected();
        QProgressDialog progress("Removing generations", "Cancel", 0, gens.size(), mpParentWidget);
        progress.setWindowModality(Qt::WindowModal);
        progress.show();
        progress.setValue(0);
        // Since removeGeneration will also remove from mGenerationItemMap, we cant remove directly in for loop above
        for (const int g : gens) {
            // Abort if canceled
            if (progress.wasCanceled()) {
                break;
            }
            removeGeneration(g);
            progress.setValue(progress.value()+1);
        }
        progress.setValue(progress.maximum());
    }

    void removeGeneration(const int gen)
    {
        if (mpLogDataHandler) {
            // Force removal of data generation
            mpLogDataHandler->removeGeneration(gen, true);
            // We assume that the generation was removed, now delete it from the view (quicker than reloading the view)
            GenerationItem *pItem = mGenerationItemMap.value(gen);
            if (pItem) {
                pItem->deleteLater();
                mGenerationItemMap.remove(gen);
            }
        }
    }

    void toggleSelectAllGenerations()
    {
        mAllSelectedToggle = !mAllSelectedToggle;
        for (auto& value : mGenerationItemMap) {
            value->mChosenCheckBox.setChecked(mAllSelectedToggle);
        }
    }

    void openImportDataDialog()
    {
        QFileDialog fd(mpParentWidget, tr("Choose Hopsan Data File"), gpConfig->getStringSetting(cfg::dir::plotdata),
                       tr("Data Files (*.plo *.PLO *.csv *.CSV);; Space-separated Column Data (*.*);; All (Treat as csv) (*.*)"));
        fd.setFileMode(QFileDialog::ExistingFiles);
        const auto rc = fd.exec();
        QStringList selectedFiles = fd.selectedFiles();
        QString selectedNameFilter = fd.selectedNameFilter();
        if (rc==QDialog::Accepted && mpLogDataHandler!=nullptr && !selectedFiles.isEmpty()) {
            for (QString &file : selectedFiles) {
                QFileInfo fi(file);
                if (selectedNameFilter.contains("Space-separated")) {
                    mpLogDataHandler->importFromPlainColumnCsv(file, ' ');
                }
                else if (fi.suffix().toLower() == "plo") {
                    mpLogDataHandler->importFromPlo(file);
                }
                else {
                    mpLogDataHandler->importFromCSV_AutoFormat(file);
                }
            }
        }
    }

    void openExportDataDialog()
    {
        QDialog exportOptions;

        QButtonGroup *pFormatButtons = new QButtonGroup(&exportOptions);
        QRadioButton *pPLOv1Button = new QRadioButton("PLO v1");
        QRadioButton *pPLOv2Button = new QRadioButton("PLO v2");
        QRadioButton *pPLOv3Button = new QRadioButton("PLO v3");
        QRadioButton *pCSVButton = new QRadioButton("csv");
        QRadioButton *pHdf5Button = new QRadioButton("hdf5");
        pFormatButtons->addButton(pPLOv1Button);
        pFormatButtons->addButton(pPLOv2Button);
        pFormatButtons->addButton(pPLOv3Button);
        pFormatButtons->addButton(pCSVButton);
        pFormatButtons->addButton(pHdf5Button);
        pPLOv2Button->setChecked(true);

        QGroupBox *pFormatGroupBox = new QGroupBox("Choose Export Format:", &exportOptions);
        QHBoxLayout *pFormatButtonLayout = new QHBoxLayout();
        pFormatButtonLayout->addWidget(pPLOv1Button);
        pFormatButtonLayout->addWidget(pPLOv2Button);
        pFormatButtonLayout->addWidget(pPLOv3Button);
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

        if (exportOptions.exec() == QDialog::Accepted) {
            QVector<int> gens = gensFromSelected();
            QStringList fNames;
            QString suffixFilter;

            // Select suffix filter for file format
            if ( (pFormatButtons->checkedButton() == pPLOv1Button) || (pFormatButtons->checkedButton() == pPLOv2Button)
                 || (pFormatButtons->checkedButton() == pPLOv3Button) ) {
                suffixFilter = "*.plo *.PLO";
            }
            else if (pFormatButtons->checkedButton() == pHdf5Button) {
                suffixFilter = "*.h5";
            }
            else {
                suffixFilter = "*.csv *.CSV";
            }


            // Get save file name
            if ((pFilenameButtons->checkedButton() == pAppendGenButton) && (gens.size() > 1)) {
                QString fileName = QFileDialog::getSaveFileName(mpParentWidget,tr("Choose Hopsan Data File Name"), gpConfig->getStringSetting(cfg::dir::plotdata), tr("Data Files")+QString(" (%1)").arg(suffixFilter));
                QFileInfo file(fileName);
                for (int i=0; i<gens.size(); ++i) {
                    fNames.append(file.absolutePath()+"/"+file.baseName()+QString("_%1").arg(gens[i])+"."+file.suffix());
                }
            }
            else {
                for (int i=0; i<gens.size(); ++i) {
                    fNames.append(QFileDialog::getSaveFileName(mpParentWidget,QString("Choose File Name for Gen: %1").arg(gens[i]), gpConfig->getStringSetting(cfg::dir::plotdata),  tr("Data Files")+QString(" (%1)").arg(suffixFilter)));
                }
            }

            // Remember this output dir for later use
            if (!fNames.isEmpty()) {
                QFileInfo exportPath(fNames.last());
                gpConfig->setStringSetting(cfg::dir::plotdata, exportPath.absolutePath());
            }

            QProgressDialog progress("Exporting Data", "Cancel", 0, fNames.size(), mpParentWidget);
            progress.setWindowModality(Qt::WindowModal);
            progress.show();
            progress.setValue(0);
            for (int i=0; i<fNames.size(); ++i) {
                // Abort if canceled
                if (progress.wasCanceled())
                    break;

                QString &file = fNames[i];
                if (!file.isEmpty()) {
                    const int g = gens[i];
                    if (pFormatButtons->checkedButton() == pPLOv1Button) {
                        mpLogDataHandler->exportGenerationToPlo(file, g, 1);
                    }
                    else if (pFormatButtons->checkedButton() == pPLOv2Button) {
                        mpLogDataHandler->exportGenerationToPlo(file, g, 2);
                    }
                    else if (pFormatButtons->checkedButton() == pPLOv3Button) {
                        mpLogDataHandler->exportGenerationToPlo(file, g, 3);
                    }
                    else if (pFormatButtons->checkedButton() == pHdf5Button) {
                        mpLogDataHandler->exportGenerationToHDF5(file, g);
                    }
                    else {
                        mpLogDataHandler->exportGenerationToCSV(file, g);
                    }
                }
                progress.setValue(i+1);
            }
            progress.setValue(progress.maximum());
        }
    }

public:

    QMap<int, GenerationItem*> mGenerationItemMap;
    QPointer<LogDataHandler2> mpLogDataHandler;
    QWidget *mpGenerationsListWidget = nullptr;
    QWidget *mpParentWidget = nullptr;
    QWidget *mpGenButtonsWidget = nullptr;
    QScrollArea *mpGenScrollArea = nullptr;
    QPushButton *mpImportGenButton = nullptr;
    QPushButton *mpExportGenButton = nullptr;
    QPushButton *mpDeleteGenButton = nullptr;
    QScrollArea *mpGenerationsScrollArea = nullptr;
    bool mAllSelectedToggle = false;
};

DataExplorer::DataExplorer(QWidget *parent) : QDialog(parent), mpPrivates(new DataExplorerPrivates(this))
{
    QGridLayout *pMainLayout = new QGridLayout(this);
    pMainLayout->addWidget(mpPrivates->mpGenButtonsWidget,0,0);
    pMainLayout->addWidget(mpPrivates->mpGenerationsScrollArea,1,0);
    resize(800, 600);
    setWindowTitle("Data Explorer");
}

DataExplorer::~DataExplorer() = default;

void DataExplorer::setLogdataHandler(LogDataHandler2 *pLogDataHandler)
{
    mpPrivates->setLogdataHandler(pLogDataHandler);
}

void DataExplorer::refresh()
{
    mpPrivates->refreshGenerationList();
}

void DataExplorer::openImportDataDialog()
{
    mpPrivates->openImportDataDialog();
}

void DataExplorer::openExportDataDialog()
{
    mpPrivates->openExportDataDialog();
}

#include "DataExplorer.moc"
