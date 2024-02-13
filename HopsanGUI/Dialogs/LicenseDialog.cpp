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

#include "LicenseDialog.h"

#include <QVBoxLayout>
#include <QFont>
#include <QCheckBox>
#include <QPushButton>

#include "Utilities/GUIUtilities.h"
#include "HelpDialog.h"
#include "Configuration.h"
#include "DesktopHandler.h"
#include "global.h"

LicenseDialog::LicenseDialog(QWidget *pParent) :
    QDialog(pParent)
{
    setObjectName("LicenseDialog");
    resize(640,480);
    setWindowTitle(tr("Hopsan License"));
    setAttribute(Qt::WA_DeleteOnClose, true);

    QVBoxLayout *pLayout = new QVBoxLayout(this);
    QLabel *pGUIText = new QLabel(this);
    QLabel *pGPLv3Header = new QLabel(this);
    QLabel *pGPLv3Notice = new QLabel(this);
    QLabel *pALv2Notice = new QLabel(this);
    QLabel *pALv2Header = new QLabel(this);
    QLabel *pCoreText = new QLabel(this);
    QLabel *pDepHeader = new QLabel(this);
    QLabel *pDepText = new QLabel(this);

    QString gplv3_notice;
    QFile gplv3_notice_file(":/license/licenseNoticeGPLv3");
    if (gplv3_notice_file.exists()) {
        gplv3_notice_file.open(QIODevice::ReadOnly);
        QTextStream ts(&gplv3_notice_file);
        gplv3_notice = ts.readAll();
        gplv3_notice_file.close();
    }

    QString alv2_notice;
    QFile al2_notice_file(":/license/licenseNoticeALv2");
    if (al2_notice_file.exists()) {
        al2_notice_file.open(QIODevice::ReadOnly);
        QTextStream ts(&al2_notice_file);
        alv2_notice = ts.readAll();
        al2_notice_file.close();
    }

    QFont bigFont = pGUIText->font();
    QFont mediumFont = pGUIText->font();
    mediumFont.setPointSizeF(mediumFont.pointSizeF()*1.2);
    bigFont.setPointSizeF(bigFont.pointSizeF()*2.0);

    pGUIText->setText("This application, HopsanGUI, is released under the GPL version 3 license.");
    pCoreText->setText("The HopsanCore simulation library, default component library and utility function libraries are released under the\nApache License version 2.0.");
    pGUIText->setFont(mediumFont);
    pCoreText->setFont(mediumFont);

    QWidget *pNoticeBox = new QWidget(this);
    QGridLayout *pNoticeLayout = new QGridLayout(pNoticeBox);
    pNoticeLayout->setColumnMinimumWidth(1,30);

    pGPLv3Header->setText("GPL version 3");
    pGPLv3Header->setFont(bigFont);
    pGPLv3Notice->setText(gplv3_notice);
    pNoticeLayout->addWidget(pGPLv3Header,0,0,1,1);
    pNoticeLayout->addWidget(pGPLv3Notice,1,0,1,1);

    pALv2Header->setText("Apache License version 2.0");
    pALv2Header->setFont(bigFont);
    pALv2Notice->setText(alv2_notice);
    pNoticeLayout->addWidget(pALv2Header,0,2,1,1);
    pNoticeLayout->addWidget(pALv2Notice,1,2,1,1);

    pDepHeader->setText("Dependencies:");
    pDepHeader->setFont(bigFont);
    QString deps;
    deps.append("Hopsan uses third-party dependencies released under various licenses such as:\n");
    deps.append("LGPL v2.1, LGPL v3.0, BSD, MIT and Apache License 2.0\n\n");
    deps.append("For details on third-party dependencies and their respective licenses, see the documentation!");
    pDepText->setText(deps);

    pLayout->addWidget(pGUIText, 1);
    pLayout->addWidget(pCoreText, 1);
    pLayout->addWidget(pNoticeBox);

    pLayout->addWidget(pDepHeader, 1);
    pLayout->addWidget(pDepText, 1);
    pLayout->addWidget(new QLabel(this), 4);

    QPushButton *pDocsButton = new QPushButton("Show license documentation",  this);
    connect(pDocsButton, SIGNAL(clicked(bool)), this, SLOT(showLicenseDocs()));
    pLayout->addWidget(pDocsButton, 1);

    QCheckBox *pAlwaysShow = new QCheckBox("Always show on startup", this);
    pAlwaysShow->setChecked(gpConfig->getBoolSetting(cfg::showlicenseonstartup));
    connect(pAlwaysShow, SIGNAL(clicked(bool)), this, SLOT(toggleAlwaysShow(bool)));

    QPushButton *pCloseButton = new QPushButton("Close", this);
    pCloseButton->setDefault(true);
    connect(pCloseButton, SIGNAL(clicked(bool)), this, SLOT(close()));

    QHBoxLayout *pBottomHLayout = new QHBoxLayout();
    pBottomHLayout->addWidget(pAlwaysShow, 2);
    pBottomHLayout->addWidget(pCloseButton, 1);
    pLayout->addLayout(pBottomHLayout, 1);

    setLayout(pLayout);
}

void LicenseDialog::toggleAlwaysShow(bool tf)
{
    gpConfig->setBoolSetting(cfg::showlicenseonstartup, tf);
}

void LicenseDialog::showLicenseDocs()
{
    gpHelpDialog->open("page_hopsandependencies.html");
}
