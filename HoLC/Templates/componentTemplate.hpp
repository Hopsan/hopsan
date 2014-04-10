#ifndef <<<headerguard>>>
#define <<<headerguard>>>

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    class <<<typename>>> : public Component<<<cqstype>>>
    {
    private:
        <<<membervariables>>>

    public:
        static Component *Creator()
        {
            return new <<<typename>>>();
        }

        void configure()
        {
            //Register constant parameters
            <<<addconstants>>>
            //Register input variables
            <<<addinputs>>>
            //Register output variables
            <<<addoutputs>>>
            //Add power ports
            <<<addports>>>
            //Set default power port start values
            <<<setdefaultstartvalues>>>
        }


        void initialize()
        {
            <<<getdataptrs>>>

            <<<localvariables>>>

            //Read variable values from nodes
            <<<readfromnodes>>>

            //WRITE YOUR INITIALIZATION CODE HERE

            //Write new values to nodes
            <<<writetonodes>>>
        }


        void simulateOneTimestep()
        {
            <<<localvariables>>>

            //Read variable values from nodes
            <<<readfromnodes>>>

            //WRITE YOUR EQUATIONS HERE

            //Write new values to nodes
            <<<writetonodes>>>
        }


        void finalize()
        {
            //WRITE YOUR FINALIZE CODE HERE (OPTIONAL)
        }


        void deconfigure()
        {
            //WRITE YOUR DECONFIGURATION CODE HERE (OPTIONAL)
        }
    };
}

#endif //<<<headerguard>>>


