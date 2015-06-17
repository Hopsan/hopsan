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

#include "FindWidget.h"

#include "GUIObjects/GUIModelObject.h"
#include "GUIObjects/GUIContainerObject.h"
#include "ModelWidget.h"

#include "GraphicsView.h"

#include <QLineEdit>
#include <QLabel>
#include <QToolButton>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>


FindHelper::FindHelper(QWidget *pParent) :
    QWidget(pParent)
{
    mpLineEdit = new QLineEdit(this);

    QToolButton *pClearButton = new QToolButton(this);
    pClearButton->setIcon(QIcon(ICONPATH"Hopsan-Discard.png"));
    pClearButton->setToolTip("Clear");
    pClearButton->setMaximumWidth(24);

    QToolButton *pFindButton = new QToolButton(this);
    pFindButton->setToolTip("Find");
    pFindButton->setIcon(QIcon(ICONPATH"Hopsan-Zoom.png"));

    QHBoxLayout *pLayout = new QHBoxLayout(this);
    pLayout->addWidget(pClearButton);
    pLayout->addWidget(mpLineEdit);
    pLayout->addWidget(pFindButton);

    connect(pClearButton, SIGNAL(clicked()), mpLineEdit, SLOT(clear()));
    connect(mpLineEdit, SIGNAL(returnPressed()), this, SLOT(doFind()));
    connect(pFindButton, SIGNAL(clicked()), this, SLOT(doFind()));
}

void FindHelper::doFind()
{
    emit find(mpLineEdit->text());
}



FindWidget::FindWidget(QWidget *parent) :
    QDialog(parent)
{
    FindHelper *pComponentFinder = new FindHelper(this);
    FindHelper *pAliasFinder = new FindHelper(this);
    FindHelper *pSystemparFinder = new FindHelper(this);
    QPushButton *pCloseButton = new QPushButton("Close", this);
    pCloseButton->setAutoDefault(false);

    QVBoxLayout *pLayout = new QVBoxLayout(this);
    pLayout->addWidget(new QLabel("Find by name. Note! You can use wildcard matching: (? one any charachter), (* one or more any characters), ([...] specific characters)",this));
    pLayout->addSpacing(10);
    pLayout->addWidget(new QLabel("Find Component:", this));
    pLayout->addWidget(pComponentFinder);
    pLayout->addWidget(new QLabel("Find Variable Alias:", this));
    pLayout->addWidget(pAliasFinder);
    pLayout->addWidget(new QLabel("Find System Parameter:", this));
    pLayout->addWidget(pSystemparFinder);
    pLayout->addWidget(pCloseButton,0,Qt::AlignRight);

    connect(pComponentFinder, SIGNAL(find(QString)), this, SLOT(findComponent(QString)));
    connect(pAliasFinder, SIGNAL(find(QString)), this, SLOT(findAlias(QString)));
    connect(pSystemparFinder, SIGNAL(find(QString)), this, SLOT(findSystemParameter(QString)));
    connect(pCloseButton, SIGNAL(clicked()), this, SLOT(close()));

    resize(600, height());
    setWindowTitle("Find Widget");
}

void FindWidget::setContainer(ContainerObject *pContainer)
{
    mpContainer = pContainer;
}

void FindWidget::findComponent(const QString &rName, const bool centerView)
{
    if (mpContainer)
    {
        clearHighlights();

        QPointF mean;
        int nFound=0;
        QStringList compNames = mpContainer->getModelObjectNames();
        //!  @todo what about searching in subsystems
        foreach(QString comp, compNames)
        {
            QRegExp re(rName, Qt::CaseInsensitive, QRegExp::Wildcard);
            //if (comp.compare(rName, Qt::CaseInsensitive) == 0)
            if (re.exactMatch(comp))
            {
                ModelObject *pMO = mpContainer->getModelObject(comp);
                if (pMO)
                {
                    ++nFound;
                    pMO->highlight();
                    mean += pMO->pos();
                }
            }
        }

        // Now center view over found model objects
        if (nFound > 0 && centerView)
        {
            mean /= double(nFound);
            mpContainer->mpModelWidget->getGraphicsView()->centerOn(mean);
        }
    }
}

void FindWidget::findAlias(const QString &rName, const bool centerView)
{
    if (mpContainer)
    {
        clearHighlights();

        QPointF mean;
        int nFound=0;
        QStringList aliasNames = mpContainer->getAliasNames();
        //!  @todo what about searching in subsystems
        foreach(QString alias, aliasNames)
        {
            QRegExp re(rName, Qt::CaseInsensitive, QRegExp::Wildcard);
            //if (alias.compare(rName, Qt::CaseInsensitive) == 0)
            if (re.exactMatch(alias))
            {
                QString fullName = mpContainer->getFullNameFromAlias(alias);
                QString comp, port, var;
                QStringList sysHierarchy;
                splitFullVariableName(fullName, sysHierarchy, comp, port, var);
                ModelObject *pMO = mpContainer->getModelObject(comp);
                //! @todo we should actually highlight the port also (and center on the port)
                if (pMO)
                {
                    ++nFound;
                    pMO->highlight();
                    mean += pMO->pos();
                }
            }
        }

        // Now center view over found model objects
        if (nFound > 0 && centerView)
        {
            mean /= double(nFound);
            mpContainer->mpModelWidget->getGraphicsView()->centerOn(mean);
        }
    }
}

void FindWidget::findSystemParameter(const QString &rName, const bool centerView)
{
    if (mpContainer)
    {
        clearHighlights();

        QPointF mean;
        int nFound=0;
        const QList<ModelObject*> mops = mpContainer->getModelObjects();
        foreach(ModelObject* pMO, mops)
        {
            bool hasPar = false;
            QVector<CoreParameterData> pars;
            pMO->getParameters(pars);
            foreach(CoreParameterData par, pars)
            {
                QRegExp re(rName, Qt::CaseInsensitive, QRegExp::Wildcard);
                QRegExp reNeg("-"+rName, Qt::CaseInsensitive, QRegExp::Wildcard);
                //if(par.mValue == rName)
                if (re.exactMatch(par.mValue) || reNeg.exactMatch(par.mValue))
                {
                    hasPar = true;
                }
            }
            if(hasPar)
            {
                pMO->highlight();
                ++nFound;
                mean += pMO->pos();
            }
        }

        if (nFound > 0 && centerView)
        {
            mean /= double(nFound);
            mpContainer->mpModelWidget->getGraphicsView()->centerOn(mean);
        }
    }
}

void FindWidget::findAny(const QString &rName)
{
    //Not yet implemented
}

void FindWidget::clearHighlights()
{
    mpContainer->mpModelWidget->getGraphicsView()->clearHighlights();
}
