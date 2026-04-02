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
//! @file   SSVEditorWidget.cpp
//! @brief A widget for editing SSV parameter sets
//! @author Robert Braun <robert.braun@liu.se>
//!
//$Id$

#include "SSVEditorWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QHeaderView>
#include <QDebug>

#include "ssp4c.h"
#include "ssp4c_ssv_parameter_set.h"
#include "ssp4c_ssv_parameter.h"

SSVEditorWidget::SSVEditorWidget(ssvParameterSetHandle *pSsv, QString fileName, QWidget *parent)
    : QWidget(parent), mpSsv(pSsv), mFileName(fileName)
{
    // Create table widget with columns for Name, Unit, and Value
    mpTable = new QTableWidget(this);
    mpTable->setColumnCount(3);
    mpTable->setHorizontalHeaderLabels(QStringList() << "Name" << "Unit" << "Value");
    mpTable->horizontalHeader()->setStretchLastSection(true);
    mpTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    mpTable->setSelectionMode(QAbstractItemView::SingleSelection);

    // Create buttons
    mpAddButton = new QPushButton("Add Parameter", this);
    mpRemoveButton = new QPushButton("Remove Parameter", this);

    // Connect button signals
    connect(mpAddButton, SIGNAL(clicked()), this, SLOT(addParameter()));
    connect(mpRemoveButton, SIGNAL(clicked()), this, SLOT(removeParameter()));

    // Create layout
    QVBoxLayout *pMainLayout = new QVBoxLayout(this);
    pMainLayout->addWidget(mpTable);

    QHBoxLayout *pButtonLayout = new QHBoxLayout();
    pButtonLayout->addWidget(mpAddButton);
    pButtonLayout->addWidget(mpRemoveButton);
    pButtonLayout->addStretch();

    pMainLayout->addLayout(pButtonLayout);

    // Load the SSV data
    loadSSVData();
}

SSVEditorWidget::~SSVEditorWidget()
{
}

void SSVEditorWidget::loadSSVData()
{
    if (!mpSsv)
        return;

    // Clear existing rows
    mpTable->setRowCount(0);

    // Get number of parameters
    int paramCount = ssp4c_ssv_parameterSet_getNumberOfParameters(mpSsv);

    // Add each parameter as a table row
    for (int i = 0; i < paramCount; ++i) {
        ssvParameterHandle *pParam = ssp4c_ssv_parameterSet_getParameterByIndex(mpSsv, i);
        if (!pParam)
            continue;

        // Insert a new row
        int row = mpTable->rowCount();
        mpTable->insertRow(row);

        // Get parameter data
        QString name = QString::fromUtf8(ssp4c_ssv_parameter_getName(pParam));
        QString unit = QString::fromUtf8(ssp4c_ssv_parameter_getUnit(pParam));
        QString value;

        sspDataType type = ssp4c_ssv_parameter_getDatatype(pParam);
        if (type == sspDataTypeString) {
            value = QString::fromUtf8(ssp4c_ssv_parameter_getStringValue(pParam));
        } else if (type == sspDataTypeReal || type == sspDataTypeFloat64 || type == sspDataTypeFloat32) {
            value = QString::number(ssp4c_ssv_parameter_getRealValue(pParam));
        } else if (type == sspDataTypeInteger || 
                   type == sspDataTypeInt8 || type == sspDataTypeUInt8 ||
                   type == sspDataTypeInt16 || type == sspDataTypeUInt16 ||
                   type == sspDataTypeInt32 || type == sspDataTypeUInt32 ||
                   type == sspDataTypeInt64 || type == sspDataTypeUInt64) {
            // For integer types, try to get as double and convert
            value = QString::number((int)ssp4c_ssv_parameter_getRealValue(pParam));
        } else if (type == sspDataTypeBoolean) {
            // For boolean, show as "true" or "false" based on the value
            double boolValue = ssp4c_ssv_parameter_getRealValue(pParam);
            value = (boolValue != 0.0) ? "true" : "false";
        } else {
            // For other types (Enumeration, Binary, etc.), try to get string representation
            value = QString::fromUtf8(ssp4c_ssv_parameter_getStringValue(pParam));
        }

        // Add items to table
        QTableWidgetItem *pNameItem = new QTableWidgetItem(name);
        QTableWidgetItem *pUnitItem = new QTableWidgetItem(unit);
        QTableWidgetItem *pValueItem = new QTableWidgetItem(value);

        // Add items to row
        mpTable->setItem(row, 0, pNameItem);
        mpTable->setItem(row, 1, pUnitItem);
        mpTable->setItem(row, 2, pValueItem);

        // Make name and unit read-only, but allow value editing
        pNameItem->setFlags(pNameItem->flags() & ~Qt::ItemIsEditable);
        pUnitItem->setFlags(pUnitItem->flags() & ~Qt::ItemIsEditable);
    }

    // Resize columns to content
    mpTable->resizeColumnsToContents();
}


void SSVEditorWidget::addParameter()
{
    // Insert a new row at the end
    int row = mpTable->rowCount();
    mpTable->insertRow(row);

    // Create editable items
    QTableWidgetItem *pNameItem = new QTableWidgetItem("New_Parameter");
    QTableWidgetItem *pUnitItem = new QTableWidgetItem("");
    QTableWidgetItem *pValueItem = new QTableWidgetItem("0");

    // Make name and unit read-only
    pNameItem->setFlags(pNameItem->flags() & ~Qt::ItemIsEditable);
    pUnitItem->setFlags(pUnitItem->flags() & ~Qt::ItemIsEditable);

    // Add items to row
    mpTable->setItem(row, 0, pNameItem);
    mpTable->setItem(row, 1, pUnitItem);
    mpTable->setItem(row, 2, pValueItem);

    // Resize columns
    mpTable->resizeColumnsToContents();

    // Note: Actual SSV data modification would happen on save
}

void SSVEditorWidget::removeParameter()
{
    // Remove currently selected row
    int currentRow = mpTable->currentRow();
    if (currentRow >= 0) {
        mpTable->removeRow(currentRow);
    }

    // Note: Actual SSV data modification would happen on save
}
