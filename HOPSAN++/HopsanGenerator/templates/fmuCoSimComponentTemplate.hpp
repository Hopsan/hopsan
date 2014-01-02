#ifndef <<<modelname>>>_H
#define <<<modelname>>>_H

//*******************************************//
//             *** WARNING ***               //
//                                           //
//         AUTO GENERATED COMPONENT!         //
// ANY CHANGES WILL BE LOST IF RE-GENERATED! //
//*******************************************//

#define FMI_COSIMULATION

#define BUFSIZE 4096

#define _WIN32_WINNT 0x0502
#include "../fmi_cs.h"
#include "../xml_parser.h"
#include "<<<includepath>>>ComponentEssentials.h"

#include <sstream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <assert.h>
#ifdef WIN32
#include <windows.h>
#endif

void fmuLogger(fmiComponent c, fmiString instanceName, fmiStatus status,
               fmiString category, fmiString message, ...){}

namespace hopsan {

//*******************************************//
//             *** WARNING ***               //
//                                           //
//         AUTO GENERATED COMPONENT!         //
// ANY CHANGES WILL BE LOST IF RE-GENERATED! //
//*******************************************//

    class <<<modelname>>> : public Component<<<cqstype>>>
    {
    private:
        FMU mFMU;
        FILE* mFile;
        fmiComponent c;                  // instance of the fmu
        fmiEventInfo eventInfo;          // updated by calls to initialize and eventUpdate

        //Declare ports
>>>portdecl>>>        Port *<<<varname>>>;
<<<portdecl<<<

        //Declare node data pointers
>>>dataptrdecl>>>        double *<<<mpndname>>>;
<<<dataptrdecl<<<

        //Declare parameters
>>>pardecl>>>        double <<<varname>>>;
<<<pardecl<<<

    public:
        static Component *Creator()
        {
            return new <<<modelname>>>();
        }

        void configure()
        {
            //*******************************************//
            //             *** WARNING ***               //
            //                                           //
            //         AUTO GENERATED COMPONENT!         //
            // ANY CHANGES WILL BE LOST IF RE-GENERATED! //
            //*******************************************//

            mFMU.modelDescription = parse("<<<fmudir>>>/modelDescription.xml");
            assert(mFMU.modelDescription);
            assert(loadLib("<<<fmudir>>>/<<<modelname>>>.<<<fileext>>>"));
            addInfoMessage(getString(mFMU.modelDescription, att_modelIdentifier));

            //Add ports
>>>addports>>>            <<<varname>>> = add<<<porttype>>>("<<<portname>>>", "<<<nodetype>>>", <<<notrequired>>>);
<<<addports<<<

<<<addvars>>>

            //Initialize and register parameters
>>>addconstants>>>            addConstant("<<<parname>>>", "<<<description>>>", "-", <<<initvalue>>>, <<<varname>>>);
<<<addconstants<<<        }

        void initialize()
        {
            //*******************************************//
            //             *** WARNING ***               //
            //                                           //
            //         AUTO GENERATED COMPONENT!         //
            // ANY CHANGES WILL BE LOST IF RE-GENERATED! //
            //*******************************************//

            if (!mFMU.modelDescription)
            {
                addErrorMessage("Missing FMU model description");
                stopSimulation();
                return;
            }

            //Initialize node data pointers
>>>initdataptrs>>>            <<<mpndname>>> = getSafeNodeDataPtr(<<<varname>>>, <<<datatype>>>);
<<<initdataptrs<<<
           
            //Initialize FMU
            ModelDescription* md;            // handle to the parsed XML file
            const char* guid;                // global unique id of the fmu
            fmiCallbackFunctions callbacks;  // called by the model during simulation
            fmiStatus fmiFlag;               // return code of the fmu functions
            fmiReal t0 = mTime;                  // start time
            fmiBoolean toleranceControlled = fmiFalse;
            int loggingOn = 0;
            fmiReal timeout = 1000;          // wait period in milli seconds, 0 for unlimited wait period"
            fmiBoolean visible = fmiFalse;   // no simulator user interface
            fmiBoolean interactive = fmiFalse; // simulation run without user interaction
            const char* fmuLocation = NULL;  // path to the fmu as URL, "file://C:\QTronic\sales"
            const char* mimeType = "application/x-fmu-sharedlibrary"; // denotes tool in case of tool coupling
    
            // instantiate the fmu
            md = mFMU.modelDescription;
            guid = getString(md, att_guid);
            callbacks.logger = fmuLogger;
            callbacks.allocateMemory = calloc;
            callbacks.freeMemory = free;
            c = mFMU.instantiateSlave(getModelIdentifier(md), guid, fmuLocation, mimeType, timeout, visible, interactive, callbacks, loggingOn);
              
            if(!c)
            {
                addErrorMessage("Could not instantiate model.");
                stopSimulation();
                return;
            }  
            

    
              
            ScalarVariable** vars = mFMU.modelDescription->modelVariables;
            double value;
            ScalarVariable* sv;
            fmiValueReference vr;
                        
            //Set parameters
>>>setpars>>>            sv = vars[<<<valueref>>>];
            vr = getValueReference(sv);
            value=<<<varname>>>;
            mFMU.setReal(c, &vr, 1, &value);
<<<setpars<<<

                        // StopTimeDefined=fmiFalse means: ignore value of tEnd
            fmiFlag = mFMU.initializeSlave(c, t0, fmiTrue, 1);
            if(fmiFlag > fmiWarning)
            {
                addErrorMessage("Could not initialize model.");
                stopSimulation();
                return;
            }
        }

        void simulateOneTimestep()
        {
            //*******************************************//
            //             *** WARNING ***               //
            //                                           //
            //         AUTO GENERATED COMPONENT!         //
            // ANY CHANGES WILL BE LOST IF RE-GENERATED! //
            //*******************************************//

            ScalarVariable** vars = mFMU.modelDescription->modelVariables;
            double value;
            ScalarVariable* sv;
            fmiValueReference vr;

            //write input values
>>>readsignalinputs>>>            sv = vars[<<<valueref>>>];
                vr = getValueReference(sv);
                value = (*<<<mpndname>>>);
                mFMU.setReal(c, &vr, 1, &value);
<<<readsignalinputs<<<
>>>readinputs>>>            if(<<<varname>>>->isConnected())
            {
                sv = vars[<<<valueref>>>];
                vr = getValueReference(sv);
                value = (*<<<mpndname>>>);
                mFMU.setReal(c, &vr, 1, &value);
            }
<<<readinputs<<<
            //run simulation
            simulateFMU();

            //write back output values
>>>writeoutputs>>>            sv = vars[<<<valueref>>>];
            vr = getValueReference(sv);
            mFMU.getReal(c, &vr, 1, &value);
            (*<<<varname>>>) = value;
<<<writeoutputs<<<
        }
        void finalize()
        {
            //*******************************************//
            //             *** WARNING ***               //
            //                                           //
            //         AUTO GENERATED COMPONENT!         //
            // ANY CHANGES WILL BE LOST IF RE-GENERATED! //
            //*******************************************//

            //cleanup
            mFMU.terminateSlave(c);
            mFMU.freeSlaveInstance(c);
        }

        bool loadLib(std::string path)
        {
            //*******************************************//
            //             *** WARNING ***               //
            //                                           //
            //         AUTO GENERATED COMPONENT!         //
            // ANY CHANGES WILL BE LOST IF RE-GENERATED! //
            //*******************************************//

            bool success = true;
            void *h;
            std::string libdir = path;
            while(libdir.at(libdir.size()-1) != '/')
            {
            libdir.erase(libdir.size()-1, 1);
            }
#ifdef WIN32
            SetDllDirectoryA(libdir.c_str());       //Set search path for dependencies\n");
            h = LoadLibraryA(path.c_str());
#elif linux
            h = dlopen(path.c_str(), RTLD_LAZY);
            std::cout << dlerror();
#endif
            if (!h)
            {
                success = false; // failure
                return success;
            }
            mFMU.dllHandle = h;

            mFMU.getTypesPlatform   = (fGetTypesPlatform)   getAdr(&success, "fmiGetTypesPlatform");
            mFMU.instantiateSlave   = (fInstantiateSlave)   getAdr(&success, "fmiInstantiateSlave");
            mFMU.freeSlaveInstance  = (fFreeSlaveInstance)  getAdr(&success, "fmiFreeSlaveInstance");
            mFMU.initializeSlave    = (fInitializeSlave)    getAdr(&success, "fmiInitializeSlave");
            mFMU.terminateSlave     = (fTerminateSlave)     getAdr(&success, "fmiTerminateSlave");
            mFMU.resetSlave         = (fResetSlave)         getAdr(&success, "fmiResetSlave");
            mFMU.doStep             = (fDoStep)             getAdr(&success, "fmiDoStep");
            mFMU.cancelStep         = (fCancelStep)         getAdr(&success, "fmiCancelStep");

            mFMU.getVersion         = (fGetVersion)         getAdr(&success, "fmiGetVersion");
            mFMU.setDebugLogging    = (fSetDebugLogging)    getAdr(&success, "fmiSetDebugLogging");
            mFMU.setReal            = (fSetReal)            getAdr(&success, "fmiSetReal");
            mFMU.setInteger         = (fSetInteger)         getAdr(&success, "fmiSetInteger");
            mFMU.setBoolean         = (fSetBoolean)         getAdr(&success, "fmiSetBoolean");
            mFMU.setString          = (fSetString)          getAdr(&success, "fmiSetString");
            mFMU.getReal            = (fGetReal)            getAdr(&success, "fmiGetReal");
            mFMU.getInteger         = (fGetInteger)         getAdr(&success, "fmiGetInteger");
            mFMU.getBoolean         = (fGetBoolean)         getAdr(&success, "fmiGetBoolean");
            mFMU.getString          = (fGetString)          getAdr(&success, "fmiGetString");
            return success;
        }

        void* getAdr(bool* s, const char* functionName)
        {
            //*******************************************//
            //             *** WARNING ***               //
            //                                           //
            //         AUTO GENERATED COMPONENT!         //
            // ANY CHANGES WILL BE LOST IF RE-GENERATED! //
            //*******************************************//

            char name[BUFSIZE];
            void* fp;
            sprintf(name, "%s_%s", getModelIdentifier(mFMU.modelDescription), functionName);
#ifdef WIN32
            fp = (void*)GetProcAddress(static_cast<HINSTANCE__*>(mFMU.dllHandle), name);
#else
            fp = dlsym(mFMU.dllHandle, name);
#endif
            if (!fp)
            {
                *s = false; // mark dll load as 'failed'
            }
            return fp;
        }

        void simulateFMU()
        {
            //*******************************************//
            //             *** WARNING ***               //
            //                                           //
            //         AUTO GENERATED COMPONENT!         //
            // ANY CHANGES WILL BE LOST IF RE-GENERATED! //
            //*******************************************//
            fmiStatus fmiFlag = mFMU.doStep(c, mTime, mTimestep, fmiTrue);
            if (fmiFlag != fmiOK)  
            {
                addErrorMessage("Could not complete simulation of the model.");
                stopSimulation();
                return;
            }
        }
    };
}

#endif // <<<modelname>>>_H
