#ifndef <<<modelname>>>_H
#define <<<modelname>>>_H

//*******************************************//
//             *** WARNING ***               //
//                                           //
//         AUTO GENERATED COMPONENT!         //
// ANY CHANGES WILL BE LOST IF RE-GENERATED! //
//*******************************************//


#define BUFSIZE 4096

#define _WIN32_WINNT 0x0502
#include "../fmi_me.h"
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
        int nx;                          // number of state variables
        int nz;                          // number of state event indicators
        double *x;                       // continuous states
        double *xdot;                    // the crresponding derivatives in same order
        double *z;                // state event indicators
        double *prez;             // previous values of state event indicators

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
            fmiReal t0 = 0;                  // start time
            fmiBoolean toleranceControlled = fmiFalse;
            int loggingOn = 0;

            // instantiate the fmu
            md = mFMU.modelDescription;
            guid = getString(md, att_guid);
            callbacks.logger = fmuLogger;
            callbacks.allocateMemory = calloc;
            callbacks.freeMemory = free;
            c = mFMU.instantiateModel(getModelIdentifier(md), guid, callbacks, loggingOn);

            // allocate memory
            nx = getNumberOfStates(md);
            nz = getNumberOfEventIndicators(md);
            x    = (double *) calloc(nx, sizeof(double));
            xdot = (double *) calloc(nx, sizeof(double));
            if (nz>0)
            {
                z    =  (double *) calloc(nz, sizeof(double));
                prez =  (double *) calloc(nz, sizeof(double));
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

            // set the start time and initialize
            fmiFlag =  mFMU.setTime(c, t0);
            fmiFlag =  mFMU.initialize(c, toleranceControlled, t0, &eventInfo);
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
            mFMU.terminate(c);
            mFMU.freeModelInstance(c);
            if (x!=NULL) free(x);
            if (xdot!= NULL) free(xdot);
            if (z!= NULL) free(z);
            if (prez!= NULL) free(prez);
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

            mFMU.getModelTypesPlatform   = (fGetModelTypesPlatform) getAdr(&success, "fmiGetModelTypesPlatform");
            mFMU.instantiateModel        = (fInstantiateModel)   getAdr(&success, "fmiInstantiateModel");
            mFMU.freeModelInstance       = (fFreeModelInstance)  getAdr(&success, "fmiFreeModelInstance");
            mFMU.setTime                 = (fSetTime)            getAdr(&success, "fmiSetTime");
            mFMU.setContinuousStates     = (fSetContinuousStates)getAdr(&success, "fmiSetContinuousStates");
            mFMU.completedIntegratorStep = (fCompletedIntegratorStep)getAdr(&success, "fmiCompletedIntegratorStep");
            mFMU.initialize              = (fInitialize)         getAdr(&success, "fmiInitialize");
            mFMU.getDerivatives          = (fGetDerivatives)     getAdr(&success, "fmiGetDerivatives");
            mFMU.getEventIndicators      = (fGetEventIndicators) getAdr(&success, "fmiGetEventIndicators");
            mFMU.eventUpdate             = (fEventUpdate)        getAdr(&success, "fmiEventUpdate");
            mFMU.getContinuousStates     = (fGetContinuousStates)getAdr(&success, "fmiGetContinuousStates");
            mFMU.getNominalContinuousStates = (fGetNominalContinuousStates)getAdr(&success, "fmiGetNominalContinuousStates");
            mFMU.getStateValueReferences = (fGetStateValueReferences)getAdr(&success, "fmiGetStateValueReferences");
            mFMU.terminate               = (fTerminate)          getAdr(&success, "fmiTerminate");

            mFMU.getVersion              = (fGetVersion)         getAdr(&success, "fmiGetVersion");
            mFMU.setDebugLogging         = (fSetDebugLogging)    getAdr(&success, "fmiSetDebugLogging");
            mFMU.setReal                 = (fSetReal)            getAdr(&success, "fmiSetReal");
            mFMU.setInteger              = (fSetInteger)         getAdr(&success, "fmiSetInteger");
            mFMU.setBoolean              = (fSetBoolean)         getAdr(&success, "fmiSetBoolean");
            mFMU.setString               = (fSetString)          getAdr(&success, "fmiSetString");
            mFMU.getReal                 = (fGetReal)            getAdr(&success, "fmiGetReal");
            mFMU.getInteger              = (fGetInteger)         getAdr(&success, "fmiGetInteger");
            mFMU.getBoolean              = (fGetBoolean)         getAdr(&success, "fmiGetBoolean");
            mFMU.getString               = (fGetString)          getAdr(&success, "fmiGetString");
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

            int i;                          // For loop index
            fmiBoolean timeEvent, stateEvent, stepEvent;
            fmiStatus fmiFlag;               // return code of the fmu functions

            if (eventInfo.terminateSimulation)
            {
                stopSimulation();
            }

            //Simulate one step

            // get current state and derivatives
            fmiFlag = mFMU.getContinuousStates(c, x, nx);
            fmiFlag = mFMU.getDerivatives(c, xdot, nx);

            // advance time
            timeEvent = eventInfo.upcomingTimeEvent && eventInfo.nextEventTime < mTime;
            fmiFlag = mFMU.setTime(c, mTime);

            // perform one step
            for (i=0; i<nx; i++) x[i] += mTimestep*xdot[i]; // forward Euler method
            fmiFlag = mFMU.setContinuousStates(c, x, nx);

            // Check for step event, e.g. dynamic state selection
            fmiFlag = mFMU.completedIntegratorStep(c, &stepEvent);

            // Check for state event
            for (i=0; i<nz; i++) prez[i] = z[i];
            fmiFlag = mFMU.getEventIndicators(c, z, nz);
            stateEvent = FALSE;
            for (i=0; i<nz; i++)
            {
                stateEvent = stateEvent || (prez[i] * z[i] < 0);
            }

            // handle events
            if (timeEvent || stateEvent || stepEvent)
            {
                // event iteration in one step, ignoring intermediate results
                fmiFlag = mFMU.eventUpdate(c, fmiFalse, &eventInfo);
                // terminate simulation, if requested by the model
                if (eventInfo.terminateSimulation)
                {
                    stopSimulation();
                }
            } // if event
        }
    };
}

#endif // <<<modelname>>>_H
