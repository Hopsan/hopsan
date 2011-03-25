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
#include "../win32dll.h"

namespace hopsan {
extern "C" {
    double DLLIMPORTEXPORT multByTwo(double input);     //! @todo Vad är det här bra för?
    void DLLIMPORTEXPORT limitValue(double &value, double min, double max);
    bool DLLIMPORTEXPORT doubleToBool(double value);
    double DLLIMPORTEXPORT boolToDouble(bool value);
    double DLLIMPORTEXPORT sign(double x);

    //! Functions converted from auxhop in old Hopsan
    double DLLIMPORTEXPORT onPositive(double x);
    double DLLIMPORTEXPORT dxOnPositive(double x);
    double DLLIMPORTEXPORT onNegative(double x);
    double DLLIMPORTEXPORT dxOnNegative(double x);
    double DLLIMPORTEXPORT signedSquareL(double x, double x0);
    double DLLIMPORTEXPORT dxSignedSquareL(double x, double x0);
    double DLLIMPORTEXPORT squareAbsL(double x, double x0);
    double DLLIMPORTEXPORT dxSquareAbsL(double x, double x0);
    double DLLIMPORTEXPORT equalSigns(double x, double y);
    double DLLIMPORTEXPORT limit(double x, double xmin, double xmax);
    double DLLIMPORTEXPORT dxLimit(double x, double xmin, double xmax);
    double DLLIMPORTEXPORT limit2(double x, double sx, double xmin, double xmax);
    double DLLIMPORTEXPORT dxLimit2(double x, double sx, double xmin, double xmax);

    //! Wrapped functions for Mathematica syntax
    double DLLIMPORTEXPORT Power(double x, double y);
    double DLLIMPORTEXPORT Sin(double x);
    double DLLIMPORTEXPORT Cos(double x);
    double DLLIMPORTEXPORT Tan(double x);
    double DLLIMPORTEXPORT Sqrt(double x);
}
}
#endif // AUXILIARYSIMULATIONFUNCTIONS_H
