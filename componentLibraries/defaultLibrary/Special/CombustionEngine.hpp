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

#ifndef COMBUSTIONENGINE_HPP_INCLUDED
#define COMBUSTIONENGINE_HPP_INCLUDED

#include <math.h>
#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

using namespace std;

namespace hopsan {

class CombustionEngine : public ComponentC
{
private:
    double w1, T1, a1, c1, Zc1, in;
    double P_max;
    HString Characteristics;
    CSVParserNG mDataFile;
    LookupTable1D mLookupTable;
    double *mpND_w1, *mpND_T1, *mpND_a1, *mpND_c1, *mpND_Zc1, *mpND_in;
    Port *mpin, *mpP1;

public:
    static Component *Creator()
    {
        return new CombustionEngine();
    }

    void configure()
    {
        addConstant("P_max", "Max Power", "W", 150000, P_max);
        addConstant("Characteristics", "Torque-Speed Characteristics", "", "../componentLibraries/defaultLibrary/Special/CombustionEngine.csv",Characteristics);

        mpin = addReadPort("in", "NodeSignal");

        setDefaultStartValue(mpin, NodeSignal::Value, 0.00);
        mpP1 = addPowerPort("P1", "NodeMechanicRotational");
    }

    void initialize()
    {
        mpND_in = getSafeNodeDataPtr(mpin, NodeSignal::Value);
        mpND_w1 = getSafeNodeDataPtr(mpP1, NodeMechanicRotational::AngularVelocity);
        mpND_T1 = getSafeNodeDataPtr(mpP1, NodeMechanicRotational::Torque);
        mpND_a1 = getSafeNodeDataPtr(mpP1, NodeMechanicRotational::Angle);
        mpND_c1 = getSafeNodeDataPtr(mpP1, NodeMechanicRotational::WaveVariable);
        mpND_Zc1 = getSafeNodeDataPtr(mpP1, NodeMechanicRotational::CharImpedance);

        bool success=false;
        mLookupTable.clear();
        success = mDataFile.openFile(findFilePath(Characteristics));
    	if(!success)
    	{
            addErrorMessage("Unable to initialize CSV file: "+Characteristics+", "+mDataFile.getErrorString());
        	stopSimulation();
    	}
	    else
	    {
            mDataFile.indexFile();
            success = mDataFile.copyColumn(0, mLookupTable.getIndexDataRef());
            success = success && mDataFile.copyColumn(1, mLookupTable.getValueDataRef());
            // Now the data is in the lookuptable and we can close the csv file and clear the index
            mDataFile.closeFile();

            if(!success)
            {
                addErrorMessage("Unable to initialize lookup table from CSV file: "+Characteristics+", "+"Could not copy index / data columns");
                stopSimulation();
            }

            // Make sure strictly increasing (no sorting will be done if that is already the case)
            mLookupTable.sortIncreasing();
            success = success && mLookupTable.isDataOK() && mLookupTable.allIndexStrictlyIncreasing();
	        if(!success)
	        {
                addErrorMessage("Unable to initialize lookup table from CSV file: "+Characteristics+", "+"Even after sorting, index column is still not strictly increasing");
	            stopSimulation();
	        }
	    }

        (*mpND_c1) = 0;
        (*mpND_Zc1) = 0;
    }

    void simulateOneTimestep()
    {
        in = (*mpND_in);
        w1 = (*mpND_w1);

        double T_max = mLookupTable.interpolate(w1);
        c1=limit(-in*P_max/w1,0,T_max);

        (*mpND_c1) = c1;
        (*mpND_Zc1) = 0;
    }

    void finalize()
    {
        mLookupTable.clear();
    }
};
}

#endif // COMBUSTIONENGINE_HPP_INCLUDED
