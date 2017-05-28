/*-----------------------------------------------------------------------------

 Copyright 2017 Hopsan Group

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

 The full license is available in the file GPLv3.
 For details about the 'Hopsan Group' or information about Authors and
 Contributors see the HOPSANGROUP and AUTHORS files that are located in
 the Hopsan source code root directory.

-----------------------------------------------------------------------------*/

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


