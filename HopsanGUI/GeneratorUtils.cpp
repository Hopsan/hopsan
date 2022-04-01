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

QSharedPointer<HopsanGeneratorGUI> createDefaultGenerator(bool autoCloseOnSuccess)
{
    auto pGenerator = QSharedPointer<HopsanGeneratorGUI>(new HopsanGeneratorGUI(gpDesktopHandler->getMainPath(),
                                                                                gpMainWindowWidget));
    pGenerator->setCompilerPath(gpConfig->getGCCPath());
    pGenerator->setAutoCloseWidgetsOnSuccess(autoCloseOnSuccess);
    return pGenerator;
}

QSharedPointer<HopsanGeneratorGUI> createDefaultImportGenerator()
{
    return createDefaultGenerator(true);
}

QSharedPointer<HopsanGeneratorGUI> createDefaultExportGenerator()
{
    return createDefaultGenerator(false);
}
