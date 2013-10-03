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

//!
//! @file   ComponentGeneratorUtilities.h
//! @author Robert Braun <robert.braun@liu.se
//! @date   2012-01-08
//!
//! @brief Contains component generation utiluties
//!
//$Id: GUIUtilities.cpp 3813 2012-01-05 17:11:57Z robbr48 $

#include <QStringList>
#include <QProcess>


#include "Configuration.h"
#include "GUIObjects/GUIModelObjectAppearance.h"
#include "Utilities/ComponentGeneratorUtilities.h"
#include "Widgets/LibraryWidget.h"
#include "Widgets/MessageWidget.h"
#include "common.h"


using namespace std;


//! @brief Returns a list of custom Hopsan functions that need to be allowed in the symbolic library
//! @todo Duplicated with HopsanGenerator
QStringList getCustomFunctionList()
{
    return QStringList() << "hopsanLimit" << "hopsanDxLimit" << "onPositive" << "onNegative" << "signedSquareL" << "limit";
}
