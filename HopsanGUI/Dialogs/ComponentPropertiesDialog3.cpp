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

//!
//! @file   ComponentPropertiesDialog3.cpp
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2010-03-01
//!
//! @brief Contains a dialog class for changing component properties
//!
//$Id$

//Qt includes
#include <QDebug>
#include <QMessageBox>
#include <QScrollArea>
#include <QApplication>
#include <QDesktopWidget>
#include <QTableView>
#include <QHeaderView>
#include <QScrollBar>
#include <QMenu>
#include <QWebView>
#include <QWebFrame>
#include <QMainWindow>
#include <QLineEdit>
#include <QGroupBox>
#include <QToolButton>


//Hopsan includes
#include "common.h"
#include "global.h"
#include "ComponentPropertiesDialog3.h"
#include "Configuration.h"
#include "DesktopHandler.h"
#include "Dialogs/EditComponentDialog.h"
#include "Dialogs/MovePortsDialog.h"
#include "GUIObjects/GUIComponent.h"
#include "GUIObjects/GUIContainerObject.h"
#include "GUIPort.h"
#include "UndoStack.h"
#include "Utilities/GUIUtilities.h"
#include "Utilities/HighlightingUtilities.h"
#include "LibraryHandler.h"
#include "Widgets/ModelWidget.h"
#include "Widgets/SystemParametersWidget.h"
#include "LibraryHandler.h"
#include "MessageHandler.h"

#ifdef USEDISCOUNT
extern "C" {
#include "mkdio.h"
}
#endif

//! @class ComponentPropertiesDialog3
//! @brief The ComponentPropertiesDialog3 class is a Widget used to interact with component parameters.
//!
//! It reads and writes parameters to the core components.
//!





//! @brief Constructor for the parameter dialog for components
//! @param pGUIComponent Pointer to the component
//! @param parent Pointer to the parent widget
ComponentPropertiesDialog3::ComponentPropertiesDialog3(ModelObject *pModelObject, QWidget *pParent)
    : QDialog(pParent)
{
    mpModelObject = pModelObject;

    if(mpModelObject->getTypeName() == MODELICATYPENAME && false)    //! @todo Temporarily disabled for Modelica experiments, DO NOT MERGE
    {
        this->hide();

        EditComponentDialog *pEditDialog = new EditComponentDialog("", EditComponentDialog::Modelica, gpMainWindowWidget);
        pEditDialog->exec();

        if(pEditDialog->result() == QDialog::Accepted)
        {
            CoreGeneratorAccess coreAccess;
            QString typeName = pEditDialog->getCode().section("model ", 1, 1).section(" ",0,0);
            QString dummy = gpDesktopHandler->getGeneratedComponentsPath();
            QString libPath = dummy+typeName+"/";
            QDir().mkpath(libPath);
            int solver = pEditDialog->getSolver();

            QFile moFile(libPath+typeName+".mo");
            moFile.open(QFile::WriteOnly | QFile::Truncate);
            moFile.write(pEditDialog->getCode().toUtf8());
            moFile.close();

            coreAccess.generateFromModelica(libPath+typeName+".mo", solver, true);
            gpLibraryHandler->loadLibrary(libPath);

            mpModelObject->getParentContainerObject()->replaceComponent(mpModelObject->getName(), typeName);
        }
        delete(pEditDialog);

        this->close();
        return;
    }
    else if(mpModelObject->getTypeName() == "CppComponent")
    {
        this->hide();

        EditComponentDialog *pEditDialog = new EditComponentDialog("", EditComponentDialog::Cpp, gpMainWindowWidget);
        pEditDialog->exec();

        if(pEditDialog->result() == QDialog::Accepted)
        {
            CoreGeneratorAccess coreAccess;
            QString typeName = pEditDialog->getCode().section("class ", 1, 1).section(" ",0,0);

            QString dummy = gpDesktopHandler->getGeneratedComponentsPath();
            QString libPath = dummy+typeName+"/";
            QDir().mkpath(libPath);

            QFile hppFile(libPath+typeName+".hpp");
            hppFile.open(QFile::WriteOnly | QFile::Truncate);
            hppFile.write(pEditDialog->getCode().toUtf8());
            hppFile.close();

            coreAccess.generateFromCpp(libPath+typeName+".hpp");
            coreAccess.generateLibrary(libPath, QStringList() << typeName+".hpp");
            coreAccess.compileComponentLibrary(libPath+typeName+"_lib.xml");
            gpLibraryHandler->loadLibrary(libPath+typeName+"_lib.xml");

            mpModelObject->getParentContainerObject()->replaceComponent(mpModelObject->getName(), typeName);
        }
        delete(pEditDialog);

        this->close();
        return;
    }

    this->setPalette(gpConfig->getPalette());
    setWindowTitle(tr("Component Properties"));
    createEditStuff();

    connect(this, SIGNAL(setLimitedModelEditingLock(bool)), mpModelObject->getParentContainerObject()->mpModelWidget, SLOT(lockModelEditingLimited(bool)));

    // Lock model for changes
    emit setLimitedModelEditingLock(true);
}


//! @brief Verifies that a parameter value does not begin with a number but still contains illegal characters.
//! @note This is a temporary solution. It shall be removed when parsing equations as parameters works.
bool VariableTableWidget::cleanAndVerifyParameterValue(QString &rValue, const QString type)
{
    //! @todo I think CORE should handle all of this stuff
    QString tempVal = rValue;
    QStringList sysParamNames = mpModelObject->getParentContainerObject()->getParameterNames();
    QString error;

    if(verifyParameterValue(tempVal, type, sysParamNames, error))
    {
        // Set corrected text
        rValue = tempVal;
        return true;
    }
    else
    {
        QMessageBox::critical(gpMainWindowWidget, "Error", error.append(" Resetting parameter value!"));
        return false;
    }
}

bool VariableTableWidget::setAliasName(const int row)
{
    // Skip separator rows
    if (columnSpan(row,0)>2)
    {
        return true;
    }

    QString alias = item(row,VariableTableWidget::Alias)->text();
    //! @todo since alias=empty means unregister we can not skip it, but there should be some check if a variable has changed, so that we do not need to go through set for all variables every freaking time
    QString name = item(row,VariableTableWidget::Name)->toolTip();
    QStringList parts = name.split("#");
    if (parts.size() == 2)
    {
        //! @todo look over this name stuff
        mpModelObject->getParentContainerObject()->setVariableAlias(mpModelObject->getName()+"#"+parts[0]+"#"+parts[1], alias);
    }
    return true;
}


//! @brief Reads the values from the dialog and writes them into the core component
void ComponentPropertiesDialog3::okPressed()
{
    setName();
    setAliasNames();
    if (setVariableValues())
    {
        close();
    }
}


void ComponentPropertiesDialog3::editPortPos()
{
    //! @todo who owns the dialog, is it ever removed?
    MovePortsDialog *dialog = new MovePortsDialog(mpModelObject->getAppearanceData(), mpModelObject->getLibraryAppearanceData(), mpModelObject->getParentContainerObject()->getGfxType());
    connect(dialog, SIGNAL(finished()), mpModelObject, SLOT(refreshExternalPortsAppearanceAndPosition()), Qt::UniqueConnection);
}

void ComponentPropertiesDialog3::copyToNewComponent()
{
    QString sourceCode = mpSourceCodeTextEdit->toPlainText();

    QDateTime time = QDateTime();
    uint t = time.currentDateTime().toTime_t();     //Number of milliseconds since 1970
    double rd = rand() / (double)RAND_MAX;
    int r = int(rd*1000000.0);                      //Random number between 0 and 1000000
    QString randomName = mpModelObject->getTypeName()+QString::number(t)+QString::number(r);


    sourceCode.replace("#ifdef "+mpModelObject->getTypeName().toUpper()+"_HPP_INCLUDED", "");
    sourceCode.replace("#define "+mpModelObject->getTypeName().toUpper()+"_HPP_INCLUDED", "");
    sourceCode.replace("#endif //  "+mpModelObject->getTypeName().toUpper()+"_HPP_INCLUDED", "");
    sourceCode.replace("class "+mpModelObject->getTypeName()+" :", "class "+randomName+" :");
    sourceCode.replace("return new "+mpModelObject->getTypeName()+"()", "return new "+randomName+"()");

    EditComponentDialog *pEditDialog = new EditComponentDialog(sourceCode, EditComponentDialog::Cpp, gpMainWindowWidget);

    pEditDialog->exec();

    if(pEditDialog->result() == QDialog::Accepted)
    {
        CoreGeneratorAccess coreAccess;
        QString typeName = pEditDialog->getCode().section("class ", 1, 1).section(" ",0,0);
        QString dummy = gpDesktopHandler->getGeneratedComponentsPath();
        QString libPath = dummy+typeName+"/";
        QDir().mkpath(libPath);

        QFile hppFile(libPath+typeName+".hpp");
        hppFile.open(QFile::WriteOnly | QFile::Truncate);
        hppFile.write(pEditDialog->getCode().toUtf8());
        hppFile.close();

        coreAccess.generateFromCpp(libPath+typeName+".hpp");
        coreAccess.generateLibrary(libPath, QStringList() << typeName+".hpp");
        coreAccess.compileComponentLibrary(libPath+typeName+"_lib.xml");
        gpLibraryHandler->loadLibrary(libPath+typeName+"_lib.xml");
        mpModelObject->getParentContainerObject()->replaceComponent(mpModelObject->getName(), typeName);
    }
    delete(pEditDialog);
}

void ComponentPropertiesDialog3::recompile()
{
    QString basePath = this->mpModelObject->getAppearanceData()->getBasePath();
    QString fileName = this->mpModelObject->getAppearanceData()->getSourceCodeFile();

    QString sourceCode = mpSourceCodeTextEdit->toPlainText();
    int solver = mpSolverComboBox->currentIndex();

    //Read source code from file
    QFile oldSourceFile(basePath+fileName);
    if(!oldSourceFile.open(QIODevice::ReadOnly | QIODevice::Text))
        return;
    QString oldSourceCode;
    while(!oldSourceFile.atEnd())
    {
        oldSourceCode.append(oldSourceFile.readLine());
    }
    oldSourceFile.close();

    QFile sourceFile(basePath+fileName);
    if(!sourceFile.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text))
        return;
    sourceFile.write(sourceCode.toStdString().c_str());
    sourceFile.close();

    //Recompile with new code
    LibraryEntry entry = gpLibraryHandler->getEntry(mpModelObject->getTypeName());
    if (!entry.isNull())
    {
        gpLibraryHandler->recompileLibrary(entry.pLibrary, true, solver);
    }

    this->close();
}

bool ComponentPropertiesDialog3::setAliasNames()
{
    return mpVariableTableWidget->setAliasNames();
}


//! @brief Sets the parameters and start values in the core component. Read the values from the dialog and write them into the core component.
bool ComponentPropertiesDialog3::setVariableValues()
{
    bool isOK = mpVariableTableWidget->setStartValues();
    //isOK *= mpVariableTableWidget->setCustomPlotScaleValues();
    return isOK;
}

void ComponentPropertiesDialog3::setName()
{
    mpModelObject->getParentContainerObject()->renameModelObject(mpModelObject->getName(), mpNameEdit->text());
}

void ComponentPropertiesDialog3::closeEvent(QCloseEvent* event)
{
    emit setLimitedModelEditingLock(false);
    QWidget::closeEvent(event);
}

void ComponentPropertiesDialog3::reject()
{
    QDialog::reject();
    QDialog::close();
}

void ComponentPropertiesDialog3::openDescription()
{
    QMainWindow *pDescriptionWindow = new QMainWindow(gpMainWindowWidget);
    pDescriptionWindow->setCentralWidget(createHelpWidget());
    pDescriptionWindow->setWindowTitle(gpLibraryHandler->getModelObjectAppearancePtr(mpModelObject->getTypeName())->getDisplayName());
    pDescriptionWindow->resize(this->size());
    pDescriptionWindow->show();
}

QGridLayout* ComponentPropertiesDialog3::createNameAndTypeEdit()
{
    QGridLayout *pNameTypeLayout = new QGridLayout();
    QLabel *pNameLabel = new QLabel("Name: ");
    mpNameEdit = new QLineEdit(mpModelObject->getName());
    QLabel *pCQSTypeLabel = new QLabel("CQS Type: \""+mpModelObject->getTypeCQS()+"\"");
    QLabel *pTypeNameLabel = new QLabel("Typename: \""+mpModelObject->getTypeName()+"\"");
    int r=0;
    pNameTypeLayout->addWidget(pNameLabel,r,0);
    pNameTypeLayout->addWidget(mpNameEdit,r,1);
    r++;
    pNameTypeLayout->addWidget(pTypeNameLabel,r,0,1,2);
    r++;
    if (!mpModelObject->getSubTypeName().isEmpty())
    {
        QLabel *pSubTypeNameLabel = new QLabel("SubTypename: \""+mpModelObject->getSubTypeName()+"\"");
        pNameTypeLayout->addWidget(pSubTypeNameLabel,r,0,1,2);
        r++;
    }
    pNameTypeLayout->addWidget(pCQSTypeLabel,r,0,1,2);

    return pNameTypeLayout;
}

QDialogButtonBox* ComponentPropertiesDialog3::createButtonBox()
{
    QPushButton *pEditPortPos = new QPushButton(tr("&Move ports"), this);
    QPushButton *pCancelButton = new QPushButton(tr("&Cancel"), this);
    QPushButton *pOkButton = new QPushButton(tr("&Ok"), this);
    pOkButton->setDefault(true);
    QDialogButtonBox *pButtonBox = new QDialogButtonBox(Qt::Vertical, this);
    pButtonBox->addButton(pOkButton, QDialogButtonBox::ActionRole);
    pButtonBox->addButton(pCancelButton, QDialogButtonBox::ActionRole);
    pButtonBox->addButton(pEditPortPos, QDialogButtonBox::ActionRole);
    connect(pOkButton, SIGNAL(clicked()), this, SLOT(okPressed()));
    connect(pCancelButton, SIGNAL(clicked()), this, SLOT(close()));
    connect(pEditPortPos, SIGNAL(clicked()), this, SLOT(editPortPos()));
    return pButtonBox;
}

QWidget *ComponentPropertiesDialog3::createHelpWidget()
{
    if(!mpModelObject->getHelpText().isEmpty() || !mpModelObject->getHelpPicture().isEmpty() || !mpModelObject->getHelpLinks().isEmpty() || !mpModelObject->getHelpHtmlPath().isEmpty())
    {
        QScrollArea *pHelpScrollArea = new QScrollArea();
        QGroupBox *pHelpWidget = new QGroupBox();
        QVBoxLayout *pHelpLayout = new QVBoxLayout(pHelpWidget);

        QLabel *pHelpHeading = new QLabel(gpLibraryHandler->getModelObjectAppearancePtr(mpModelObject->getTypeName())->getDisplayName());
        pHelpHeading->setAlignment(Qt::AlignLeft);
        QFont tempFont = pHelpHeading->font();
        tempFont.setPixelSize(16);
        tempFont.setBold(true);
        pHelpHeading->setFont(tempFont);
        pHelpLayout->addWidget(pHelpHeading);

        if (!mpModelObject->getHelpHtmlPath().isEmpty())
        {
            QWebView * pHtmlView = new QWebView();
            QString path = mpModelObject->getAppearanceData()->getBasePath() + mpModelObject->getHelpHtmlPath();
            if (path.endsWith(".md"))
            {
#ifdef USEDISCOUNT
                QFileInfo mdFi(path);

                QString htmlFilePath = mpModelObject->getHelpHtmlPath();
                htmlFilePath.truncate(htmlFilePath.size()-3);
                htmlFilePath.append(".html");
                htmlFilePath.prepend(gpDesktopHandler->getTempPath()+"html/");

                // Make sure temp dir exist
                QFileInfo fi(htmlFilePath);
                QDir().mkpath(fi.absolutePath());

                FILE *inFile = fopen(path.toStdString().c_str(), "r");
                FILE *outFile = fopen(htmlFilePath.toStdString().c_str(), "w");
                if (inFile && outFile)
                {
                    // Generate html from markdown
                    MMIOT *doc = mkd_in(inFile, 0);
                    markdown(doc, outFile, MKD_TABSTOP);
                    fclose(outFile);
                    fclose(inFile);
                    mkd_cleanup(doc);

                    // Now parse the html file for equations and images
                    QFile htmlFile(htmlFilePath);
                    htmlFile.open(QIODevice::ReadOnly);
                    QTextStream htmlStream(&htmlFile);
                    QString tmp_html = htmlStream.readAll();
                    htmlFile.close();

                    QMap<size_t, QString> equationMap;
                    QMap<size_t, QString> imageMap;
                    QTextStream inHtmlStream(&tmp_html);
                    size_t ln=0;
                    while(!inHtmlStream.atEnd())
                    {
                        //! @todo multiline ?
                        QString line = inHtmlStream.readLine().trimmed();
                        if (line.startsWith("<!---EQUATION"))
                        {
                            QString eq = line.mid(13, line.size()-18).trimmed();
                            //eq.replace("\\", "\\\\");
                            equationMap.insert(ln, QString("<div class=\"equation\" expr=\"%1\"></div>").arg(eq));
                        }
                        else if (line.startsWith("<p><img"))
                        {
                            size_t b = line.indexOf("src=")+4;
                            QString image = line.left(b+1)+"file:///"+mdFi.absolutePath()+"/"+line.right(line.size()-b-1);
                            imageMap.insert( ln, image );
                        }
                        ++ln;
                    }

                    htmlFile.open(QIODevice::WriteOnly);
                    htmlStream.seek(0);

                    if (!equationMap.empty())
                    {
                        htmlStream << "<!DOCTYPE html>\n";
                        htmlStream << "<html>\n";
                        htmlStream << "  <head>\n";
                        htmlStream << "    <meta charset=\"UTF-8\">\n";
                        htmlStream << "      " << QString("<script src=\"file:///%1katex/katex.min.js\" type=\"text/javascript\"></script>\n").arg(gpDesktopHandler->getExecPath()+"../Dependencies/");
                        htmlStream << "      " << QString("<link href=\"file:///%1katex/katex.min.css\" rel=\"stylesheet\" type=\"text/css\">\n").arg(gpDesktopHandler->getExecPath()+"../Dependencies/");
                        htmlStream << "  </head>\n";

                        inHtmlStream.seek(0);
                        ln=0;
                        while (!inHtmlStream.atEnd())
                        {
                            // Check if image line should be replaced
                            auto imit = imageMap.find(ln);
                            if (imit != imageMap.end())
                            {
                                htmlStream << imit.value() << "\n";
                                imageMap.erase(imit);
                                // Also discard the actual line
                                inHtmlStream.readLine();
                            }
                            // If not then print the actual line
                            else
                            {
                                htmlStream << inHtmlStream.readLine() << "\n";
                            }

                            // Check if an equation should be added
                            auto eqit = equationMap.find(ln);
                            if (eqit != equationMap.end())
                            {
                                htmlStream << eqit.value() << "\n";
                                equationMap.erase(eqit);
                            }

                            ln++;
                        }

                        htmlStream << "  <script type=\"text/javascript\">\n";
                        htmlStream << "    var eqns = document.getElementsByClassName(\"equation\");\n";
                        htmlStream << "    Array.prototype.forEach.call(eqns, function(eq) {\n";
                        htmlStream << "      katex.render(eq.getAttribute(\"expr\"), eq, { displayMode: true });\n";
                        htmlStream << "    });\n";
                        htmlStream << "  </script>\n";

                        htmlStream << "</html>\n";

                        htmlFile.close();

                    }
                    else
                    {
                        // Check if image line should be replaced
                        auto imit = imageMap.find(ln);
                        if (imit != imageMap.end())
                        {
                            htmlStream << imit.value() << "\n";
                            imageMap.erase(imit);
                        }
                        // If not then print the actual line
                        else
                        {
                            htmlStream << inHtmlStream.readLine() << "\n";
                        }
                    }

                    // Set html to view
                    pHtmlView->load(QUrl::fromLocalFile(htmlFilePath));
                }
                else
                {
                    pHtmlView->setHtml(QString("<p>Error: Could not convert %1 to HTML or load the file.</p>").arg(path));
                }
#else
                pHtmlView->setHtml(QString("<p>Error: Markdown support is not available in this build!</p>").arg(path));
#endif
            }
            else
            {
                pHtmlView->load(QUrl::fromLocalFile(mpModelObject->getAppearanceData()->getBasePath() + mpModelObject->getHelpHtmlPath()));
            }
            pHelpLayout->addWidget(pHtmlView);
        }

        if(!mpModelObject->getHelpPicture().isEmpty())
        {
            QLabel *pHelpPicture = new QLabel();
            QPixmap helpPixMap(mpModelObject->getAppearanceData()->getBasePath() + mpModelObject->getHelpPicture());
            pHelpPicture->setPixmap(helpPixMap);
            pHelpLayout->addWidget(pHelpPicture);
            pHelpPicture->setAlignment(Qt::AlignLeft);
        }

        if(!mpModelObject->getHelpText().isEmpty())
        {
            QLabel *pHelpText = new QLabel(mpModelObject->getHelpText(), this);
            pHelpText->setWordWrap(true);
            pHelpLayout->addWidget(pHelpText);
            pHelpText->setAlignment(Qt::AlignLeft);
        }

        const QStringList &links = mpModelObject->getHelpLinks();
        for (auto &link : links)
        {
            QString linkstr = QString("External document: <a href=\"%1\">%2</a>").arg(mpModelObject->getAppearanceData()->getBasePath()+link).arg(link);
            QLabel *pHelpLink = new QLabel(linkstr,this);
            pHelpLink->setOpenExternalLinks(true);
            pHelpLayout->addWidget(pHelpLink);
            pHelpLink->setAlignment(Qt::AlignLeft);
        }

        // Add dummy stretch widget at bottom if no html is pressent (to force image / text / link together)
        if (mpModelObject->getHelpHtmlPath().isEmpty())
        {
            QWidget *pDummyWidget = new QWidget(this);
            pHelpLayout->addWidget(pDummyWidget, 1);
        }

        pHelpWidget->setStyleSheet(QString::fromUtf8("QGroupBox {background-color: white; border: 2px solid gray; border-radius: 5px;}"));

        pHelpScrollArea->setWidget(pHelpWidget);
        pHelpScrollArea->setWidgetResizable(true);
        pHelpScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        return pHelpScrollArea;
    }
    return 0;
}

QWidget *ComponentPropertiesDialog3::createSourcodeBrowser(QString &rFilePath)
{
    rFilePath.prepend(mpModelObject->getAppearanceData()->getBasePath());
    QFile file(rFilePath);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QString code;
    QTextStream t(&file);
    code = t.readAll();
    file.close();

    mpSourceCodeTextEdit = new QTextEdit();
    mpSourceCodeTextEdit->setReadOnly(false);
    mpSourceCodeTextEdit->setText(code);
    if(rFilePath.endsWith(".hpp"))
    {
        //! @todo who own and who deletes
        CppHighlighter *pHighLighter = new CppHighlighter(mpSourceCodeTextEdit->document());
        Q_UNUSED(pHighLighter);
    }
    else if(rFilePath.endsWith(".mo"))
    {
        ModelicaHighlighter *pHighLighter = new ModelicaHighlighter(mpSourceCodeTextEdit->document());
        Q_UNUSED(pHighLighter);
    }

    QWidget *pTempWidget = new QWidget(this);
    QVBoxLayout *pLayout = new QVBoxLayout(pTempWidget);
    pLayout->addWidget(mpSourceCodeTextEdit);

    QLabel *pSolverLabel = new QLabel("Solver: ", this);
    mpSolverComboBox = new QComboBox(this);
    mpSolverComboBox->addItem("Numerical Integration");
    mpSolverComboBox->addItem("Bilinear Transform");
    mpNewComponentButton = new QPushButton(tr("&Copy to new component"), this);
    connect(mpNewComponentButton, SIGNAL(clicked()), this, SLOT(copyToNewComponent()));
    mpRecompileButton = new QPushButton(tr("&Recompile"), this);
    mpRecompileButton->setEnabled(true);
    connect(mpRecompileButton, SIGNAL(clicked()), this, SLOT(recompile()));
    QHBoxLayout *pSolverLayout = new QHBoxLayout();
    pSolverLayout->addWidget(pSolverLabel);
    pSolverLayout->addWidget(mpSolverComboBox);
    pSolverLayout->addWidget(new QWidget(this));
    pSolverLayout->addWidget(mpNewComponentButton);
    pSolverLayout->addWidget(mpRecompileButton);
    pSolverLayout->setStretch(2,1);
    pLayout->addLayout(pSolverLayout);

    return pTempWidget;//return pSourceCodeTextEdit;
}

void ComponentPropertiesDialog3::createEditStuff()
{
    QGridLayout* pPropertiesLayout = new QGridLayout(this);
    int row=0;

    // Parents to new objects bellow should be set automatically when adding layout or widget to other layout or widget

    // Add help picture and text
    //------------------------------------------------------------------------------------------------------------------------------
    QWidget *pHelpWidget = createHelpWidget();
    //------------------------------------------------------------------------------------------------------------------------------

    // Add name edit and type information
    //------------------------------------------------------------------------------------------------------------------------------
    QGridLayout *pNameTypeLayout = createNameAndTypeEdit();
    pPropertiesLayout->addLayout(pNameTypeLayout, row, 0, Qt::AlignLeft);
    pPropertiesLayout->setRowStretch(row,0);
    //------------------------------------------------------------------------------------------------------------------------------

    // Add button widget with description and copy to clipboard buttons
    //------------------------------------------------------------------------------------------------------------------------------
    QWidget *pDCButtonWidget = new QWidget(this);
    QVBoxLayout *pDCButtonsLayout = new QVBoxLayout(pDCButtonWidget);
    QPushButton *pDescriptionButton = new QPushButton(tr("&Description"), this);
    pDescriptionButton->setToolTip("Open description in separate window");
    connect(pDescriptionButton, SIGNAL(clicked()), this, SLOT(openDescription()));
    pDCButtonsLayout->addWidget(pDescriptionButton);
    pPropertiesLayout->addWidget(pDCButtonWidget, row, 1);
    //------------------------------------------------------------------------------------------------------------------------------


    // Add button box with buttons
    //------------------------------------------------------------------------------------------------------------------------------
    QDialogButtonBox *pButtonBox = createButtonBox();
    pPropertiesLayout->addWidget(pButtonBox, row, 2);
    //------------------------------------------------------------------------------------------------------------------------------
    ++row;

    // Add Parameter settings table
    //------------------------------------------------------------------------------------------------------------------------------
    mpVariableTableWidget = new VariableTableWidget(mpModelObject,this);
    mpVariableTableWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    pPropertiesLayout->addWidget(mpVariableTableWidget, row, 0, 1, 3);
    pPropertiesLayout->setRowStretch(row,1);
    qDebug() << "Table: " << mpVariableTableWidget->sizeHint() << "  " << mpVariableTableWidget->minimumWidth() << "  " << mpVariableTableWidget->minimumHeight();
    //------------------------------------------------------------------------------------------------------------------------------

    QWidget *pPropertiesWidget = new QWidget(this);
    pPropertiesWidget->setLayout(pPropertiesLayout);
    pPropertiesWidget->setPalette(gpConfig->getPalette());

    // Add Code edit stuff, A new tab in a new widget will be created
    //------------------------------------------------------------------------------------------------------------------------------
    QString filePath = mpModelObject->getAppearanceData()->getSourceCodeFile();

    QGridLayout *pMainLayout = new QGridLayout(this);

    QTabWidget *pTabWidget = new QTabWidget(this);
    pTabWidget->addTab(pPropertiesWidget, "Properties");
    pTabWidget->addTab(pHelpWidget, "Description");
    if(!filePath.isEmpty())
    {
        QWidget* pSourceBrowser = createSourcodeBrowser(filePath);
        pTabWidget->addTab(pSourceBrowser, "Source Code");
    }
    pMainLayout->addWidget(pTabWidget);

    //------------------------------------------------------------------------------------------------------------------------------

    setLayout(pMainLayout);

    // Avoid a dialog that is higher than the available space
    //! @todo this prevents fullscreen mode ,maybe limit should be fullscreen height
    int maxHeight = qApp->desktop()->screenGeometry().height()-100;
    this->setMaximumHeight(maxHeight);
}

VariableTableWidget::VariableTableWidget(ModelObject *pModelObject, QWidget *pParent) :
    TableWidgetTotalSize(pParent)
{
    mpModelObject = pModelObject;

    this->setColumnCount(NumCols);
    QStringList columnHeaders;
    columnHeaders.append("Name");
    columnHeaders.append("Alias");
    columnHeaders.append("Description");
    columnHeaders.append("Unit");
    columnHeaders.append("Value");
    columnHeaders.append("Quantity");
    columnHeaders.append("PlotSettings");
    columnHeaders.append("Port");
    this->setHorizontalHeaderLabels(columnHeaders);

    // Decide which parameters should be shown as Constants
    QVector<CoreParameterData> parameters;
    mpModelObject->getParameters(parameters);
    QVector<int> constantsIds;
    for (int i=0; i<parameters.size(); ++i)
    {
        if (!parameters[i].mName.contains("#"))
        {
            constantsIds.push_back(i);
        }
    }

    // Write the constants first
    int r=0;
    if (!constantsIds.isEmpty())
    {
        createSeparatorRow(r,"Constants", Constant);
        ++r;
    }
    for (int i=0; i<constantsIds.size(); ++i)
    {
        CoreVariameterDescription variameter;
        variameter.mName = parameters[constantsIds[i]].mName;

        if(mpModelObject->getAppearanceData()->isParameterHidden(variameter.mName))
            continue;

        //Don't add ports, parameters and defaults constants for modelica components
        if(mpModelObject->getTypeName() == MODELICATYPENAME && (variameter.mName == "ports" || variameter.mName == "parameters" || variameter.mName == "defaults"))
            continue;

        variameter.mDescription = parameters[constantsIds[i]].mDescription;
        variameter.mUnit = parameters[constantsIds[i]].mUnit;
        variameter.mQuantity = parameters[constantsIds[i]].mQuantity;
        variameter.mDataType = parameters[constantsIds[i]].mType;
        variameter.mConditions = parameters[constantsIds[i]].mConditions;
        createTableRow(r, variameter, Constant);
        ++r;
    }

    // Now fetch and write the input variables
    QVector<CoreVariameterDescription> variameters;
    mpModelObject->getVariameterDescriptions(variameters);

    // Write inputVariables
    const int inputVarSeparatorRow = r;
    for (int i=0; i<variameters.size(); ++i)
    {
        if(mpModelObject->getAppearanceData()->isParameterHidden(variameters[i].mPortName+"#"+variameters[i].mName))
            continue;

        if (variameters[i].mVariameterType == InputVaraiable)
        {
            createTableRow(r, variameters[i], InputVaraiable);
            ++r;
        }
    }
    // Insert the separator row if there were any input variables)
    if (r != inputVarSeparatorRow)
    {
        createSeparatorRow(inputVarSeparatorRow,"InputVariables", InputVaraiable);
        ++r;
    }

    // Write outputVariables
    const int outputVarSeparatorRow = r;
    for (int i=0; i<variameters.size(); ++i)
    {
        if (variameters[i].mVariameterType == OutputVariable)
        {
            createTableRow(r, variameters[i], OutputVariable);
            ++r;
        }
    }
    // Insert the separator row if there were any output variables)
    if (r != outputVarSeparatorRow)
    {
        createSeparatorRow(outputVarSeparatorRow,"OutputVariables", OutputVariable);
        ++r;
    }

    // Write remaining port variables
    QString currPortName;
    for (int i=0; i<variameters.size(); ++i)
    {
        if ( (variameters[i].mVariameterType == OtherVariable) &&
             ( gpConfig->getBoolSetting(CFG_SHOWHIDDENNODEDATAVARIABLES) || (variameters[i].mVariabelType != "Hidden") ) )
        {
            // Extract current port name to see if we should make a separator
            QString portName = variameters[i].mPortName;
            if (portName != currPortName)
            {
                currPortName = portName;
                Port* pPort = mpModelObject->getPort(portName);
                if (pPort)
                {
                    QString desc = pPort->getPortDescription();
                    if (!desc.isEmpty())
                    {
                        portName.append("     "+desc);
                    }
                }
                createSeparatorRow(r,"Port: "+portName, OtherVariable);
                ++r;
            }

            createTableRow(r, variameters[i], OtherVariable);
            ++r;
        }
    }

    resizeColumnToContents(Name);
    resizeColumnToContents(Unit);
    resizeColumnToContents(Value);
    resizeColumnToContents(Quantity);
    resizeColumnToContents(PlotSettings);
    resizeColumnToContents(ShowPort);
    setColumnWidth(Description, 2*columnWidth(Description));

#if QT_VERSION >= 0x050000
    horizontalHeader()->setSectionResizeMode(Name, QHeaderView::ResizeToContents);
//    horizontalHeader()->setSectionResizeMode(Alias, QHeaderView::ResizeToContents);
    horizontalHeader()->setSectionResizeMode(Description, QHeaderView::Stretch);
    horizontalHeader()->setSectionResizeMode(Value, QHeaderView::ResizeToContents);
    horizontalHeader()->setSectionResizeMode(Unit, QHeaderView::ResizeToContents);
    horizontalHeader()->setSectionResizeMode(Quantity, QHeaderView::ResizeToContents);
    horizontalHeader()->setSectionResizeMode(PlotSettings, QHeaderView::ResizeToContents);
    horizontalHeader()->setSectionResizeMode(ShowPort, QHeaderView::ResizeToContents);
    horizontalHeader()->setSectionsClickable(false);
#else
    horizontalHeader()->setResizeMode(Name, QHeaderView::ResizeToContents);
//    horizontalHeader()->setResizeMode(Alias, QHeaderView::ResizeToContents);
    horizontalHeader()->setResizeMode(Description, QHeaderView::Stretch);
    horizontalHeader()->setResizeMode(Value, QHeaderView::ResizeToContents);
    horizontalHeader()->setResizeMode(Unit, QHeaderView::ResizeToContents);
    horizontalHeader()->setResizeMode(Quantity, QHeaderView::ResizeToContents);
    horizontalHeader()->setResizeMode(PlotSettings, QHeaderView::ResizeToContents);
    horizontalHeader()->setResizeMode(ShowPort, QHeaderView::ResizeToContents);
    horizontalHeader()->setClickable(false);
#endif

    verticalHeader()->hide();
}

bool VariableTableWidget::setStartValues()
{
    //Hack that will make sure all values currently being edited is set before using them
    setDisabled(true);
    setEnabled(true);

    bool addedUndoPost = false;
    bool allok=true;
    for (int row=0; row<rowCount(); ++row)
    {

        // First check if row is separator, then skip it
        if (columnSpan(row,0)>1)
        {
            continue;
        }

        // If startvalue is empty (disabled, then we should not attempt to change it)
        bool isDisabled = qobject_cast<ParameterValueSelectionWidget*>(cellWidget(row, int(VariableTableWidget::Value)))->isValueDisabled();
        if (isDisabled)
        {
            continue;
        }

        ParameterValueSelectionWidget *pValueWideget = qobject_cast<ParameterValueSelectionWidget*>(cellWidget(row, int(VariableTableWidget::Value)));
        // Extract name and value from row
        QString name = pValueWideget->getName();
        QString value = pValueWideget->getValueText();

        if(mpModelObject->getTypeName() == MODELICATYPENAME && name != "model")
        {
            continue;
        }

        // Check if we have new custom scaling
        UnitScale newCustomUnitScale, previousUnitScale;
        mpModelObject->getCustomParameterUnitScale(name, previousUnitScale);
        UnitSelectionWidget *pUnitWidget = pValueWideget->getUnitSelectionWidget();
        if (pUnitWidget)
        {
            bool isDouble=false;
            value.toDouble(&isDouble);

            // Unregister unitscale if default unit has been reset or if the value is not a number (but a parameter name, (Actually a string))
            if (pUnitWidget->isDefaultSelected() || !isDouble)
            {
                mpModelObject->unregisterCustomParameterUnitScale(name);
            }
            // Else register (remember) the new unit scale
            else
            {
                pUnitWidget->getSelectedUnitScale(newCustomUnitScale);
                mpModelObject->registerCustomParameterUnitScale(name, newCustomUnitScale);
            }
        }

        // If we have a custom unit scale then undo the scale and set a value expressed in the default unit
        if (!newCustomUnitScale.isEmpty())
        {
            value = newCustomUnitScale.invRescale(value);
        }

        // Get the old value to see if a change has occurred
        // We also check the unit scale as that may have changed to even if the value (in original unit) is the same
        QString oldValue = mpModelObject->getParameterValue(name);
        bool setNewValueSucess=false;
        if ((oldValue != value) || (previousUnitScale != newCustomUnitScale))
        {
            // Parameter has changed, add to undo stack and set the parameter
            if( cleanAndVerifyParameterValue(value, qobject_cast<ParameterValueSelectionWidget*>(cellWidget(row, int(VariableTableWidget::Value)))->getDataType()) )
            {
                // Try to set the actual parameter
                if(mpModelObject->setParameterValue(name, value))
                {
                    // Add an undo post (but only one for all values changed this time
                    if(!addedUndoPost)
                    {
                        mpModelObject->getParentContainerObject()->getUndoStackPtr()->newPost(UNDO_CHANGEDPARAMETERS);
                        addedUndoPost = true;
                    }
                    // Register the change in undo stack
                    mpModelObject->getParentContainerObject()->getUndoStackPtr()->registerChangedParameter(mpModelObject->getName(),
                                                                                                           name,
                                                                                                           oldValue,
                                                                                                           value);
                    // Mark project tab as changed
                    mpModelObject->getParentContainerObject()->hasChanged();

                    setNewValueSucess =true;
                }
                // If we fail to set the parameter, then warning box
                else
                {
                    QMessageBox::critical(0, "Hopsan GUI", QString("'%1' is an invalid value for parameter '%2'. Resetting old value '%3'!").arg(value).arg(name).arg(oldValue));
                }
            }

            // Reset old value
            if (!setNewValueSucess)
            {
                ParameterValueSelectionWidget *pWidget = qobject_cast<ParameterValueSelectionWidget*>(cellWidget(row, int(VariableTableWidget::Value)));
                pWidget->setValueAndScale_nosignals(previousUnitScale.rescale(oldValue), previousUnitScale);
                mpModelObject->registerCustomParameterUnitScale(name, previousUnitScale);
                allok = false;
                break;
            }
        }
        //! @todo if we set som ok but som later fail pressing cancel will not restore the first that were set ok /Peter
    }
    return allok;
}

//bool VariableTableWidget::setCustomPlotScaleValues()
//{
//    bool allok=true;
//    for (int row=0; row<rowCount(); ++row)
//    {

//        // First check if row is separator, then skip it
//        if (columnSpan(row,0)>1)
//        {
//            continue;
//        }

//        // Extract PlotScaleSelector from row
//        PlotScaleSelectionWidget *pPlotScaleSelector = qobject_cast<PlotScaleSelectionWidget*>(cellWidget(row, int(VariableTableWidget::Scale)));
//        if (!pPlotScaleSelector)
//        {
//            continue;
//        }

//        // Only register if changed
//        if (pPlotScaleSelector->hasChanged())
//        {
//            pPlotScaleSelector->registerCustomScale();
//        }
//    }
//    return allok;
//}

bool VariableTableWidget::setAliasNames()
{
    for (int r=0; r<rowCount(); ++r)
    {
        if (!setAliasName(r))
        {
            return false;
        }
    }
    return true;
}

bool VariableTableWidget::focusNextPrevChild(bool next)
{
    bool retval = QTableWidget::focusNextPrevChild(next);

    int col = currentColumn();
    int row = currentRow();

    //Skip non-editable columns and separator rows
    if(columnSpan(row,0)>2 && next && row != rowCount()-1)
    {
        ++row;
        setCurrentCell(row,col);
    }
    else if(columnSpan(row,0)>2 && !next && row != 0)
    {
        --row;
        setCurrentCell(row,col);
    }

    while(!columnSpan(row,0)>2 && (col == Description || col == Unit || col == NumCols))
    {
        retval = focusNextPrevChild(next);
        col = currentColumn();
        row = currentRow();
    }

    QModelIndex idx = currentIndex();
    QWidget *pIndexWidget = this->indexWidget(idx);

    if(row == 0)
    {
        return retval;
    }
    else if(currentColumn() == Value)
    {
        ParameterValueSelectionWidget *pParWidget = qobject_cast<ParameterValueSelectionWidget*>(pIndexWidget);
        if(pParWidget)
        {
            pParWidget->getValueEditPtr()->setFocus();
        }
    }
    else if(currentColumn() == Quantity)
    {
        //! @todo quantitys?
//        PlotScaleSelectionWidget *pScaleWidget = qobject_cast<PlotScaleSelectionWidget*>(pIndexWidget);
//        pScaleWidget->getPlotScaleEditPtr()->setFocus();
    }
    else if(currentColumn() == ShowPort)
    {
        HideShowPortWidget *pPortWidget = qobject_cast<HideShowPortWidget*>(pIndexWidget);
        pPortWidget->getCheckBoxPtr()->setFocus();
    }
    else
    {
        this->setFocus();
    }

    return retval;
}

void VariableTableWidget::createTableRow(const int row, const CoreVariameterDescription &rData, const VariameterTypEnumT variametertype)
{
    QString fullName, name;
    QTableWidgetItem *pItem;
    insertRow(row);

    //! @todo maybe store the variameter data objects locally, and check for hidden info there, would also make it possible to check for changes without asking core all of the time /Peter

    if (variametertype == Constant)
    {
        fullName = rData.mName;
        name = rData.mName;
    }
    else if (variametertype == OtherVariable)
    {
        if (rData.mPortName.isEmpty())
        {
            fullName = rData.mName;
        }
        else
        {
            fullName = rData.mPortName+"#"+rData.mName;
        }
        name = rData.mName;
    }
    else //For input and output variables
    {
        fullName = rData.mPortName+"#"+rData.mName;
        name = rData.mPortName;
    }

    // Set the name field
    pItem = new QTableWidgetItem(name);
    pItem->setToolTip(fullName);
    pItem->setTextAlignment(Qt::AlignCenter);
    pItem->setFlags(Qt::NoItemFlags | Qt::ItemIsEnabled);
    setItem(row,Name,pItem);

    // Set the alias name field
    pItem = new QTableWidgetItem(rData.mAlias);
    pItem->setTextAlignment(Qt::AlignCenter);
    pItem->setFlags(Qt::NoItemFlags);
    if (variametertype != Constant)
    {
        pItem->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled);
    }
    setItem(row,Alias,pItem);

    // Set the description field
    pItem = new QTableWidgetItem(rData.mDescription);
    pItem->setTextAlignment(Qt::AlignCenter);
    pItem->setFlags(Qt::NoItemFlags | Qt::ItemIsEnabled);
    pItem->setToolTip(rData.mDescription);
    setItem(row,Description,pItem);

    // Set the unit field
    pItem = new QTableWidgetItem(rData.mUnit);
    pItem->setTextAlignment(Qt::AlignCenter);
    pItem->setFlags(Qt::NoItemFlags | Qt::ItemIsEnabled);
    setItem(row,Unit,pItem);

    // Set the value field
    ParameterValueSelectionWidget *pValueWidget = new ParameterValueSelectionWidget(rData, variametertype, mpModelObject, this);
    this->setIndexWidget(model()->index(row,Value), pValueWidget);
    // Trigger signal to unit selector if syspar entered to disable the unit scroll box
    pValueWidget->checkIfSysParEntered();

    // Create the quantity widget
    if (variametertype != Constant)
    {
//        QWidget *pPlotScaleWidget = new PlotScaleSelectionWidget(rData, mpModelObject, this);
//        this->setIndexWidget(model()->index(row,Scale), pPlotScaleWidget);
        QWidget *pQuantityWidget = new QuantitySelectionWidget(rData, mpModelObject, this);
        setIndexWidget(model()->index(row,Quantity), pQuantityWidget);
    }
    else
    {
        pItem = new QTableWidgetItem();
        pItem->setFlags(Qt::NoItemFlags);
        setItem(row,Quantity,pItem);
    }

    // Set invert plot check box
    if (variametertype != Constant)
    {
        QWidget *pPlotWidget = new PlotRelatedWidget(rData, mpModelObject, this);
        setIndexWidget(model()->index(row,PlotSettings), pPlotWidget);
    }
    else
    {
        pItem = new QTableWidgetItem();
        pItem->setFlags(Qt::NoItemFlags);
        setItem(row,PlotSettings,pItem);
    }


    // Set the port hide/show button
    if ( (variametertype == InputVaraiable) || (variametertype == OutputVariable))
    {
        HideShowPortWidget *pWidget = new HideShowPortWidget(rData, mpModelObject, this);
        connect(pWidget, SIGNAL(toggled(bool)), pValueWidget, SLOT(refreshValueTextStyle()));
        this->setIndexWidget(model()->index(row,ShowPort), pWidget);
    }
    else
    {
        pItem = new QTableWidgetItem();
        pItem->setFlags(Qt::NoItemFlags);
        setItem(row,ShowPort,pItem);
    }
}

void VariableTableWidget::createSeparatorRow(const int row, const QString &rName, const VariameterTypEnumT variametertype)
{
    insertRow(row);

    QTableWidgetItem *pItem, *pItem2=0;
    pItem = new QTableWidgetItem(rName);
    pItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    pItem->setFlags(Qt::NoItemFlags | Qt::ItemIsEnabled);
    pItem->setBackgroundColor(Qt::lightGray);

    if ( variametertype == InputVaraiable )
    {
        pItem2 = new QTableWidgetItem("Default Value");
    }
    else if ( variametertype == OutputVariable )
    {
        pItem2 = new QTableWidgetItem("Start Value");
    }
    else if ( variametertype == OtherVariable )
    {
        pItem2 = new QTableWidgetItem("Start Value");
    }

    if (pItem2)
    {
        pItem2->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        pItem2->setFlags(Qt::NoItemFlags | Qt::ItemIsEnabled);
        pItem2->setBackgroundColor(Qt::lightGray);

        setSpan(row,Name,1,Value);
        setItem(row,Name,pItem);
        setSpan(row,Value,1,NumCols-Value);
        setItem(row,Value,pItem2);
    }
    else
    {
        setSpan(row,Name,1,NumCols);
        setItem(row,Name,pItem);
    }
    resizeRowToContents(row);
}

TableWidgetTotalSize::TableWidgetTotalSize(QWidget *pParent) : QTableWidget(pParent)
{
    mMaxVisibleRows=20;
}

QSize TableWidgetTotalSize::sizeHint() const
{
    int w=0;
    if (verticalHeader()->isVisible())
    {
        w += verticalHeader()->sizeHint().width();
    }
    w +=  + frameWidth()*2 + verticalScrollBar()->sizeHint().width();
    //qDebug() << "w: " << w << " lw: " << lineWidth() << "  mLw: " << midLineWidth() << "  frameWidth: " << frameWidth();
    //qDebug() << verticalScrollBar()->sizeHint().width();

    for (int c=0; c<columnCount(); ++c)
    {
        w += columnWidth(c);
    }

    int h = horizontalHeader()->sizeHint().height() + frameWidth()*2;
    for (int r=0; r<qMin(mMaxVisibleRows,rowCount()); ++r)
    {
        h += rowHeight(r);
    }
    return QSize(w, h);
}

void TableWidgetTotalSize::setMaxVisibleRows(const int maxRows)
{
    mMaxVisibleRows = maxRows;
}


//PlotScaleSelectionWidget::PlotScaleSelectionWidget(const CoreVariameterDescription &rData, ModelObject *pModelObject, QWidget *pParent) :
//    QWidget(pParent)
//{
//    mVariableTypeName = rData.mName;
//    mVariablePortDataName = rData.mPortName+"#"+rData.mName;
//    mOriginalUnit = rData.mUnit;
//    mpModelObject = pModelObject;

//    QHBoxLayout* pLayout = new QHBoxLayout(this);
//    QMargins margins = pLayout->contentsMargins(); margins.setBottom(0); margins.setTop(0);
//    pLayout->setContentsMargins(margins);

//    mpPlotScaleEdit = new QLineEdit();
//    mpPlotScaleEdit->setAlignment(Qt::AlignCenter);
//    mpPlotScaleEdit->setFrame(false);
//    pLayout->addWidget(mpPlotScaleEdit);

//    UnitScale currCustom;
//    pModelObject->getCustomPlotUnitOrScale(mVariablePortDataName, currCustom);
//    if (!currCustom.mScale.isEmpty()) // Check if data exists
//    {
//        // If minus one scale then show -1
//        if (currCustom.isMinusOne())
//        {
//            mpPlotScaleEdit->setText(currCustom.mScale);
//        }
//        // If unit not given, display the scale value
//        else if (currCustom.mUnit.isEmpty())
//        {
//            mpPlotScaleEdit->setText(currCustom.mScale);
//        }
//        // If description given use it (usually the custom unit)
//        else
//        {
//            mpPlotScaleEdit->setText(currCustom.mUnit);
//        }

//        //! @todo what about showinf manually set custom units like "1 [m]"
//    }

//    QToolButton *pScaleSelectionButton =  new QToolButton(this);
//    pScaleSelectionButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-NewPlot.png"));
//    pScaleSelectionButton->setToolTip("Select Unit Scaling");
//    pScaleSelectionButton->setFixedSize(24,24);
//    connect(pScaleSelectionButton, SIGNAL(clicked()), this, SLOT(createPlotScaleSelectionMenu()));
//    pLayout->addWidget(pScaleSelectionButton);
//}

//void PlotScaleSelectionWidget::createPlotScaleSelectionMenu()
//{
//    QMap<QString, double> unitScales;
//    QMenu menu;
//    if (mVariableTypeName == "Value")
//    {
//        QStringList pqs = gpConfig->getQuantitiesForUnit(mOriginalUnit);
//        if (pqs.size() > 0)
//        {
//            unitScales = gpConfig->getUnitScales(pqs.first());
//        }
//    }
//    else
//    {
//        unitScales = gpConfig->getUnitScales(mVariableTypeName);
//    }
//    if (!unitScales.isEmpty())
//    {
//        QList<QString> keys = unitScales.keys();
//        QMap<QAction*, int> actionScaleMap;

//        for (int i=0; i<keys.size(); ++i)
//        {
//            QAction *tempAction = menu.addAction(keys[i]);
//            actionScaleMap.insert(tempAction, i);
//            tempAction->setIconVisibleInMenu(false);
//        }

//        //! @todo maybe add this
//        //    if(!menu.isEmpty())
//        //    {
//        //        menu.addSeparator();
//        //    }
//        //    QAction *pAddAction = menu.addAction("Add global unit scale");
//        QAction *pAddAction = 0;


//        QCursor cursor;
//        QAction *selectedAction = menu.exec(cursor.pos());
//        if(selectedAction == pAddAction)
//        {
//            //! @todo maybe add this
//            return;
//        }

//        int idx = actionScaleMap.value(selectedAction,-1);
//        if (idx >= 0)
//        {
//            QString key =  keys.at(idx);
//            if(!key.isEmpty())
//            {
//                // Set the selected unit scale
//                //mpModelObject->registerCustomPlotUnitOrScale(mVariablePortDataName, key, QString("%1").arg(unitScales.value(key)));
//                mpPlotScaleEdit->setText(key);
//            }
//        }
//    }
//}

//void PlotScaleSelectionWidget::registerCustomScale()
//{
//    QMap<QString, double> unitScales;


//    QString val = mpPlotScaleEdit->text();

//    if (val=="-1" || val=="-1.0")
//    {
//        val = QString("-1 [%1]").arg(mOriginalUnit);
//    }
//    else
//    {
//        if (mVariableTypeName == "Value")
//        {
//            QStringList pqs = gpConfig->getQuantitiesForUnit(mOriginalUnit);
//            if (pqs.size() > 0)
//            {
//                unitScales = gpConfig->getUnitScales(pqs.first());
//            }
//        }
//        else
//        {
//            unitScales = gpConfig->getUnitScales(mVariableTypeName);
//        }
//    }

//    if (unitScales.contains(val))
//    {
//        mpModelObject->registerCustomPlotUnitOrScale(mVariablePortDataName, val, QString("%1").arg(unitScales.value(val)));
//    }
//    else if (val.contains('['))
//    {
//        // Ok the user want to add a custom specific scale
//        QStringList fields = val.split(' ');
//        if (fields.size() == 2)
//        {
//            //! @todo need to check if text is valid number
//            mpModelObject->registerCustomPlotUnitOrScale(mVariablePortDataName, fields.last().remove('[').remove(']'), fields.first());
//        }
//        else
//        {
//            //! @todo report error
//        }
//    }
//    else
//    {
//        //! @todo need to check if text is valid number
//        mpModelObject->registerCustomPlotUnitOrScale(mVariablePortDataName, "", val);
//    }
//}

//bool PlotScaleSelectionWidget::hasChanged() const
//{
//    UnitScale us;
//    mpModelObject->getCustomPlotUnitOrScale(mVariablePortDataName, us);
//    if (us.mUnit.isEmpty())
//    {
//        return us.mScale != mpPlotScaleEdit->text();
//    }
//    else
//    {
//        return us.mUnit != mpPlotScaleEdit->text();
//    }
//}

//QLineEdit *PlotScaleSelectionWidget::getPlotScaleEditPtr() const
//{
//    return mpPlotScaleEdit;
//}


ParameterValueSelectionWidget::ParameterValueSelectionWidget(const CoreVariameterDescription &rData, VariableTableWidget::VariameterTypEnumT type, ModelObject *pModelObject, QWidget *pParent) :
    QWidget(pParent)
{
    mpValueEdit = 0;
    mpConditionalValueComboBox = 0;
    mpUnitSelectionWidget = 0;
    mDefaultUnitScale.setOnlyScale(1);
    mpModelObject = pModelObject;
    mVariameterType = type;
    mVariablePortName = rData.mPortName;

    //! @todo maybe store the variameter data objects locally, and check for hidden info there, would also make it possible to check for changes without asking core all of the time /Peter
    if (rData.mPortName.isEmpty())
    {
        mVariablePortDataName = rData.mName;
    }
    else
    {
        mVariablePortDataName = rData.mPortName+"#"+rData.mName;
    }
    mVariableDataType = rData.mDataType;

    QHBoxLayout* pLayout = new QHBoxLayout(this);
    QMargins margins = pLayout->contentsMargins(); margins.setBottom(0); margins.setTop(0);
    pLayout->setContentsMargins(margins);


    if (!rData.mDataType.isEmpty())
    {
        QLabel *pTypeLetter = new QLabel(rData.mDataType[0], this);
        pTypeLetter->setToolTip("DataType: "+rData.mDataType);
        pLayout->addWidget(pTypeLetter);
    }

    // Only set the rest if a value exist (it does not for disabled startvalues)
    if (mpModelObject->hasParameter(mVariablePortDataName))
    {
        QString value = mpModelObject->getParameterValue(mVariablePortDataName);

        mpValueEdit = new QLineEdit(this);
        mpValueEdit->setAlignment(Qt::AlignCenter);
        mpValueEdit->setFrame(false);

        if(mVariableDataType == "conditional")
        {
            mpConditionalValueComboBox = new QComboBox(this);
            for(int i=0; i<rData.mConditions.size(); ++i)
            {
                mpConditionalValueComboBox->addItem(rData.mConditions[i]);
            }
            mpConditionalValueComboBox->setCurrentIndex(value.toInt());
            mpValueEdit->setText(value);
            pLayout->addWidget(mpConditionalValueComboBox);
            connect(mpConditionalValueComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setConditionalValue(int)));
            mpValueEdit->hide();
        }
        else
        {
            pLayout->addWidget(mpValueEdit);
            UnitScale currentCustomUS;
            if (mpModelObject->getCustomParameterUnitScale(mVariablePortDataName, currentCustomUS))
            {
                mCustomScale = currentCustomUS;
                mpValueEdit->setText(mCustomScale.rescale(value));
            }
            else
            {
                mpValueEdit->setText(value);
            }
            connect(mpValueEdit, SIGNAL(editingFinished()), this, SLOT(setValue()));
            connect(mpValueEdit, SIGNAL(textChanged(QString)), this, SLOT(checkIfSysParEntered()));
            refreshValueTextStyle();

            mDefaultUnitScale.setOnlyScale(1);

            // Set the unit selection field based on Quantity
            QString quantity = rData.mQuantity;
            // If quantity is empty, first try to figure out from unit (assuming base units)
            if (quantity.isEmpty())
            {
                if (gpConfig->isRegisteredBaseUnit(rData.mUnit))
                {
                    QStringList pqs = gpConfig->getQuantitiesForUnit(rData.mUnit);
                    // Only allow listing in case we have one unique quantity match for base unit
                    if (pqs.size() == 1)
                    {
                        quantity = pqs.front();
                    }
                }
            }

            // Get the default unit scale (should be one but also contains quantity and unit name)
            if (!quantity.isEmpty())
            {
                UnitScale us;
                gpConfig->getUnitScale(quantity, rData.mUnit, us);
                if (!us.isEmpty())
                {
                    mDefaultUnitScale = us;
                }
            }


            if (!quantity.isEmpty())
            {
                mpUnitSelectionWidget = new UnitSelectionWidget(quantity, this);
                mpUnitSelectionWidget->setUnitScaling(currentCustomUS);
                connect(mpUnitSelectionWidget, SIGNAL(unitChanged(UnitScale)), this, SLOT(rescaleByUnitScale(UnitScale)));
                pLayout->addWidget(mpUnitSelectionWidget);

            }
        }

        QToolButton *pResetButton =  new QToolButton(this);
        pResetButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-ResetDefault.png"));
        pResetButton->setToolTip("Reset Default Value");
        pResetButton->setFixedSize(24,24);
        connect(pResetButton, SIGNAL(clicked()), this, SLOT(resetDefault()));
        pLayout->addWidget(pResetButton);

        if(mVariableDataType != "conditional")
        {
            QToolButton *pSystemParameterToolButton = new QToolButton(this);
            pSystemParameterToolButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-SystemParameter.png"));
            pSystemParameterToolButton->setToolTip("Map To System Parameter");
            pSystemParameterToolButton->setFixedSize(24,24);
            connect(pSystemParameterToolButton, SIGNAL(clicked()), this, SLOT(createSysParameterSelectionMenu()));
            pLayout->addWidget(pSystemParameterToolButton);
        }
    }
    else
    {
        QLabel *pLabel = new QLabel("Disabled",this);
        pLabel->setStyleSheet("color: Gray;");
        pLayout->addWidget(pLabel, 1, Qt::AlignCenter);
    }
}

void ParameterValueSelectionWidget::setValueText(const QString &rText)
{
    if (mpValueEdit)
    {
        mpValueEdit->setText(rText);
        if (mpConditionalValueComboBox)
        {
            mpConditionalValueComboBox->setCurrentIndex(rText.toInt());
        }
    }
}

QString ParameterValueSelectionWidget::getValueText() const
{
    if (mpValueEdit)
    {
        return mpValueEdit->text();
    }
    else
    {
        return QString();
    }
}

const QString &ParameterValueSelectionWidget::getDataType() const
{
    return mVariableDataType;
}

const QString &ParameterValueSelectionWidget::getName() const
{
    return mVariablePortDataName;
}

UnitSelectionWidget *ParameterValueSelectionWidget::getUnitSelectionWidget()
{
    return mpUnitSelectionWidget;
}

bool ParameterValueSelectionWidget::isValueDisabled() const
{
    return (mpValueEdit == 0);
}

QLineEdit *ParameterValueSelectionWidget::getValueEditPtr() const
{
    return mpValueEdit;
}

//! @brief Use this to reset values without triggering rescale signals and such things
void ParameterValueSelectionWidget::setValueAndScale_nosignals(QString value, UnitScale &rCustomUS)
{
    mpValueEdit->blockSignals(true);

    mCustomScale=rCustomUS;
    mpValueEdit->setText(value);

    if (mpUnitSelectionWidget)
    {
        mpUnitSelectionWidget->blockSignals(true);
        if (mCustomScale.isEmpty())
        {
            mpUnitSelectionWidget->resetDefault();
        }
        else
        {
            mpUnitSelectionWidget->setUnitScaling(mCustomScale);
        }
        mpUnitSelectionWidget->blockSignals(false);
    }

    mpValueEdit->blockSignals(false);
}

void ParameterValueSelectionWidget::setValue()
{
    //! @todo maybe do something here, would be nice if we could check if value is OK, or maybe we should even set the value here
    refreshValueTextStyle();
}

void ParameterValueSelectionWidget::setConditionalValue(const int idx)
{
    if (mpValueEdit)
    {
        mpValueEdit->setText(QString("%1").arg(idx));
    }
}

void ParameterValueSelectionWidget::resetDefault()
{
    // It is important to reset the unit before resetting the value
    // it will make any custom unit scale to be reset first
    if (mpUnitSelectionWidget)
    {
        mpUnitSelectionWidget->resetDefault();
    }

    if(mpModelObject && mpValueEdit)
    {
        QString defaultText = mpModelObject->getDefaultParameterValue(mVariablePortDataName);
        if(!defaultText.isEmpty())
        {
            mpValueEdit->setText(defaultText);
            setDefaultValueTextStyle();

            if (mpConditionalValueComboBox)
            {
                mpConditionalValueComboBox->setCurrentIndex(defaultText.toInt());
            }
        }
    }
}

void ParameterValueSelectionWidget::createSysParameterSelectionMenu()
{
    QMenu menu;
    QMap<QAction*, QString> actionParamMap;

    QVector<CoreParameterData> paramDataVector;
    mpModelObject->getParentContainerObject()->getParameters(paramDataVector);

    for (int i=0; i<paramDataVector.size(); ++i)
    {
        QAction *tempAction = menu.addAction(paramDataVector[i].mName+" = "+paramDataVector[i].mValue);
        tempAction->setIconVisibleInMenu(false);
        actionParamMap.insert(tempAction, paramDataVector[i].mName);
    }

    if(!menu.isEmpty())
    {
        menu.addSeparator();
    }
    QAction *pAddAction = menu.addAction("Add System Parameter");

    QCursor cursor;
    QAction *selectedAction = menu.exec(cursor.pos());
    if(selectedAction == pAddAction)
    {
        //! @todo maybe the gpSystemParametersWidget should not be global, should be one for each system
        gpSystemParametersWidget->openAddParameterDialog();
        return;
    }
    QString parNameString = actionParamMap.value(selectedAction);
    if(!parNameString.isEmpty())
    {
        mpValueEdit->setText(parNameString);
        refreshValueTextStyle();
    }
}

void ParameterValueSelectionWidget::refreshValueTextStyle()
{
    if(mpModelObject && mpValueEdit)
    {
        if( !mCustomScale.isEmpty() || (mpValueEdit->text() != mpModelObject->getDefaultParameterValue(mVariablePortDataName)) )
        {
            QString style("color: black; font: bold;");
            decideBackgroundColor(style);
            mpValueEdit->setStyleSheet(style);
        }
        else
        {
            setDefaultValueTextStyle();
        }

    }
}

void ParameterValueSelectionWidget::rescaleByUnitScale(const UnitScale &rUnitScale)
{
    if (mpValueEdit)
    {
        bool isOK=false;
        mpValueEdit->text().toDouble(&isOK);
        QString valS = mpValueEdit->text();
        if (isOK)
        {
            // If we already have a custom scale then unconvert first
            if (!mCustomScale.isEmpty())
            {
                //val = val / mCustomScale.toDouble();
                valS = mDefaultUnitScale.rescale(mCustomScale.invRescale(valS));
            }
            // Now convert based on new scale values
            valS = rUnitScale.rescale(mDefaultUnitScale.invRescale(valS));

            // Set new value and remember new custom scale
            mpValueEdit->setText(QString("%1").arg(valS));
            mCustomScale = rUnitScale;
            refreshValueTextStyle();
        }
    }
}

bool ParameterValueSelectionWidget::checkIfSysParEntered()
{
    bool syspar = false;
    if (mpValueEdit)
    {
        syspar = mpModelObject->getParentContainerObject()->hasParameter(mpValueEdit->text());
    }

    if (mpUnitSelectionWidget)
    {
        if (syspar)
        {
            mpUnitSelectionWidget->resetDefault();
        }
        mpUnitSelectionWidget->setDisabled(syspar);
    }
    return syspar;
}

void ParameterValueSelectionWidget::setDefaultValueTextStyle()
{
    if (mpValueEdit)
    {
        QString style("color: black; font: normal;");
        decideBackgroundColor(style);
        mpValueEdit->setStyleSheet(style);
    }
}

void ParameterValueSelectionWidget::decideBackgroundColor(QString &rStyleString)
{
    if (mpValueEdit)
    {
        if (mVariameterType == VariableTableWidget::InputVaraiable)
        {
            Port *pPort = mpModelObject->getPort(mVariablePortName);
            if (pPort && pPort->isConnected())
            {
                rStyleString.append(" color: gray; background: LightGray; font: italic;");
                return;
            }
        }
        rStyleString.append(" background: white;");
    }
}


HideShowPortWidget::HideShowPortWidget(const CoreVariameterDescription &rData, ModelObject *pModelObject, QWidget *pParent) :
    QWidget(pParent)
{
    mpModelObject = pModelObject;
    mPortName = rData.mPortName;

    QHBoxLayout *pLayout = new QHBoxLayout(this);
    QMargins margins = pLayout->contentsMargins(); margins.setBottom(0); margins.setTop(0);
    pLayout->setContentsMargins(margins);
    mpCheckBox = new QCheckBox(this);
    pLayout->addWidget(mpCheckBox,Qt::AlignRight);

    mpCheckBox->setToolTip("Show/hide port");
    Port *pPort = mpModelObject->getPort(mPortName);
    mpCheckBox->setChecked((pPort && pPort->getPortAppearance()->mEnabled));
    connect(mpCheckBox, SIGNAL(toggled(bool)), this, SLOT(hideShowPort(bool)));
    connect(mpCheckBox, SIGNAL(toggled(bool)), this, SIGNAL(toggled(bool)));
}

QCheckBox *HideShowPortWidget::getCheckBoxPtr() const
{
    return mpCheckBox;
}

void HideShowPortWidget::hideShowPort(const bool doShow)
{
    if (doShow)
    {
        Port *pPort = mpModelObject->getPort(mPortName);
        if (pPort)
        {
            pPort->setEnable(true);
            mpModelObject->createRefreshExternalPort(mPortName);
        }
        else
        {
            pPort = mpModelObject->createRefreshExternalPort(mPortName);
            if (pPort)
            {
                // Make sure that our new port has the "correct" angle
                //! @todo this is incorrect, can not have hardcoded 180
                pPort->setRotation(180);
                pPort->setModified(true);
            }
        }
    }
    else
    {
        Port *pPort = mpModelObject->getPort(mPortName);
        if (pPort)
        {
            pPort->setEnable(false);
        }
    }
}


UnitSelectionWidget::UnitSelectionWidget(const QString &rQuantity, QWidget *pParent) :
    QWidget(pParent)
{
    mpUnitComboBox = 0;
    mQuantity = rQuantity;
    mBaseUnitIndex = -1;

    QHBoxLayout *pLayout = new QHBoxLayout(this);
    pLayout->setContentsMargins(0,0,0,0);

    gpConfig->getUnitScales(mQuantity, mUnitScales);
    mBaseUnit = gpConfig->getBaseUnit(mQuantity);

    if (!mUnitScales.isEmpty())
    {
        mpUnitComboBox = new QComboBox(this);
        mpUnitComboBox->installEventFilter(new MouseWheelEventEater(this));
        mpUnitComboBox->setMinimumWidth(60);

        for( UnitScale &us : mUnitScales )
        {
            mpUnitComboBox->addItem(us.mUnit);
            if (mBaseUnitIndex < 0)
            {
                // check if this is the default unit
                if ( mBaseUnit == us.mUnit )
                {
                    mBaseUnitIndex = mpUnitComboBox->count()-1;
                }
            }
        }

        mpUnitComboBox->setCurrentIndex(mBaseUnitIndex);
        pLayout->addWidget(mpUnitComboBox);
        connect(mpUnitComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(selectionChanged(int)));
    }
}

void UnitSelectionWidget::setUnitScaling(const UnitScale &rUs)
{
    if (mpUnitComboBox && !rUs.isEmpty())
    {
        int i = mpUnitComboBox->findText(rUs.mUnit);
        // If we have it, then make it selected
        if (i >= 0)
        {
            mpUnitComboBox->setCurrentIndex(i);
        }
        // If not then Add to list
        else
        {
            mUnitScales.append(rUs);
            mpUnitComboBox->addItem(rUs.mUnit);
            mpUnitComboBox->setCurrentIndex(mpUnitComboBox->count()-1);
        }
    }
}

QString UnitSelectionWidget::getSelectedUnit() const
{
    UnitScale us;
    getSelectedUnitScale(us);
    return us.mUnit;
}

double UnitSelectionWidget::getSelectedUnitScale() const
{
    UnitScale us;
    getSelectedUnitScale(us);
    return us.toDouble();
}

void UnitSelectionWidget::getSelectedUnitScale(UnitScale &rUnitScale) const
{
    if (mpUnitComboBox)
    {
        int i = mpUnitComboBox->currentIndex();
        if (i < mUnitScales.size())
        {
            rUnitScale =  mUnitScales[i];
        }
    }
    else
    {
        rUnitScale = UnitScale(mQuantity, mBaseUnit, "1.0");
    }
}

bool UnitSelectionWidget::isDefaultSelected() const
{
    if (mpUnitComboBox)
    {
        return mpUnitComboBox->currentIndex() == mBaseUnitIndex;
    }
    else
    {
        return true;
    }
}

void UnitSelectionWidget::resetDefault()
{
    if (mpUnitComboBox)
    {
        mpUnitComboBox->setCurrentIndex(mBaseUnitIndex);
    }
}

void UnitSelectionWidget::selectionChanged(int idx)
{
    Q_UNUSED(idx)
    UnitScale us;
    getSelectedUnitScale(us);
    emit unitChanged(us);
}


QuantitySelectionWidget::QuantitySelectionWidget(const CoreVariameterDescription &rData, ModelObject *pModelObject, QWidget *pParent) :
    QWidget(pParent)
{
    mVariableTypeName = rData.mName;
    mVariablePortDataName = rData.mPortName+"#"+rData.mName;
    mOriginalUnit = rData.mUnit;
    mQuantity = rData.mQuantity;
    mpModelObject = pModelObject;

    QHBoxLayout* pLayout = new QHBoxLayout(this);
    QMargins margins = pLayout->contentsMargins(); margins.setBottom(0); margins.setTop(0);
    pLayout->setContentsMargins(margins);

    mpQuantityLabel = new QLabel(this);
    mpQuantityLabel->setAlignment(Qt::AlignLeft);
    //mpQuantityLabel->setFrame(false);
    mpQuantityLabel->setText(mQuantity);
    pLayout->addWidget(mpQuantityLabel);

    if (rData.mUserModifiableQuantity)
    {
//        //! @todo read custom quantity maybe (but if it is set in core we wont need to read it since we already have iti
//        QString custQuant = mpModelObject->getCustomQuantity(mVariablePortDataName);
//        mpQuantityLabel->setText(custQuant);
//        mQuantity = custQuant;

        QToolButton *pQuantitySelectionButton =  new QToolButton(this);
        pQuantitySelectionButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-NewPlot.png"));
        pQuantitySelectionButton->setToolTip("Select Custom Quantity");
        pQuantitySelectionButton->setFixedSize(24,24);
        connect(pQuantitySelectionButton, SIGNAL(clicked()), this, SLOT(createQuantitySelectionMenu()));
        pLayout->addWidget(pQuantitySelectionButton);
    }
}

void QuantitySelectionWidget::registerCustomQuantity()
{
    if (hasChanged())
    {
        bool rc = mpModelObject->setModifyableSignalQuantity(mVariablePortDataName, mpQuantityLabel->text());
        if (rc)
        {
            mQuantity = mpQuantityLabel->text();
        }
    }
}

bool QuantitySelectionWidget::hasChanged() const
{
    return (mQuantity != mpQuantityLabel->text());
}

void QuantitySelectionWidget::createQuantitySelectionMenu()
{
    QStringList quantities = gpConfig->getUnitQuantities();

    QMenu menu;
    QAction *pClearAction = menu.addAction("Clear");
    menu.addSeparator();
    QMap<QAction*, int> actionScaleMap;
    for (int i=0; i<quantities.size(); ++i)
    {
        QAction *tempAction = menu.addAction(quantities[i]);
        actionScaleMap.insert(tempAction, i);
        tempAction->setIconVisibleInMenu(false);
    }

    QCursor cursor;
    QAction *pSelectedAction = menu.exec(cursor.pos());

    if (pSelectedAction == pClearAction)
    {
        mpQuantityLabel->clear();
        registerCustomQuantity();
    }
    else
    {
        int idx = actionScaleMap.value(pSelectedAction,-1);
        if (idx >= 0)
        {
            mpQuantityLabel->setText(quantities[idx]);
            registerCustomQuantity();
        }
    }
}

PlotRelatedWidget::PlotRelatedWidget(const CoreVariameterDescription &rData, ModelObject *pModelObject, QWidget *pParent)
    : QWidget(pParent)
{
    mpModelObject = pModelObject;
    mVariablePortDataName = rData.mPortName+"#"+rData.mName;
    mOrigInverted = mpModelObject->getInvertPlotVariable(mVariablePortDataName);
    mOriginalPlotLabel = mpModelObject->getVariablePlotLabel(mVariablePortDataName);

    QHBoxLayout *pLayout = new QHBoxLayout(this);
    QMargins margins = pLayout->contentsMargins(); margins.setBottom(0); margins.setTop(0);
    pLayout->setContentsMargins(margins);
    QCheckBox *pInverCheckbox = new QCheckBox(this);
    pInverCheckbox->setToolTip("Invert plot");
    pInverCheckbox->setChecked(mOrigInverted);
    QLineEdit *pLabelEdit = new QLineEdit(mOriginalPlotLabel,this);
    pLabelEdit->setFrame(false);
    pLabelEdit->setToolTip("Custom label");
    pLayout->addWidget(pInverCheckbox);
    pLayout->addWidget(pLabelEdit);

    connect(pInverCheckbox, SIGNAL(toggled(bool)), this, SLOT(invertPlot(bool)));
    connect(pLabelEdit, SIGNAL(textChanged(QString)), this, SLOT(setPlotLabel(QString)));
}

void PlotRelatedWidget::invertPlot(bool tf)
{
    mpModelObject->setInvertPlotVariable(mVariablePortDataName, tf);
}

void PlotRelatedWidget::setPlotLabel(QString label)
{
    mpModelObject->setVariablePlotLabel(mVariablePortDataName, label);
}
