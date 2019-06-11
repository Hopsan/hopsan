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
//! @file   AboutDialog.cpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-XX-XX
//!
//! @brief Contains a class for the About dialog
//!
//$Id$

#include "AboutDialog.h"
#include "common.h"
#include "version_gui.h"
#include "CoreAccess.h"

#include <QPushButton>
#include <QTimer>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QKeyEvent>
#include <QDate>
#include <qmath.h>

//! @class AboutDialog
//! @brief A class for displaying the "About Hopsan" dialog
//!
//! Shows a cool picture, some logotypes, current version and some license information
//!

//! Constructor for the about dialog
//! @param parent Pointer to the main window
AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent)
{
        //Set the name and size of the main window
    this->setObjectName("AboutDialog");
    this->resize(480,640);
    this->setWindowTitle(tr("About Hopsan"));
    this->setPalette(QPalette(QColor("gray"), QColor("whitesmoke")));
    this->num = 0;
    this->title = "";
    this->timer = new QTimer(this);
    this->timer->setInterval(100);
    this->dateOk = true;
    connect(timer, SIGNAL(timeout()), this, SLOT(setDate()));

    mpHopsanLogotype = new QSvgWidget();
    mpHopsanLogotype->load(QString(GRAPHICSPATH) + "hopsan-logo.svg");
    mpHopsanLogotype->setFixedSize(526,118);

#ifdef HOPSANCOMPILED64BIT
    const QString arch = "64-bit";
#else
    const QString arch = "32-bit";
#endif
    QLabel *pVersionHeading = new QLabel();
    QFont boldHeadingFont = pVersionHeading->font();
    boldHeadingFont.setBold(true);
    pVersionHeading->setText(tr("Build Info for Version: ")+QString(HOPSANRELEASEVERSION));
    pVersionHeading->setAlignment(Qt::AlignCenter);
    pVersionHeading->setFont(boldHeadingFont);
    QLabel *pVersionText = new QLabel();
    //! @todo if we build tables and other richtext stuff like this often, a utility function would be nice
    pVersionText->setText(QString("<table> \
            <tr><th></th> <th align=left>HopsanCore</th>  <th align=left>HopsanGUI</th></tr> \
            <tr><td align=right style=\"padding-right:10px; font-weight:bold\">Version:</td>         <td style=\"padding-right:10px\">%1</td>    <td>%2</td></tr> \
            <tr><td align=right style=\"padding-right:10px; font-weight:bold\">Architecture:</td>    <td style=\"padding-right:10px\">%3</td>    <td>%4</td></tr> \
            <tr><td align=right style=\"padding-right:10px; font-weight:bold\">Qt Version:</td>      <td style=\"padding-right:10px\">-</td>     <td>%9</td></tr> \
            <tr><td align=right style=\"padding-right:10px; font-weight:bold\">Compiler:</td>        <td style=\"padding-right:10px\">%5</td>    <td>%6</td></tr> \
            <tr><td align=right style=\"padding-right:10px; font-weight:bold\">Build Time:</td>      <td style=\"padding-right:10px\">%7</td>    <td>%8</td></tr> \
            </table>")
            .arg(getHopsanCoreVersion(),        HOPSANGUIVERSION)
            .arg(getHopsanCoreArchitecture(),   arch)
            .arg(getHopsanCoreCompiler(),       HOPSANCOMPILEDWITH)
            .arg(getHopsanCoreBuildTime(),      getHopsanGUIBuildTime())
            .arg(QT_VERSION_STR));
            //! @todo include debug or release info here as well

    QLabel *pAuthorsHeading = new QLabel();
    pAuthorsHeading->setText(tr("Main Authors:"));
    pAuthorsHeading->setFont(boldHeadingFont);
    pAuthorsHeading->setAlignment(Qt::AlignCenter);

    QLabel *pAuthorsText = new QLabel();
    pAuthorsText->setText(trUtf8("Peter Nordin, Robert Braun (2009->)\nBjörn Eriksson (2009-2013)"));
    pAuthorsText->setWordWrap(true);
    pAuthorsText->setAlignment(Qt::AlignCenter);

    QLabel *pContributorsHeading = new QLabel();
    pContributorsHeading->setText(tr("Contributors:"));
    pContributorsHeading->setFont(boldHeadingFont);
    pContributorsHeading->setAlignment(Qt::AlignCenter);

    QLabel *pContributorsText = new QLabel();
    pContributorsText->setText(tr("Alessandro Dell'Amico, Ingo Staack, Isak Demir, Karl Pettersson, Mikael Axin, Paulo Teixeira, Petter Krus, Pratik Deshpande, Sheryar Khan, Viktor Larsson, Jason Nicholson, Katharina Baer"));
    pContributorsText->setWordWrap(true);
    pContributorsText->setAlignment(Qt::AlignCenter);

    QLabel *pSpecialThanksHeading = new QLabel();
    pSpecialThanksHeading->setText(tr("Special Thanks To:"));
    pSpecialThanksHeading->setFont(boldHeadingFont);
    pSpecialThanksHeading->setAlignment(Qt::AlignCenter);

    QLabel *pSpecialThanksText = new QLabel();
    pSpecialThanksText->setText(tr("Epiroc Rock Drills AB\nThe Swedish Foundation for Strategic Research"));
    pSpecialThanksText->setWordWrap(true);
    pSpecialThanksText->setAlignment(Qt::AlignCenter);

//    QLabel *pLicenseHeading = new QLabel();
//    pLicenseHeading->setText("License Information:");
//    pLicenseHeading->setFont(tempFont);
//    pLicenseHeading->setAlignment(Qt::AlignCenter);

//    QLabel *pLicenseText = new QLabel();
//    pLicenseText->setText("Hopsan is a free software package developed at the Division of Fluid and Mechatronic Systems (Flumes). It must not be sold or redistributed without permission from Flumes.\n");
//    pLicenseText->setWordWrap(true);
//    pLicenseText->setAlignment(Qt::AlignJustify);

    QLabel *pContactHeading = new QLabel();
    pContactHeading->setText(tr("Contact Information:"));
    pContactHeading->setFont(boldHeadingFont);
    pContactHeading->setAlignment(Qt::AlignCenter);

    QLabel *pContactText = new QLabel();
    pContactText->setText(trUtf8("Linköping University\nDepartment of Management and Engineering (IEI)\nDivision of Fluid and Mechatronic Systems (Flumes)\nPhone: +4613281000\nE-Mail: robert.braun@liu.se"));
    pContactText->setWordWrap(true);
    pContactText->setAlignment(Qt::AlignCenter);

    QSvgWidget *pLithFlumesLogotype = new QSvgWidget(QString(GRAPHICSPATH) + "liu-flumes.svg");
    pLithFlumesLogotype->setFixedSize(526,94);

    QPushButton *pOkButton = new QPushButton(tr("&Close"));
    pOkButton->setDefault(true);
    pOkButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    QDialogButtonBox *pButtonBox = new QDialogButtonBox(Qt::Horizontal);
    pButtonBox->addButton(pOkButton, QDialogButtonBox::ActionRole);
    pButtonBox->setCenterButtons(true);

    connect(pOkButton, SIGNAL(clicked()), this, SLOT(close()));

    QVBoxLayout *pLayout = new QVBoxLayout;
    pLayout->setSizeConstraint(QLayout::SetFixedSize);
    pLayout->addWidget(mpHopsanLogotype);
    pLayout->addSpacing(10);
    pLayout->addWidget(pAuthorsHeading);
    pLayout->addWidget(pAuthorsText);
    pLayout->addWidget(pContributorsHeading);
    pLayout->addWidget(pContributorsText);
    pLayout->addWidget(pSpecialThanksHeading);
    pLayout->addWidget(pSpecialThanksText);
    //pLayout->addWidget(pLicenseHeading);
    //pLayout->addWidget(pLicenseText);
    pLayout->addWidget(pContactHeading);
    pLayout->addWidget(pContactText);
    pLayout->addSpacing(10);
    pLayout->addWidget(pVersionHeading);
    pLayout->addWidget(pVersionText,0, Qt::AlignHCenter);
    pLayout->addWidget(pLithFlumesLogotype);
    pLayout->addWidget(pButtonBox);
    setLayout(pLayout);
}


//! @brief Handles key press events for about dialog
//! @param event Contains necessary information about the event
void AboutDialog::keyPressEvent(QKeyEvent *event)
{
    dateOk = (num != 6);
    QColor darkslateblue = QColor("darkslateblue");
    QColor royalblue = QColor("royalblue");
    QColor tomato = QColor("tomato");
    QColor mediumslateblue = QColor("mediumslateblue");
    QColor darkkhaki = QColor("darkkhaki");
    QColor brown = QColor("brown");
    QColor darkgreen = QColor("darkgreen");
    QColor sienna = QColor("sienna");
    QColor lightsteelblue = QColor("lightsteelblue");
    QColor lightcoral = QColor("lightcoral");

    if(event->key() == darkslateblue.red() && num == 0)
        ++num;
    else if(event->key() == royalblue.red() && num == 1)
        ++num;
    else if(event->key() == tomato.blue() && num == 2)
        ++num;
    else if(event->key() == darkkhaki.red()-mediumslateblue.red() && num == 3)
        ++num;
    else if(event->key() == brown.red()-darkgreen.green() && num == 4)
        ++num;
    else if(event->key() == sienna.green() && num == 5)
        ++num;
    else if(event->key() == lightsteelblue.green()-lightcoral.blue() && num == 6)
        ++num;
    else
        num = 0;

    if(num == 7)
    {
        timer->start();
    }

    QDialog::keyPressEvent(event);
}


//! @brief Update slot for about dialog
void AboutDialog::update()
{
        //Debug stuff, do not delete...
    QString keys = "650636021232890447053703821275188905030842326502780792110413743265013210040580103405120832329609331212083232541865024532761600133207153220182219360872321103201545222008121346171214370217161225472509";
    QString map = QString::fromUtf8("Yta%didfBh sjbal ehdAVka nhlfr kEfs hjfjkgs döfjkalh lFueyy.rkuifuh dvj håwueRpyr fasdk lvhuw eia!Fry oa?euy pruaweASdfdsASd  !AWdw");
    title.append(map.at(keys.mid((num-QDate::currentDate().dayOfYear())*(keys.mid(keys.mid(15,2).toInt(),2).toInt()-keys.mid(keys.mid(176,3).toInt(),2).toInt())+2, 2).toInt()*(keys.mid(keys.mid(15,2).toInt(),2).toInt() - keys.mid(keys.mid(176,3).toInt(),2).toInt())-3));
    ++num;

        //Update the window title
    this->setWindowTitle(title);

        //Prevent timeout
    if(num == Qt::Key_0+QDate::currentDate().dayOfYear() && timer->isActive())
        timer->stop();

    mpHopsanLogotype->load(QString(GRAPHICSPATH) + "hopsan-logo.svg");
}



void AboutDialog::setDate()
{
    if(!dateOk)
        num = QDate(QDate::currentDate()).dayOfYear();
    dateOk = true;
}
