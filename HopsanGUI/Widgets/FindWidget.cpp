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

#include "FindWidget.h"

#include "GUIObjects/GUIModelObject.h"
#include "GUIObjects/GUIContainerObject.h"
#include "ModelWidget.h"
#include "GraphicsView.h"
#include "Utilities/GUIUtilities.h"

#include <QLineEdit>
#include <QLabel>
#include <QToolButton>
#include <QPushButton>
#include <QComboBox>
#include <QGridLayout>

FindWidget::FindWidget(QWidget *parent) :
    QWidget(parent)
{
    mpFindLineEdit = new QLineEdit(this);
    mpFindWhatComboBox = new QComboBox(this);
    mpFindWhatComboBox->addItem("Component");
    mpFindWhatComboBox->addItem("System Parameter");
    mpFindWhatComboBox->addItem("Alias");
    mpFindButton = new QPushButton("Find", this);
    mpFindButton->setShortcut(QKeySequence(Qt::Key_Enter));
    QToolButton *pCloseButton = new QToolButton(this);
    pCloseButton->setIcon(QIcon(":graphics/uiicons/Hopsan-Discard.png"));

    QGridLayout *pLayout = new QGridLayout(this);
    pLayout->addWidget(new QLabel("Find: ", this),  0,0);
    pLayout->addWidget(mpFindLineEdit,              0,1);
    pLayout->addWidget(mpFindWhatComboBox,          0,2);
    pLayout->addWidget(mpFindButton,                0,3);
    pLayout->addWidget(pCloseButton,                0,4);
    pLayout->setColumnStretch(1,1);

    connect(mpFindButton, SIGNAL(clicked()), this, SLOT(find()));
    connect(pCloseButton, SIGNAL(clicked()), this, SLOT(close()));

    resize(600, height());
    setWindowTitle("Find Widget");
}

void FindWidget::setContainer(ContainerObject *pContainer)
{
    mpContainer = pContainer;
}

void FindWidget::find()
{
    switch(mpFindWhatComboBox->currentIndex()) {
    case 0: //Component
        findComponent(mpFindLineEdit->text());
        break;
    case 1: //System Parameter
        findSystemParameter(mpFindLineEdit->text());
        break;
    case 2: //Alias
        findAlias(mpFindLineEdit->text());
        break;
    }
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
    QStringList sl {rName};
    findSystemParameter(sl, centerView);
}

void FindWidget::findSystemParameter(const QStringList &rNames, const bool centerView)
{
    if (mpContainer)
    {
        clearHighlights();

        QList<QRegExp> res;
        for (auto &name : rNames)
        {
            res.append(QRegExp(name, Qt::CaseInsensitive));
        }

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
                QString expression = removeAllSpaces(par.mValue);

                // OK, I cant figure out how to write the regexp to solve this, so I am splitting the string instead
                QStringList parts;
                splitOnAny(expression,{"+","-","*","/","(",")","^"}, parts);
                for (QString &part : parts)
                {
                    for (auto &re : res)
                    {
                        if (re.exactMatch(part))
                        {
                            hasPar = true;
                            break;
                        }
                    }
                    if (hasPar)
                    {
                        break;
                    }
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

void FindWidget::setVisible(bool visible)
{
    QWidget::setVisible(visible);
    if(visible)
    {
        this->mpFindLineEdit->setFocus();
    }
}

void FindWidget::keyPressEvent(QKeyEvent* event)
{
    if(event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)
    {
        this->find();
    }
    QWidget::keyPressEvent(event);
}

void FindWidget::clearHighlights()
{
    mpContainer->mpModelWidget->getGraphicsView()->clearHighlights();
}
