#!/bin/bash

# Shell script for updating license headers in Hopsan sub projects
# Author: Peter Nordin

set -e
set -u

DOSET='--set'
#DOSET=''

LIC_HEADER='licenseHeaderAPL2'
Utilities/licenseChanger.py HopsanCore ${LIC_HEADER} -e HopsanCore/dependencies ${DOSET}
Utilities/licenseChanger.py HopsanCLI ${LIC_HEADER} ${DOSET}
Utilities/licenseChanger.py HopsanRemote ${LIC_HEADER} ${DOSET}
Utilities/licenseChanger.py HopsanGenerator ${LIC_HEADER} ${DOSET}
Utilities/licenseChanger.py Ops ${LIC_HEADER} ${DOSET}
Utilities/licenseChanger.py SymHop ${LIC_HEADER} ${DOSET}
Utilities/licenseChanger.py componentLibraries/defaultLibrary ${LIC_HEADER} ${DOSET}
Utilities/licenseChanger.py componentLibraries/exampleComponentLibrary ${LIC_HEADER} ${DOSET}
Utilities/licenseChanger.py componentLibraries/devLibraries ${LIC_HEADER} ${DOSET}
Utilities/licenseChanger.py componentLibraries/extensionLibrary ${LIC_HEADER} ${DOSET}
Utilities/licenseChanger.py componentLibraries/extensionLibrary08 ${LIC_HEADER} ${DOSET}
Utilities/licenseChanger.py UnitTests ${LIC_HEADER} ${DOSET}

LIC_HEADER='licenseHeaderGPL3'
Utilities/licenseChanger.py HopsanGUI ${LIC_HEADER} -e HopsanGUI/Dependencies ${DOSET}
Utilities/licenseChanger.py HoLC ${LIC_HEADER} ${DOSET}
