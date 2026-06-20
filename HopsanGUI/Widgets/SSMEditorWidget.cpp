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

//!
//! @file   SSMEditorWidget.cpp
//! @brief A widget for editing SSM parameter mappings
//! @author Robert Braun <robert.braun@liu.se>
//!

#include "SSMEditorWidget.h"
#include <QDebug>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QVBoxLayout>

#include "ssp4c_ssm_parameter_mapping.h"
#include "ssp4c_ssm_mapping_entry.h"

SSMEditorWidget::SSMEditorWidget(ssmParameterMappingHandle *pSsm, QString fileName, QWidget *parent)
    : QWidget(parent), mpSsm(pSsm), mFileName(fileName)
{
    // Create table widget with columns for Source and Target
    mpTable = new QTableWidget(this);
    mpTable->setColumnCount(2);
    mpTable->setHorizontalHeaderLabels(QStringList() << "Source" << "Target");
    mpTable->horizontalHeader()->setStretchLastSection(true);
    mpTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    mpTable->setSelectionMode(QAbstractItemView::SingleSelection);

    // Create buttons
    mpAddButton = new QPushButton("Add Mapping", this);
    mpRemoveButton = new QPushButton("Remove Mapping", this);

    // Connect button signals
    connect(mpAddButton, SIGNAL(clicked()), this, SLOT(addMapping()));
    connect(mpRemoveButton, SIGNAL(clicked()), this, SLOT(removeMapping()));

    // Create layout
    QVBoxLayout *pMainLayout = new QVBoxLayout(this);
    pMainLayout->addWidget(mpTable);

    QHBoxLayout *pButtonLayout = new QHBoxLayout();
    pButtonLayout->addWidget(mpAddButton);
    pButtonLayout->addWidget(mpRemoveButton);
    pButtonLayout->addStretch();

    pMainLayout->addLayout(pButtonLayout);

    // Load the SSM data
    loadSSMData();
}

SSMEditorWidget::~SSMEditorWidget()
{
}

void SSMEditorWidget::loadSSMData()
{
    if (!mpSsm)
        return;

    // Clear existing rows
    mpTable->setRowCount(0);

    // Get number of mapping entries
    int entryCount = ssp4c_ssm_parameterMapping_getNumberOfMappingEntries(mpSsm);

    // Add each mapping entry as a table row
    for (int i = 0; i < entryCount; ++i) {
        ssmParameterMappingEntryHandle *pEntry = ssp4c_ssm_parameterMapping_getMappingEntryByIndex(mpSsm, i);
        if (!pEntry)
            continue;

        // Insert a new row
        int row = mpTable->rowCount();
        mpTable->insertRow(row);

        // Get mapping data
        QString source = QString::fromUtf8(ssp4c_ssm_mappingEntry_getSource(pEntry));
        QString target = QString::fromUtf8(ssp4c_ssm_mappingEntry_getTarget(pEntry));

        // Add items to table
        QTableWidgetItem *pSourceItem = new QTableWidgetItem(source);
        QTableWidgetItem *pTargetItem = new QTableWidgetItem(target);

        // Add items to row
        mpTable->setItem(row, 0, pSourceItem);
        mpTable->setItem(row, 1, pTargetItem);
    }

    // Resize columns to content
    mpTable->resizeColumnsToContents();
}

void SSMEditorWidget::addMapping()
{
    // Insert a new row at the end
    int row = mpTable->rowCount();
    mpTable->insertRow(row);

    // Create editable items
    QTableWidgetItem *pSourceItem = new QTableWidgetItem("");
    QTableWidgetItem *pTargetItem = new QTableWidgetItem("");

    // Add items to row
    mpTable->setItem(row, 0, pSourceItem);
    mpTable->setItem(row, 1, pTargetItem);

    // Resize columns
    mpTable->resizeColumnsToContents();

    // Note: Actual SSM data modification would happen on save
}

void SSMEditorWidget::removeMapping()
{
    // Remove currently selected row
    int currentRow = mpTable->currentRow();
    if (currentRow >= 0) {
        mpTable->removeRow(currentRow);
    }

    // Note: Actual SSM data modification would happen on save
}
