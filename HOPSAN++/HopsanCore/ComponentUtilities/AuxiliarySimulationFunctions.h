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

namespace hopsan {

    double multByTwo(double input);     //! @todo Vad är det här bra för?
    void limitValue(double &value, double min, double max);
    bool doubleToBool(double value);
    double boolToDouble(bool value);
    double sign(double x);

    //! Functions converted from auxhop in old Hopsan
    double onPositive(double x);
    double dxOnPositive(double x);
    double onNegative(double x);
    double dxOnNegative(double x);
    double signedSquareL(double x, double x0);
    double dxSignedSquareL(double x, double x0);
    double squareAbsL(double x, double x0);
    double dxSquareAbsL(double x, double x0);
    double equalSigns(double x, double y);
    double limit(double x, double xmin, double xmax);
    double dxLimit(double x, double xmin, double xmax);
    double limit2(double x, double sx, double xmin, double xmax);
    double dxLimit2(double x, double sx, double xmin, double xmax);

}
#endif // AUXILIARYSIMULATIONFUNCTIONS_H
