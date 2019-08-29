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
#include "Widgets/TextEditorWidget.h"
#include "ModelHandler.h"
#include "global.h"

#include <QLineEdit>
#include <QLabel>
#include <QToolButton>
#include <QPushButton>
#include <QComboBox>
#include <QHBoxLayout>
#include <QVBoxLayout>

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
    mpCaseSensitivityCheckBox = new QCheckBox("Case Sensitive", this);
    mpWildcardCheckBox = new QCheckBox("Match Wildcards (*)", this);
    mpBackwardsCheckBox = new QCheckBox("Search Backwards", this);
    pCloseButton->setIcon(QIcon(":graphics/uiicons/svg/Hopsan-Discard.svg"));

    QVBoxLayout *pMainLayout = new QVBoxLayout(this);
    QHBoxLayout *pSubLayout1 = new QHBoxLayout(this);
    QHBoxLayout *pSubLayout2 = new QHBoxLayout(this);
    pMainLayout->addLayout(pSubLayout1);
    pMainLayout->addLayout(pSubLayout2);
    pSubLayout1->addWidget(new QLabel("Find: ", this));
    pSubLayout1->addWidget(mpFindLineEdit);
    pSubLayout1->addWidget(mpFindWhatComboBox);
    pSubLayout1->addWidget(mpFindButton);
    pSubLayout1->addWidget(pCloseButton);
    pSubLayout1->setStretch(1,1);
    pSubLayout2->addWidget(mpCaseSensitivityCheckBox);
    pSubLayout2->addWidget(mpWildcardCheckBox);
    pSubLayout2->addWidget(mpBackwardsCheckBox);
    pSubLayout2->addWidget(new QWidget(this), 1);
    connect(mpFindButton, SIGNAL(clicked()), this, SLOT(find()));
    connect(pCloseButton, SIGNAL(clicked()), this, SLOT(close()));

    resize(600, height());
    setWindowTitle("Find Widget");
}

void FindWidget::setContainer(ContainerObject *pContainer)
{
    mpContainer = pContainer;
    mpTextEditor = nullptr;
    mpFindWhatComboBox->setVisible(true);
    mpWildcardCheckBox->setVisible(true);
    mpBackwardsCheckBox->setVisible(false);
}

void FindWidget::setTextEditor(TextEditorWidget *pEditor)
{
    mpTextEditor = pEditor;
    mpContainer = nullptr;
    mpFindWhatComboBox->setVisible(false);
    mpWildcardCheckBox->setVisible(false);
    mpBackwardsCheckBox->setVisible(true);
}

void FindWidget::find()
{
    if(mpTextEditor) {
        QTextDocument::FindFlags flags;
        if(mpCaseSensitivityCheckBox->isChecked()) {
            flags |= QTextDocument::FindCaseSensitively;
        }
        if(mpBackwardsCheckBox->isChecked()) {
            flags |= QTextDocument::FindBackward;
        }

        mpTextEditor->find(mpFindLineEdit->text(), flags);
    }

    Qt::CaseSensitivity caseSensitivity = Qt::CaseInsensitive;
    if(mpCaseSensitivityCheckBox->isChecked())
        caseSensitivity = Qt::CaseSensitive;

    bool wildcard = mpWildcardCheckBox->isChecked();

    switch(mpFindWhatComboBox->currentIndex()) {
    case 0: //Component
        findComponent(mpFindLineEdit->text(), true, caseSensitivity, wildcard);
        break;
    case 1: //System Parameter
        findSystemParameter(mpFindLineEdit->text(), true, caseSensitivity, wildcard);
        break;
    case 2: //Alias
        findAlias(mpFindLineEdit->text(), true, caseSensitivity, wildcard);
        break;
    }
}

void FindWidget::findComponent(const QString &rName, const bool centerView, Qt::CaseSensitivity caseSensitivity, bool wildcard)
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
            bool match;
            if(wildcard)
            {
                QRegExp re(rName, caseSensitivity, QRegExp::Wildcard);
                match = re.exactMatch(comp);
            }
            else
            {
                match = comp.contains(rName, caseSensitivity);
            }

            if (match)
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

void FindWidget::findAlias(const QString &rName, const bool centerView, Qt::CaseSensitivity caseSensitivity, bool wildcard)
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
            bool match;
            if(wildcard)
            {
                QRegExp re(rName, caseSensitivity, QRegExp::Wildcard);
                match = re.exactMatch(alias);
            }
            else
            {
                match = alias.contains(rName, caseSensitivity);
            }

            if (match)
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

void FindWidget::findSystemParameter(const QString &rName, const bool centerView, Qt::CaseSensitivity caseSensitivity, bool wildcard)
{
    QStringList sl {rName};
    findSystemParameter(sl, centerView, caseSensitivity, wildcard);
}

void FindWidget::findSystemParameter(const QStringList &rNames, const bool centerView, Qt::CaseSensitivity caseSensitivity, bool wildcard)
{
    if (mpContainer)
    {
        clearHighlights();

        QList<QRegExp> res;
        for (auto &name : rNames)
        {
            res.append(QRegExp(name, caseSensitivity, QRegExp::Wildcard));
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
                    if(wildcard)
                    {
                        for (auto &re : res)
                        {
                            if (re.exactMatch(part))
                            {
                                hasPar = true;
                                break;
                            }
                        }
                    }
                    else
                    {
                        for (auto &name : rNames)
                        {
                            if(part.contains(name, caseSensitivity))
                            {
                                hasPar = true;
                                break;
                            }
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
    //If text is selected in current editor, update it in find widget
    if(mpTextEditor) {
        QString selection = mpTextEditor->getSelectedText();
        if(!selection.contains("\u2029") && mpFindLineEdit->text() != selection && !selection.isEmpty()) {
            mpFindLineEdit->setText(selection);
            visible = true;
        }
    }

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
