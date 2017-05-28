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
    setPalette(QPalette(QColor("gray"), QColor("whitesmoke")));

    QVBoxLayout *pLayout = new QVBoxLayout(this);
    QLabel *pGUIHeader = new QLabel(this);
    QLabel *pGplV3short = new QLabel(this);
    QLabel *pCoreHeader = new QLabel(this);
    QLabel *pCoreText = new QLabel(this);
    QLabel *pDepHeader = new QLabel(this);
    QLabel *pDependencies = new QLabel(this);

    QString gplv3short;
    QFile gplv3short_file(":/license/licenseHeaderGPL3");
    if (gplv3short_file.exists()) {
        gplv3short_file.open(QIODevice::ReadOnly);
        QTextStream ts(&gplv3short_file);
        // Read line by line and remove the comment lines
        while (!ts.atEnd()) {
            auto line = ts.readLine();
            if (!(line.startsWith("/*") || line.endsWith("*/"))) {
                gplv3short.append("        "+line+"\n");
            }
        }
        gplv3short_file.close();
    }

    QFont bigFont = pGUIHeader->font();
    bigFont.setPointSizeF(bigFont.pointSizeF()*2.0);

    pGUIHeader->setText("This application, HopsanGUI, is released under the\nGPL version 3 license."); ;
    pGUIHeader->setFont(bigFont);
    //pLicenseText->setAlignment(Qt::AlignHCenter);
    pGplV3short->setText(gplv3short);

    pCoreHeader->setText("The simulation library, HopsanCore, is released\nunder the Apache License version 2.0.");
    pCoreHeader->setFont(bigFont);
    pCoreText->setText("Utility libraries and the default component library are also released under this license.\n");

    pDepHeader->setText("Dependencies:");
    pDepHeader->setFont(bigFont);
    QString deps;
    deps.append("Hopsan uses third-party dependencies released under various licenses such as:\n");
    deps.append("LGPL v2.1, LGPL v3.0, BSD, MIT and Apache License 2.0\n\n");
    deps.append("For details on third-party dependencies and their respective licenses, see the documentation!");
    pDependencies->setText(deps);

    pLayout->addWidget(pGUIHeader, 1);
    pLayout->addWidget(pGplV3short, 1);
    pLayout->addWidget(pCoreHeader, 1);
    pLayout->addWidget(pCoreText, 1);
    pLayout->addWidget(pDepHeader, 1);
    pLayout->addWidget(pDependencies, 1);
    pLayout->addWidget(new QLabel(this), 4);

    QPushButton *pDocsButton = new QPushButton("Show license documentation",  this);
    connect(pDocsButton, SIGNAL(clicked(bool)), this, SLOT(showLicenseDocs()));
    pLayout->addWidget(pDocsButton, 1);

    QCheckBox *pAlwaysShow = new QCheckBox("Always show on startup", this);
    pAlwaysShow->setChecked(gpConfig->getBoolSetting(CFG_SHOWLICENSEONSTARTUP));
    connect(pAlwaysShow, SIGNAL(clicked(bool)), this, SLOT(toggleAlwaysShow(bool)));
    pLayout->addWidget(pAlwaysShow, 1);

    setLayout(pLayout);
}

void LicenseDialog::toggleAlwaysShow(bool tf)
{
    gpConfig->setBoolSetting(CFG_SHOWLICENSEONSTARTUP, tf);
}

void LicenseDialog::showLicenseDocs()
{
    gpHelpDialog->open("page_hopsandependencies.html");
}
