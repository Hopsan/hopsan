#!/bin/bash

# Shell script for updating license headers in Hopsan sub projects
# Author: Peter Nordin

set -e
set -u

DOSET='--set'
#DOSET=''

LIC_HEADER='licenseHeaderALv2'
Utilities/licenseChanger.py HopsanCore ${LIC_HEADER} ${DOSET} -e HopsanCore/dependencies
Utilities/licenseChanger.py HopsanCLI ${LIC_HEADER} ${DOSET}
Utilities/licenseChanger.py HopsanRemote ${LIC_HEADER} ${DOSET}
Utilities/licenseChanger.py HopsanGenerator ${LIC_HEADER} ${DOSET} -e HopsanGenerator/templates
Utilities/licenseChanger.py Ops ${LIC_HEADER} ${DOSET}
Utilities/licenseChanger.py SymHop ${LIC_HEADER} ${DOSET}
Utilities/licenseChanger.py componentLibraries/defaultLibrary ${LIC_HEADER} ${DOSET}
Utilities/licenseChanger.py componentLibraries/exampleComponentLib ${LIC_HEADER} ${DOSET}
Utilities/licenseChanger.py componentLibraries/extensionLibrary ${LIC_HEADER} ${DOSET}
Utilities/licenseChanger.py UnitTests ${LIC_HEADER} ${DOSET}

LIC_HEADER='licenseHeaderGPLv3'
Utilities/licenseChanger.py HopsanGUI ${LIC_HEADER} ${DOSET} -e HopsanGUI/dependencies