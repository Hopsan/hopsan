#ifndef <<<uppertypename>>>_HPP_INCLUDED
#define <<<uppertypename>>>_HPP_INCLUDED\n

//*******************************************//
//             *** WARNING ***               //
//                                           //
//         AUTO GENERATED COMPONENT!         //
// ANY CHANGES WILL BE LOST IF RE-GENERATED! //
//*******************************************//

#include <math.h>
#include "ComponentEssentials.h"
#include "ComponentUtilities.h"
#include <sstream>
#include <cstring>
#include <vector>
#include <string>

using namespace std;

namespace hopsan {

    class <<<typename>>> : public Component<<<cqstype>>>
    {
    private:                         // Private section
<<<vardecl>>>
<<<dataptrdecl>>>
<<<portdecl>>>

    public:                              //Public section
        static Component *Creator()
        {
            return new <<<typename>>>();
        }
        
        //Configure
        void configure()
        {
            //*******************************************//
            //             *** WARNING ***               //
            //                                           //
            //         AUTO GENERATED COMPONENT!         //
            // ANY CHANGES WILL BE LOST IF RE-GENERATED! //
            //*******************************************//

<<<regpar>>>

<<<addports>>>

            <<<confcode>>>
        }
        
        //Initialize
        void initialize()
        {
            //*******************************************//
            //             *** WARNING ***               //
            //                                           //
            //         AUTO GENERATED COMPONENT!         //
            // ANY CHANGES WILL BE LOST IF RE-GENERATED! //
            //*******************************************//
            
            <<<initvars>>>

            <<<getdataptrs>>>

            <<<readinputs>>>

            <<<initcode>>>
            
            <<<writeoutputs>>>
        }

        //Simulate one time step
        void simulateOneTimestep()
        {
            //*******************************************//
            //             *** WARNING ***               //
            //                                           //
            //         AUTO GENERATED COMPONENT!         //
            // ANY CHANGES WILL BE LOST IF RE-GENERATED! //
            //*******************************************//
            
            <<<readinputs>>>
            
            <<<simulatecode>>>

            <<<writeoutputs>>>
        }

        //Finalize
        void finalize()
        {
            //*******************************************//
            //             *** WARNING ***               //
            //                                           //
            //         AUTO GENERATED COMPONENT!         //
            // ANY CHANGES WILL BE LOST IF RE-GENERATED! //
            //*******************************************//
           
            <<<finalcode>>>
        }
        <<<auxiliaryfunctions>>>
    };
}

#endif // <<<uppertypename>>>_HPP_INCLUDED
