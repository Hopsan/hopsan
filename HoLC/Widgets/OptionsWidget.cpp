#include <QGridLayout>
#include <QToolButton>
#include <QFileDialog>
#include <QDebug>
#include <QProcessEnvironment>
#include <QGroupBox>
#include "OptionsWidget.h"
#include "Configuration.h"

OptionsWidget::OptionsWidget(Configuration *pConfiguration, QWidget *parent) :
    QWidget(parent)
{
    mpConfiguration = pConfiguration;

//    QFont font = this->font();
//    font.setBold(true);
//    QLabel *pOptionsLabel = new QLabel("Options", this);
//    pOptionsLabel->setFont(font);

    QLabel *pHopsanDirLabel = new QLabel("Hopsan Path:");
    mpHopsanDirLineEdit = new QLineEdit(this);
    QToolButton *pHopsanDirButton = new QToolButton(this);
    pHopsanDirButton->setIcon(QIcon(":graphics/uiicons/Hopsan-Open.png"));

    QLabel *pLibraryLabel = new QLabel("Library File:");
    mpLibraryLineEdit = new QLineEdit(this);
    mpLibraryLineEdit->setDisabled(true);

    QLabel *pIncludeLabel = new QLabel("Include Directory:");
    mpIncludeLineEdit = new QLineEdit(this);
    mpIncludeLineEdit->setDisabled(true);

    mpWarningLabel = new QLabel(this);
    mpWarningLabel->setText("<font color='red'>Warning! Library file or include files not found in specified directory!</font>");

    QLabel *pCompilerLabel = new QLabel("Compiler Path:");
    mpCompilerLineEdit = new QLineEdit(this);
    QToolButton *pCompilerButton = new QToolButton(this);
    pCompilerButton->setIcon(QIcon(":graphics/uiicons/Hopsan-Open.png"));

    mpCompilerWarningLabel = new QLabel(this);
    mpCompilerWarningLabel->setText("<font color='red'>Warning! GCC compiler not found in specified location!</font>");

    mpAlwaysSaveBeforeCompilingCheckBox = new QCheckBox("Always save all files before compiling", this);
    mpAlwaysSaveBeforeCompilingCheckBox->setChecked(mpConfiguration->getAlwaysSaveBeforeCompiling());

    QGroupBox *pCompilationOptionsBox = new QGroupBox("Compilation Options", this);
    QGridLayout *pCompilationOptionsLayout = new QGridLayout(pCompilationOptionsBox);

    mpUseTextWrappingCheckBox = new QCheckBox("Enable text wraping", this);
    mpUseTextWrappingCheckBox->setChecked(mpConfiguration->getUseTextWrapping());

    QGroupBox *pEditorOptionsBox = new QGroupBox("Editor Options", this);
    QGridLayout *pEditorOptionsLayout = new QGridLayout(pEditorOptionsBox);

    //Setup layouts

    int row=-1;
    pCompilationOptionsLayout->addWidget(pHopsanDirLabel,                     ++row,0,1,1);
    pCompilationOptionsLayout->addWidget(mpHopsanDirLineEdit,                   row,1,1,1);
    pCompilationOptionsLayout->addWidget(pHopsanDirButton,                      row,2,1,1);
    pCompilationOptionsLayout->addWidget(pLibraryLabel,                       ++row,0,1,1);
    pCompilationOptionsLayout->addWidget(mpLibraryLineEdit,                     row,1,1,2);
    pCompilationOptionsLayout->addWidget(pIncludeLabel,                       ++row,0,1,1);
    pCompilationOptionsLayout->addWidget(mpIncludeLineEdit,                     row,1,1,2);
    pCompilationOptionsLayout->addWidget(mpWarningLabel,                      ++row,0,1,3);
    pCompilationOptionsLayout->addWidget(pCompilerLabel,                      ++row,0,1,1);
    pCompilationOptionsLayout->addWidget(mpCompilerLineEdit,                    row,1,1,1);
    pCompilationOptionsLayout->addWidget(pCompilerButton,                       row,2,1,1);
    pCompilationOptionsLayout->addWidget(mpCompilerWarningLabel,              ++row,0,1,3);
    pCompilationOptionsLayout->addWidget(mpAlwaysSaveBeforeCompilingCheckBox, ++row,0,1,3);

    row=-1;
    pEditorOptionsLayout->addWidget(mpUseTextWrappingCheckBox,                ++row,0,1,1);

    QGridLayout *pLayout = new QGridLayout(this);
    pLayout->addWidget(pCompilationOptionsBox, 0,0);
    pLayout->addWidget(pEditorOptionsBox,       1,0);
    pLayout->addWidget(new QWidget(this),       2,0);
    pLayout->setRowStretch(2,1);

    //Setup connections
    connect(pHopsanDirButton, SIGNAL(clicked()), this, SLOT(setHopsanPath()));
    connect(pCompilerButton, SIGNAL(clicked()), this, SLOT(setCompilerPath()));
    connect(mpAlwaysSaveBeforeCompilingCheckBox, SIGNAL(toggled(bool)), mpConfiguration, SLOT(setAlwaysSaveBeforeCompiling(bool)));
    connect(mpUseTextWrappingCheckBox, SIGNAL(toggled(bool)), mpConfiguration, SLOT(setUseTextWrapping(bool)));
    connect(mpHopsanDirLineEdit, SIGNAL(textChanged(QString)), this, SLOT(setHopsanPath(QString)));
    connect(mpCompilerLineEdit, SIGNAL(textChanged(QString)), this, SLOT(setCompilerPath(QString)));

    if(!mpConfiguration->getCompilerPath().isEmpty())
    {
        mpCompilerLineEdit->setText(mpConfiguration->getCompilerPath());
        mpCompilerWarningLabel->hide();
    }

    //Load paths from configuration
    setCompilerPath(mpConfiguration->getCompilerPath());
    setHopsanPath(mpConfiguration->getHopsanPath());
}

void OptionsWidget::setVisible(bool visible)
{
    mpAlwaysSaveBeforeCompilingCheckBox->setChecked(mpConfiguration->getAlwaysSaveBeforeCompiling());

    QWidget::setVisible(visible);
}


void OptionsWidget::setHopsanPath()
{
    QString path = QFileDialog::getExistingDirectory(this, "Set Hopsan Path:", mpConfiguration->getHopsanPath());

    if(path.isEmpty()) return;

    setHopsanPath(path);
}

void OptionsWidget::setHopsanPath(const QString &path)
{
    bool success = true;
    QString lib, includeDir;
#ifdef WIN32
    lib = path+"/bin/HopsanCore.dll";
#else
    lib = path+"/bin/libHopsanCore.so";
#endif
    if(!QFile::exists(lib))
    {
        success = false;
    }
    includeDir = path+"/HopsanCore/include";
    if(!QFile::exists(includeDir+"/HopsanCore.h"))
    {
        success = false;
    }

    if(!mpHopsanDirLineEdit->hasFocus())
    {
        disconnect(mpHopsanDirLineEdit, SIGNAL(textChanged(QString)), this, SLOT(setHopsanPath(QString)));
        mpHopsanDirLineEdit->setText(path);
        connect(mpHopsanDirLineEdit, SIGNAL(textChanged(QString)), this, SLOT(setHopsanPath(QString)));
    }
    mpConfiguration->setHopsanPath(path);
    if(success)
    {
        mpLibraryLineEdit->setText(lib);
        mpIncludeLineEdit->setText(includeDir);
        mpWarningLabel->setVisible(false);
    }
    else
    {
        mpLibraryLineEdit->clear();
        mpIncludeLineEdit->clear();
        mpWarningLabel->setVisible(true);
    }
}

void OptionsWidget::setCompilerPath()
{
    QString path = QFileDialog::getExistingDirectory(this, "Set Compiler Path:", mpConfiguration->getCompilerPath());

    if(path.isEmpty()) return;

    setCompilerPath(path);
}

void OptionsWidget::setCompilerPath(QString path)
{
    QString filePath;
#ifdef linux
    if(path.endsWith("/gcc"))
        path.chop(4);
    filePath = path+"/gcc";
#else
    if(path.endsWith("/g++.exe") || path.endsWith("\\g++.exe"))
        path.chop(8);
    filePath = path+"/g++.exe";
#endif
    //! @todo We should also check that it is the correct version of gcc!

    mpConfiguration->setCompilerPath(path);
    if(QFile::exists(filePath))
    {
        //mpOptionsHandler->setCompilerPath(filePath);
        if(!mpCompilerLineEdit->hasFocus())
        {
            disconnect(mpCompilerLineEdit, SIGNAL(textChanged(QString)), this, SLOT(setCompilerPath(QString)));
            mpCompilerLineEdit->setText(filePath);
            connect(mpCompilerLineEdit, SIGNAL(textChanged(QString)), this, SLOT(setCompilerPath(QString)));
        }
        mpCompilerWarningLabel->hide();
    }
    else
    {
        //mpOptionsHandler->setCompilerPath("");
        if(!mpCompilerLineEdit->hasFocus())
        {
            disconnect(mpCompilerLineEdit, SIGNAL(textChanged(QString)), this, SLOT(setCompilerPath(QString)));
            mpCompilerLineEdit->setText(filePath);
            connect(mpCompilerLineEdit, SIGNAL(textChanged(QString)), this, SLOT(setCompilerPath(QString)));
        }
        mpCompilerWarningLabel->show();
    }
}
