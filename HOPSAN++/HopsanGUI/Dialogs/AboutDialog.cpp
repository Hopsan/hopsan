//!
//! @file   OptionsDialog.cpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-XX-XX
//!
//! @brief Contains a class for the options dialog
//!
//$Id$

#include "AboutDialog.h"
#include "../MainWindow.h"
#include <QPixmap>
#include <QColor>

#include "../common.h"
#include "../version.h"


//! @class AboutDialog
//! @brief A class for displaying the "About HOPSAN" dialog
//!
//! Shows a cool picture, some logotypes, current version and some license information
//!

//! Constructor for the about dialog
//! @param parent Pointer to the main window
AboutDialog::AboutDialog(MainWindow *parent)
    : QDialog(parent)
{

        //Set the name and size of the main window
    this->setObjectName("AboutDialog");
    this->resize(480,640);
    this->setWindowTitle("About HOPSAN");
    this->setPalette(QPalette(QColor("gray"), QColor("whitesmoke")));
    this->num = 0;
    this->title = "";
    this->timer = new QTimer(this);
    this->timer->setInterval(100);
    this->dateOk = true;
    connect(timer, SIGNAL(timeout()), this, SLOT(setDate()));

    mpHopsanLogotype = new QLabel();
    QPixmap image;
    image.load(QString(GRAPHICSPATH) + "about.png");
    mpHopsanLogotype->setPixmap(image);
    mpHopsanLogotype->setAlignment(Qt::AlignCenter);

    QLabel *pVersionText = new QLabel();
    pVersionText->setText("\nHOPSAN GUI version " + QString(HOPSANGUIVERSION) + "\n");
    QFont tempFont = pVersionText->font();
    tempFont.setBold(true);
    pVersionText->setFont(tempFont);

    QLabel *pAuthorsHeading = new QLabel();
    pAuthorsHeading->setText("Main Authors:");
    pAuthorsHeading->setFont(tempFont);
    pAuthorsHeading->setAlignment(Qt::AlignCenter);

    QLabel *pAuthorsText = new QLabel();
    pAuthorsText->setText(QString::fromUtf8("Björn Eriksson, Peter Nordin, Robert Braun\n"));
    pAuthorsText->setWordWrap(true);
    pAuthorsText->setAlignment(Qt::AlignCenter);

    QLabel *pContributorsHeading = new QLabel();
    pContributorsHeading->setText("Contributors:");
    pContributorsHeading->setFont(tempFont);
    pContributorsHeading->setAlignment(Qt::AlignCenter);

    QLabel *pContributorsText = new QLabel();
    pContributorsText->setText("Alessandro Dell'Amico, Ingo Staack, Karl Pettersson, Mikael Axin, Petter Krus\n");
    pContributorsText->setWordWrap(true);
    pContributorsText->setAlignment(Qt::AlignCenter);

    QLabel *pSpecialThanksHeading = new QLabel();
    pSpecialThanksHeading->setText("Special Thanks To:");
    pSpecialThanksHeading->setFont(tempFont);
    pSpecialThanksHeading->setAlignment(Qt::AlignCenter);

    QLabel *pSpecialThanksText = new QLabel();
    pSpecialThanksText->setText("Atlas Copco\nThe Swedish Foundation for Strategic Research\n");
    pSpecialThanksText->setWordWrap(true);
    pSpecialThanksText->setAlignment(Qt::AlignCenter);

    QLabel *pLicenseHeading = new QLabel();
    pLicenseHeading->setText("License Information:");
    pLicenseHeading->setFont(tempFont);
    pLicenseHeading->setAlignment(Qt::AlignCenter);

    QLabel *pLicenseText = new QLabel();
    pLicenseText->setText("Bla bla bla very important license information and other nonsense stuff which I can think of to fill up this box so we can se if it looks fine or if it needs any nice improvements to look even better than it already does even though it does not contain any useful information about anything at all...\n");
    pLicenseText->setWordWrap(true);
    pLicenseText->setAlignment(Qt::AlignJustify);

    QLabel *pContactHeading = new QLabel();
    pContactHeading->setText("Contact Information:");
    pContactHeading->setFont(tempFont);
    pContactHeading->setAlignment(Qt::AlignCenter);

    QLabel *pContactText = new QLabel();
    pContactText->setText(QString::fromUtf8("Linköping University\nDepartment of Management and Engineering (IEI)\nDivision of Fluid and Mechatronic Systems (Flumes)\nPhone: +4613281000\nE-Mail: someone@liu.se"));
    pContactText->setWordWrap(true);
    pContactText->setAlignment(Qt::AlignCenter);

    QLabel *pLithFlumesLogotype = new QLabel();
    QPixmap bottomimage;
    bottomimage.load(QString(GRAPHICSPATH) + "flumeslith.png");
    pLithFlumesLogotype->setPixmap(bottomimage);

    QPushButton *pOkButton = new QPushButton(tr("&Close"));
    pOkButton->setAutoDefault(true);
    pOkButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    QDialogButtonBox *pButtonBox = new QDialogButtonBox(Qt::Horizontal);
    pButtonBox->addButton(pOkButton, QDialogButtonBox::ActionRole);
    pButtonBox->setCenterButtons(true);

    connect(pOkButton, SIGNAL(pressed()), this, SLOT(close()));

    QGridLayout *pLayout = new QGridLayout;
    pLayout->setSizeConstraint(QLayout::SetFixedSize);
    pLayout->addWidget(mpHopsanLogotype, 0, 0);
    pLayout->addWidget(pVersionText, 1, 0);
    pLayout->addWidget(pAuthorsHeading, 2, 0);
    pLayout->addWidget(pAuthorsText, 3, 0);
    pLayout->addWidget(pContributorsHeading, 4, 0);
    pLayout->addWidget(pContributorsText, 5, 0);
    pLayout->addWidget(pSpecialThanksHeading, 6, 0);
    pLayout->addWidget(pSpecialThanksText, 7, 0);
    pLayout->addWidget(pLicenseHeading, 8, 0);
    pLayout->addWidget(pLicenseText, 9, 0);
    pLayout->addWidget(pContactHeading, 10, 0);
    pLayout->addWidget(pContactText, 11, 0);
    pLayout->addWidget(pLithFlumesLogotype, 12, 0);
    pLayout->addWidget(pButtonBox, 13, 0);
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

    QPixmap image;
    image.load(QString(GRAPHICSPATH) + "about.png");
    mpHopsanLogotype->setPixmap(image.scaled(454-300*sin((num-QDate::currentDate().dayOfYear())*3.1415/Qt::Key_0), 110));
}



void AboutDialog::setDate()
{
    if(!dateOk)
        num = QDate(QDate::currentDate()).dayOfYear();
    dateOk = true;
}
