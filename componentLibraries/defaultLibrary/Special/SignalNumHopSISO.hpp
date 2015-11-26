#ifndef SIGNALNUMHOPSISO_HPP
#define SIGNALNUMHOPSISO_HPP

#include "ComponentEssentials.h"
#include "CoreUtilities/NumHopHelper.h"
#include "HopsanTypes.h"

#include <fstream>

namespace hopsan {

//!
//! @brief The data csv file should be  on , separated form or ; separated form
//!
//! @ingroup SignalComponents
//!
class SignalNumHopSISO : public ComponentSignal
{

private:
    double *mpIn, *mpOut;
    HString mScriptFile;
    NumHopHelper *mpNumHop;

public:
    static Component *Creator()
    {
        return new SignalNumHopSISO();
    }

    void configure()
    {
        addInputVariable("in", "", "", 0.0, &mpIn);
        addOutputVariable("out", "", "", &mpOut);

        addConstant("scriptfile", "Script file (absolute or relative to model path)", "", "FilePath", mScriptFile);

        mpNumHop = new NumHopHelper();
    }


    void initialize()
    {
        HString filepath = findFilePath(mScriptFile);
        std::ifstream is(filepath.c_str());
        std::string script;

        if (!mScriptFile.empty() && is.is_open())
        {
            while (!is.eof())
            {
                std::string line;
                getline(is, line);
                script.append(line).append("\n");
            }
        }
        else
        {
            addErrorMessage("Could not open numhop script file: "+mScriptFile);
            stopSimulation();
        }

        mpNumHop->setComponent(this);
        mpNumHop->registerDataPtr("in",mpIn);
        mpNumHop->registerDataPtr("out",mpOut);

        HString output;
        bool initOK = mpNumHop->interpretNumHopScript(script.c_str(), true, output);
        if (!initOK)
        {
            addErrorMessage("Error interpreting numhop script: "+output);
            stopSimulation();
        }

        simulateOneTimestep();
    }


    void simulateOneTimestep()
    {
        // Note! Read and Write to nodes is handled internally in NumHopHelper (due to registered pointers)

        HString dummy;
        double dummy2;
        bool evalOK = mpNumHop->eval(dummy2, false, dummy);
        if (!evalOK)
        {
            stopSimulation("NumHop evaluation failed");
        }
    }

    void deconfigure()
    {
        delete mpNumHop;
    }

    bool isExperimental() const
    {
        return true;
    }
};
}

#endif // SIGNALNUMHOPSISO_HPP
