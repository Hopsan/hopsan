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

#include <QGridLayout>
#include <QKeyEvent>

#include "ProjectFilesWidget.h"
#include "Handlers/FileHandler.h"

ProjectFilesWidget::ProjectFilesWidget(QWidget *parent) :
    QWidget(parent)
{
    //Create widgets
    mpTreeWidget = new QTreeWidget(this);

    //Setup layout
    QGridLayout *pLayout = new QGridLayout(this);
    pLayout->addWidget(mpTreeWidget, 0, 0);
    this->setLayout(pLayout);

    QFont boldFont = this->font();
    boldFont.setBold(true);

    mpProjectFilesTopLevelItem = new QTreeWidgetItem(0);
    mpComponentFilesTopLevelItem = new QTreeWidgetItem(0);
    mpAuxiliaryFilesTopLevelItem = new QTreeWidgetItem(0);
    mpAppearanceFilesTopLevelItem = new QTreeWidgetItem(0);

    mpProjectFilesTopLevelItem->setText(0,"Project Files");
    mpComponentFilesTopLevelItem->setText(0,"Component Files");
    mpAuxiliaryFilesTopLevelItem->setText(0,"Auxiliary Files");
    mpAppearanceFilesTopLevelItem->setText(0,"Appearance Files");

    mpProjectFilesTopLevelItem->setFont(0,boldFont);
    mpComponentFilesTopLevelItem->setFont(0,boldFont);
    mpAuxiliaryFilesTopLevelItem->setFont(0,boldFont);
    mpAppearanceFilesTopLevelItem->setFont(0,boldFont);

    mpTreeWidget->addTopLevelItem(mpProjectFilesTopLevelItem);
    mpTreeWidget->addTopLevelItem(mpComponentFilesTopLevelItem);
    mpTreeWidget->addTopLevelItem(mpAuxiliaryFilesTopLevelItem);
    mpTreeWidget->addTopLevelItem(mpAppearanceFilesTopLevelItem);
}

QTreeWidgetItem *ProjectFilesWidget::addFile(const FileObject *pFile)
{
    QTreeWidgetItem *pNewItem = new QTreeWidgetItem(0);
    pNewItem->setText(0,pFile->mFileInfo.fileName());
    if(pFile->mType == FileObject::XML || pFile->mType == FileObject::Source)
    {
        mpProjectFilesTopLevelItem->addChild(pNewItem);
        mpProjectFilesTopLevelItem->setExpanded(true);
    }
    else if(pFile->mType == FileObject::Component)
    {
        mpComponentFilesTopLevelItem->addChild(pNewItem);
        mpComponentFilesTopLevelItem->setExpanded(true);
    }
    else if(pFile->mType == FileObject::Auxiliary)
    {
        mpAuxiliaryFilesTopLevelItem->addChild(pNewItem);
        mpAuxiliaryFilesTopLevelItem->setExpanded(true);
    }
    else if(pFile->mType == FileObject::CAF)
    {
        mpAppearanceFilesTopLevelItem->addChild(pNewItem);
        mpAppearanceFilesTopLevelItem->setExpanded(true);
    }

    update();
    return pNewItem;
}

void ProjectFilesWidget::addAsterisk()
{
    if(mpTreeWidget->currentItem() && !mpTreeWidget->currentItem()->text(0).endsWith("*"))
    {
        mpTreeWidget->currentItem()->setText(0, mpTreeWidget->currentItem()->text(0)+"*");
    }
}

void ProjectFilesWidget::removeAsterisks()
{
    QTreeWidgetItemIterator it(mpTreeWidget);
    while(*it)
    {
        QString text = (*it)->text(0);
        while(text.endsWith("*"))
            text.chop(1);
        (*it)->setText(0, text);
        ++it;
    }
}


void ProjectFilesWidget::removeItem(QTreeWidgetItem *pItem)
{
    mpProjectFilesTopLevelItem->removeChild(pItem);
    mpComponentFilesTopLevelItem->removeChild(pItem);
    mpAuxiliaryFilesTopLevelItem->removeChild(pItem);
    mpAppearanceFilesTopLevelItem->removeChild(pItem);
}

void ProjectFilesWidget::clear()
{
    QList<QTreeWidgetItem*> items = mpProjectFilesTopLevelItem->takeChildren();
    items.append(mpComponentFilesTopLevelItem->takeChildren());
    items.append(mpAuxiliaryFilesTopLevelItem->takeChildren());
    items.append(mpAppearanceFilesTopLevelItem->takeChildren());

    foreach(const QTreeWidgetItem *item, items)
    {
        delete(item);
    }

//    while(mpProjectFilesTopLevelItem->childCount() > 0)
//    {
//        mpProjectFilesTopLevelItem->removeChild(mpProjectFilesTopLevelItem->child(0));
//    }
//    while(mpComponentFilesTopLevelItem->childCount() > 0)
//    {
//        mpComponentFilesTopLevelItem->removeChild(mpComponentFilesTopLevelItem->child(0));
//    }

//    while(mpAuxiliaryFilesTopLevelItem->childCount() > 0)
//    {
//        mpAuxiliaryFilesTopLevelItem->removeChild(mpAuxiliaryFilesTopLevelItem->child(0));
    //    }
}

void ProjectFilesWidget::update()
{
    mpProjectFilesTopLevelItem->setHidden(mpProjectFilesTopLevelItem->childCount() == 0);
    mpComponentFilesTopLevelItem->setHidden(mpComponentFilesTopLevelItem->childCount() == 0);
    mpAuxiliaryFilesTopLevelItem->setHidden(mpAuxiliaryFilesTopLevelItem->childCount() == 0);
    mpAppearanceFilesTopLevelItem->setHidden(mpAppearanceFilesTopLevelItem->childCount() == 0);

    mpTreeWidget->update();
}

void ProjectFilesWidget::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Delete)
    {
        QTreeWidgetItem *pCurrentItem = mpTreeWidget->currentItem();
        if(pCurrentItem != mpProjectFilesTopLevelItem &&
           pCurrentItem != mpComponentFilesTopLevelItem &&
           pCurrentItem != mpAuxiliaryFilesTopLevelItem &&
           pCurrentItem != mpAppearanceFilesTopLevelItem)
        {
            emit deleteRequested(pCurrentItem);
        }
    }
}
