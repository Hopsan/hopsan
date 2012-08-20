/*-----------------------------------------------------------------------------
 This source file is part of Hopsan NG

 Copyright (c) 2011
    Mikael Axin, Robert Braun, Alessandro Dell'Amico, Björn Eriksson,
    Peter Nordin, Karl Pettersson, Petter Krus, Ingo Staack

 This file is provided "as is", with no guarantee or warranty for the
 functionality or reliability of the contents. All contents in this file is
 the original work of the copyright holders at the Division of Fluid and
 Mechatronic Systems (Flumes) at Linköping University. Modifying, using or
 redistributing any part of this file is prohibited without explicit
 permission from the copyright holders.
-----------------------------------------------------------------------------*/

#include "HopsanComponentGenerator.h"
#include "win32dll.h"

#include <QMessageBox>


using namespace std;

extern "C" DLLIMPORTEXPORT void callModelicaGenerator(string modelicaCode, string coreIncludePath, string binPath, bool showDialog=false)
{
    qDebug() << "Called generator (in dll)!";

    HopsanComponentGenerator *pGenerator = new HopsanComponentGenerator(QString(coreIncludePath.c_str()), QString(binPath.c_str()), showDialog);
    pGenerator->generateFromModelica(QString(modelicaCode.c_str()));
}


