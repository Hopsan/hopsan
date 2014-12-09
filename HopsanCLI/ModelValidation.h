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
//! @file   HopsanCLI/ModelValidation.h
//! @author peter.nordin@liu.se
//! @date   2014-12-09
//!
//! @brief Contains model validation functions for CLI
//!
//$Id$

#ifndef MODELVALIDATION_H
#define MODELVALIDATION_H

#include <string>

bool performModelTest(const std::string hvcFilePath);
bool createModelTestDataSet(const std::string modelPath, const std::string hvcFilePath);

#endif // MODELVALIDATION_H
