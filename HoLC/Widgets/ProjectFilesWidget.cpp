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
    mpProjectFilesTopLevelItem->setText(0,"Project Files");
    mpComponentFilesTopLevelItem->setText(0,"Component Files");
    mpAuxiliaryFilesTopLevelItem->setText(0,"Auxiliary Files");
    mpProjectFilesTopLevelItem->setFont(0,boldFont);
    mpComponentFilesTopLevelItem->setFont(0,boldFont);
    mpAuxiliaryFilesTopLevelItem->setFont(0,boldFont);
    mpTreeWidget->addTopLevelItem(mpProjectFilesTopLevelItem);
    mpTreeWidget->addTopLevelItem(mpComponentFilesTopLevelItem);
    mpTreeWidget->addTopLevelItem(mpAuxiliaryFilesTopLevelItem);
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
    return pNewItem;
}

void ProjectFilesWidget::addAsterisk()
{
    if(!mpTreeWidget->currentItem()->text(0).endsWith("*"))
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
}

void ProjectFilesWidget::clear()
{
    while(mpProjectFilesTopLevelItem->childCount() > 0)
    {
        mpProjectFilesTopLevelItem->removeChild(mpProjectFilesTopLevelItem->child(0));
    }
    while(mpComponentFilesTopLevelItem->childCount() > 0)
    {
        mpComponentFilesTopLevelItem->removeChild(mpComponentFilesTopLevelItem->child(0));
    }

    while(mpAuxiliaryFilesTopLevelItem->childCount() > 0)
    {
        mpAuxiliaryFilesTopLevelItem->removeChild(mpAuxiliaryFilesTopLevelItem->child(0));
    }
}

void ProjectFilesWidget::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Delete)
    {
        QTreeWidgetItem *pCurrentItem = mpTreeWidget->currentItem();
        if(pCurrentItem != mpProjectFilesTopLevelItem &&
           pCurrentItem != mpComponentFilesTopLevelItem &&
           pCurrentItem != mpAuxiliaryFilesTopLevelItem)
        {
            emit deleteRequested(pCurrentItem);
        }
    }
}
