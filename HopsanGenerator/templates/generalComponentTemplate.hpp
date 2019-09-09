/*-----------------------------------------------------------------------------

 Copyright 2017 Hopsan Group

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.


 The full license is available in the file LICENSE.
 For details about the 'Hopsan Group' or information about Authors and
 Contributors see the HOPSANGROUP and AUTHORS files that are located in
 the Hopsan source code root directory.

-----------------------------------------------------------------------------*/

#ifndef <<<uppertypename>>>_HPP_INCLUDED
#define <<<uppertypename>>>_HPP_INCLUDED

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
        //Declare local variables
        <<<vardecl>>>

        //Declare data pointer variables
        <<<dataptrdecl>>>

        //Declare ports
        <<<portdecl>>>

    public:                              //Public section
        static Component *Creator()
        {
            return new <<<typename>>>();
        }
        
        //Configure
        void configure()
        {
            //Register constants
            <<<regpar>>>

            //Add ports
            <<<addports>>>

            //Configuration code
            <<<confcode>>>
        }
        
        //Initialize
        void initialize()
        {
            //Initialize variables
            <<<initvars>>>

            //Get data pointers
            <<<getdataptrs>>>

            //Read input variables
            <<<readinputs>>>

            //Initialization code
            <<<initcode>>>

            //Write output variables
            <<<writeoutputs>>>
        }

        //Simulate one time step
        void simulateOneTimestep()
        {
            //Read input variables
            <<<readinputs>>>

            //Simulation code
            <<<simulatecode>>>

            //Write output variables
            <<<writeoutputs>>>
        }

        //Finalize
        void finalize()
        {
            //Finalize code
            <<<finalcode>>>
        }

        //Finalize
        void deconfigure()
        {
            //Deconfigure code
            <<<deconfcode>>>
        }

        //Auxiliary functions
        <<<auxiliaryfunctions>>>
    };
}

#endif // <<<uppertypename>>>_HPP_INCLUDED
