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

#include "GeneratorUtils.h"
#include "DesktopHandler.h"
#include "Configuration.h"
#include "LibraryHandler.h"
#include "MessageHandler.h"
#include "global.h"
#include "Utilities/GUIUtilities.h"

#include <QMessageBox>

QSharedPointer<HopsanGeneratorGUI> createDefaultGenerator()
{
    auto pGenerator = QSharedPointer<HopsanGeneratorGUI>(new HopsanGeneratorGUI(gpDesktopHandler->getMainPath(),
                                                                                gpMainWindowWidget));
    pGenerator->setCompilerPath(gpConfig->getGCCPath());
    pGenerator->setAutoCloseWidgetsOnSuccess(true);
    return pGenerator;
}

bool importFMU(const QString& fmuFilePath)
{
    auto pGenerator = createDefaultGenerator();

    QFileInfo fmuFileInfo(fmuFilePath);
    QString fmuFileName = fmuFileInfo.baseName();
    QString fmuImportDestination = QDir::cleanPath(gpDesktopHandler->getFMUPath()+"/"+fmuFileName);
    if(QDir(fmuImportDestination).exists())
    {
        QMessageBox existWarningBox(QMessageBox::Warning, QObject::tr("Warning"),
                                    QObject::tr("Another FMU with same name exist. Do you want unload this library (if loaded) and then overwrite the generated import files?"),
                                    QMessageBox::NoButton, gpMainWindowWidget);
        existWarningBox.addButton("Yes", QMessageBox::AcceptRole);
        existWarningBox.addButton("No", QMessageBox::RejectRole);
        existWarningBox.setWindowIcon(QIcon(QString(QString(ICONPATH) + "hopsan.png")));
        bool doIt = (existWarningBox.exec() == QMessageBox::AcceptRole);
        if(doIt)
        {
            gpLibraryHandler->unloadLibraryFMU(fmuFileName);
            removeDir(fmuImportDestination);
            if (QDir(fmuImportDestination).exists()) {
               gpMessageHandler->addErrorMessage(QString("Could not remove directory: %1").arg(fmuImportDestination));
            }
        }
        else
        {
            return false;
        }
    }

    if (!pGenerator->generateFromFmu(fmuFilePath, gpDesktopHandler->getFMUPath()))
    {
        gpMessageHandler->addErrorMessage("Fmu import generator failed");
    }

    if(QDir().exists(fmuImportDestination))
    {
        // Copy component icon
        QFile fmuIcon;
        fmuIcon.setFileName(QString(GRAPHICSPATH)+"/objecticons/fmucomponent.svg");
        fmuIcon.copy(fmuImportDestination+"/fmucomponent.svg");
        fmuIcon.close();
        fmuIcon.setFileName(fmuImportDestination+"/fmucomponent.svg");
        fmuIcon.setPermissions(QFile::WriteUser | QFile::ReadUser);
        fmuIcon.close();

        // Load library
        gpLibraryHandler->loadLibrary(fmuImportDestination+"/"+fmuFileName+"_lib.xml", FmuLib);
        return true;
    }
    return false;
}
