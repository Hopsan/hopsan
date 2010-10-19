//$Id$

#ifndef AUXILIARYSIMULATIONFUNCTIONS_H
#define AUXILIARYSIMULATIONFUNCTIONS_H

namespace hopsan {

    double multByTwo(double input);     //! @todo Vad är det här bra för?
    void limit(double &value, double min, double max);
    bool doubleToBool(double value);
    double boolToDouble(bool value);
}
#endif // AUXILIARYSIMULATIONFUNCTIONS_H
