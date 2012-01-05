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
//! @file   AuxiliarySimulationFunctions.h
//! @author Flumes
//! @date   2010-11-29
//!
//! @brief Contains small auxiliary functions that can be useful when creating components
//!

//$Id$

#ifndef AUXILIARYSIMULATIONFUNCTIONS_H
#define AUXILIARYSIMULATIONFUNCTIONS_H
#include "win32dll.h"

namespace hopsan {

const double pi = 3.14159265358979323846;

extern "C" {
    void DLLIMPORTEXPORT limitValue(double &value, double min, double max);
    bool DLLIMPORTEXPORT doubleToBool(double value);
    double DLLIMPORTEXPORT boolToDouble(bool value);
    double DLLIMPORTEXPORT sign(double x);

    //! Functions converted from auxhop in old Hopsan
    double DLLIMPORTEXPORT onPositive(double x);
    double DLLIMPORTEXPORT dxOnPositive(double x);
    double DLLIMPORTEXPORT onNegative(double x);
    double DLLIMPORTEXPORT dxOnNegative(double x);
    double DLLIMPORTEXPORT dxAbs(double x);
    double DLLIMPORTEXPORT ifPositive(double x, double y1, double y2);
    double DLLIMPORTEXPORT dtIfPositive(double x, double y1, double y2);
    double DLLIMPORTEXPORT dfIfPositive(double x, double y1, double y2);
    double DLLIMPORTEXPORT signedSquareL(double x, double x0);
    double DLLIMPORTEXPORT dxSignedSquareL(double x, double x0);
    double DLLIMPORTEXPORT squareAbsL(double x, double x0);
    double DLLIMPORTEXPORT dxSquareAbsL(double x, double x0);
    double DLLIMPORTEXPORT Atan2L(double y, double x);
    double DLLIMPORTEXPORT d1Atan2L(double y, double x);
    double DLLIMPORTEXPORT d2Atan2L(double y, double x);
    double DLLIMPORTEXPORT ArcSinL(double x);
    double DLLIMPORTEXPORT dxArcSinL(double x);
    double DLLIMPORTEXPORT diffAngle(double fi1, double fi2);
    double DLLIMPORTEXPORT CLift( double alpha,double CLalpha,double ap,double an,double expclp,double expcln);
    double DLLIMPORTEXPORT CDragInd(double alpha,double AR,double e,double CLalpha,double ap,double an,double expclp,double expcln);
    double DLLIMPORTEXPORT equalSigns(double x, double y);
    double DLLIMPORTEXPORT limit(double x, double xmin, double xmax);
    double DLLIMPORTEXPORT dxLimit(double x, double xmin, double xmax);
    double DLLIMPORTEXPORT limit2(double x, double sx, double xmin, double xmax);
    double DLLIMPORTEXPORT dxLimit2(double x, double sx, double xmin, double xmax);
}
}
#endif // AUXILIARYSIMULATIONFUNCTIONS_H
