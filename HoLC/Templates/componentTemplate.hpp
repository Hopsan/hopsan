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


