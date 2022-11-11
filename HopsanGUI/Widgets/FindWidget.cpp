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
    mpReplaceLabel = new QLabel("Replace: ", this);
    mpReplaceLineEdit = new QLineEdit(this);
    mpFindWhatComboBox = new QComboBox(this);
    mpFindWhatComboBox->addItem("Component");
    mpFindWhatComboBox->addItem("System Parameter");
    mpFindWhatComboBox->addItem("Alias");
    mpFindButton = new QPushButton("Find", this);
    mpFindButton->setShortcut(QKeySequence(Qt::Key_Enter));
    mpPreviousButton = new QPushButton("Find Previous", this);
    mpPreviousButton->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Enter));
    mpNextButton = new QPushButton("Find Next", this);
    mpNextButton->setShortcut(QKeySequence(Qt::Key_Enter));
    mpReplaceButton = new QPushButton("Replace", this);
    mpReplaceAndFindButton = new QPushButton("Replace & Find", this);
    mpReplaceAllButton = new QPushButton("Replace All", this);
    QToolButton *pCloseButton = new QToolButton(this);
    mpCaseSensitivityCheckBox = new QCheckBox("Case Sensitive", this);
    mpWildcardCheckBox = new QCheckBox("Match Wildcards (*)", this);
    pCloseButton->setIcon(QIcon(":graphics/uiicons/svg/Hopsan-Discard.svg"));

    QGridLayout *pLayout = new QGridLayout(this);
    pLayout->addWidget(new QLabel("Find: ", this),      0, 0);
    pLayout->addWidget(mpFindLineEdit,                  0, 1, 1, 2);
    pLayout->addWidget(mpFindWhatComboBox,              0, 3);
    pLayout->addWidget(mpFindButton,                    0, 4);
    pLayout->addWidget(mpPreviousButton,                0, 3);
    pLayout->addWidget(mpNextButton,                    0, 4);
    pLayout->addWidget(pCloseButton,                    0, 5);
    pLayout->setColumnStretch(1,1);
    pLayout->addWidget(mpReplaceLabel,                  1, 0);
    pLayout->addWidget(mpReplaceLineEdit,               1, 1);
    pLayout->addWidget(mpReplaceButton,                 1, 2);
    pLayout->addWidget(mpReplaceAndFindButton,          1, 3);
    pLayout->addWidget(mpReplaceAllButton,              1, 4);
    pLayout->addWidget(mpCaseSensitivityCheckBox,       2, 0);
    pLayout->addWidget(mpWildcardCheckBox,              2, 1);

    connect(mpFindButton,           SIGNAL(clicked()),          this, SLOT(findInContainer()));
    connect(mpPreviousButton,       SIGNAL(clicked()),          this, SLOT(findPrevious()));
    connect(mpNextButton,           SIGNAL(clicked()),          this, SLOT(findNext()));
    connect(pCloseButton,           SIGNAL(clicked()),          this, SLOT(close()));
    connect(mpReplaceButton,        SIGNAL(clicked()),          this, SLOT(replace()));
    connect(mpReplaceAndFindButton, SIGNAL(clicked()),          this, SLOT(replaceAndFind()));
    connect(mpReplaceAllButton,     SIGNAL(clicked()),          this, SLOT(replaceAll()));
    connect(mpFindLineEdit,         SIGNAL(returnPressed()),    this, SLOT(findNext()));

    resize(600, height());
    setWindowTitle("Find Widget");
}

void FindWidget::setContainer(SystemObject *pContainer)
{
    mpContainer = pContainer;
    mpTextEditor = nullptr;
    mpFindWhatComboBox->setVisible(true);
    mpWildcardCheckBox->setVisible(true);
    mpPreviousButton->setVisible(false);
    mpNextButton->setVisible(false);
    mpFindButton->setVisible(true);
    mpReplaceLabel->setVisible(false);
    mpReplaceLineEdit->setVisible(false);
    mpReplaceButton->setVisible(false);
    mpReplaceAndFindButton->setVisible(false);
    mpReplaceAllButton->setVisible(false);
}

void FindWidget::setTextEditor(TextEditorWidget *pEditor)
{
    mpTextEditor = pEditor;
    mpContainer = nullptr;
    mpFindWhatComboBox->setVisible(false);
    mpWildcardCheckBox->setVisible(false);
    mpPreviousButton->setVisible(true);
    mpNextButton->setVisible(true);
    mpFindButton->setVisible(false);
    mpReplaceLabel->setVisible(true);
    mpReplaceLineEdit->setVisible(true);
    mpReplaceButton->setVisible(true);
    mpReplaceAndFindButton->setVisible(true);
    mpReplaceAllButton->setVisible(true);
}

void FindWidget::findPrevious()
{
    QTextDocument::FindFlags flags;
    flags |= QTextDocument::FindBackward;
    if(mpCaseSensitivityCheckBox->isChecked()) {
        flags |= QTextDocument::FindCaseSensitively;
    }
    mpTextEditor->find(mpFindLineEdit->text(), flags);
}

void FindWidget::findNext()
{
    QTextDocument::FindFlags flags;
    if(mpCaseSensitivityCheckBox->isChecked()) {
        flags |= QTextDocument::FindCaseSensitively;
    }
    mpTextEditor->find(mpFindLineEdit->text(), flags);
}

void FindWidget::findInContainer()
{
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
        for(QString comp : compNames) {
            bool match;
            if(wildcard) {
                QRegExp re(rName, caseSensitivity, QRegExp::Wildcard);
                match = re.exactMatch(comp);
            }
            else {
                match = comp.contains(rName, caseSensitivity);
            }

            if (match) {
                ModelObject *pMO = mpContainer->getModelObject(comp);
                if (pMO) {
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
        for(QString alias : aliasNames) {
            bool match;
            if(wildcard) {
                QRegExp re(rName, caseSensitivity, QRegExp::Wildcard);
                match = re.exactMatch(alias);
            }
            else {
                match = alias.contains(rName, caseSensitivity);
            }

            if (match) {
                QString fullName = mpContainer->getFullNameFromAlias(alias);
                QString comp, port, var;
                QStringList sysHierarchy;
                splitFullVariableName(fullName, sysHierarchy, comp, port, var);
                ModelObject *pMO = mpContainer->getModelObject(comp);
                //! @todo we should actually highlight the port also (and center on the port)
                if (pMO) {
                    ++nFound;
                    pMO->highlight();
                    mean += pMO->pos();
                }
            }
        }

        // Now center view over found model objects
        if (nFound > 0 && centerView) {
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
        for(ModelObject* pMO : mops) {
            bool hasPar = false;
            QVector<CoreParameterData> pars;
            pMO->getParameters(pars);
            for(CoreParameterData par : pars) {
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

void FindWidget::replace()
{
    if(mpTextEditor->getSelectedText() == mpFindLineEdit->text()) {
        mpTextEditor->replaceSelectedText(mpReplaceLineEdit->text());
    }
}

void FindWidget::replaceAndFind()
{
    if(mpTextEditor->getSelectedText() == mpFindLineEdit->text()) {
        mpTextEditor->replaceSelectedText(mpReplaceLineEdit->text());
    }
    QTextDocument::FindFlags flags;
    if(mpCaseSensitivityCheckBox->isChecked()) {
        flags |= QTextDocument::FindCaseSensitively;
    }
    mpTextEditor->find(mpFindLineEdit->text(), flags);
}

void FindWidget::replaceAll()
{
    //Search backward
    findPrevious();
    while(mpTextEditor->getSelectedText() == mpFindLineEdit->text()) {
        mpTextEditor->replaceSelectedText(mpReplaceLineEdit->text());
        findPrevious();
    }

    //Search forward
    findNext();
    while(mpTextEditor->getSelectedText() == mpFindLineEdit->text()) {
        mpTextEditor->replaceSelectedText(mpReplaceLineEdit->text());
        findNext();
    }

}

void FindWidget::setVisible(bool visible)
{
    if(mpTextEditor) {

        //If text is selected in current editor, update it in find widget
        QString selection = mpTextEditor->getSelectedText();
        if(!selection.contains("\u2029") && mpFindLineEdit->text() != selection && !selection.isEmpty()) {
            mpFindLineEdit->setText(selection);
            visible = true;
        }

        //If find widget does not have focus, give it focus instead of hiding it
        if(gpMainWindowWidget->focusWidget() != mpFindLineEdit) {
            visible = true;
        }
    }

    QWidget::setVisible(visible);
    if(visible)
    {
        this->mpFindLineEdit->setFocus();
        this->mpFindLineEdit->selectAll();
    }
}

void FindWidget::keyPressEvent(QKeyEvent* event)
{
    if(event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)
    {
        this->findInContainer();
    }
    QWidget::keyPressEvent(event);
}

void FindWidget::clearHighlights()
{
    mpContainer->mpModelWidget->getGraphicsView()->clearHighlights();
}
