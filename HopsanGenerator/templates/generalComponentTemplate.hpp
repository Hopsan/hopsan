/*-----------------------------------------------------------------------------
 This source file is a part of Hopsan

 Copyright (c) 2009 to present year, Hopsan Group

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 For license details and information about the Hopsan Group see the files
 GPLv3 and HOPSANGROUP in the Hopsan source code root directory

 For author and contributor information see the AUTHORS file
-----------------------------------------------------------------------------*/

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

        //Finalize
        void deconfigure()
        {
            //*******************************************//
            //             *** WARNING ***               //
            //                                           //
            //         AUTO GENERATED COMPONENT!         //
            // ANY CHANGES WILL BE LOST IF RE-GENERATED! //
            //*******************************************//

            <<<deconfcode>>>
        }

        <<<auxiliaryfunctions>>>
    };
}

#endif // <<<uppertypename>>>_HPP_INCLUDED
