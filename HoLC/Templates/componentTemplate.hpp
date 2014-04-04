#ifndef <<<headerguard>>>
#define <<<headerguard>>>

#include <iostream>
#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    class <<<typename>>> : public Component<<<cqstype>>>
    {
    private:<<<variables>>>

    public:
        static Component *Creator()
        {
            return new <<<typename>>>();
        }

        void configure()
        {<<<addconstants>>><<<addinputs>>><<<addoutputs>>><<<setdefaultstartvalues>>><<<addports>>>
        }


        void initialize()
        {<<<getdataptrs>>>

            //Get variable values from nodes<<<readfromnodes>>>

            //WRITE INITIALIZATION HERE

            //Write new values to nodes<<<writetonodes>>>
        }


        void simulateOneTimestep()
        {
            //Get variable values from nodes<<<readfromnodes>>>

            //WRITE EQUATIONS HERE

            //Write new values to nodes<<<writetonodes>>>
        }


        void finalize()
        {
            //WRITE FINALIZE CODE HERE
        }


        void deconfigure()
        {
            //WRITE DECONFIGURATION CODE HERE
        }
    };
}

#endif //<<<headerguard>>>


