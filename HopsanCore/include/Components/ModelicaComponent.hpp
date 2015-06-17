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

//!
//! @file   ModelicaComponent.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2014-05-03
//!
//! @brief Contains an empty component to use for uncompiled Modelica components
//!
//$Id$

#ifndef MODELICACOMPONENT_HPP_INCLUDED
#define MODELICACOMPONENT_HPP_INCLUDED

#include "ComponentEssentials.h"

namespace hopsan {

    class ModelicaComponent : public ComponentSignal
    {
    private:
        HString mModel;
        HString mPorts;
        HString mParameterNames;
        HString mParameterDefaults;
        double mParametersValues[100];  //!< @todo Horrible! But for some reason it crashes when using std::vector...
        //! @note This can be solved by using a vector and reserving the size instead of using push_back; push_back moves the whole vector to a new address which ruins the previous references...

    public:
        static Component *Creator()
        {
            return new ModelicaComponent();
        }

        void configure()
        {
            if(!this->hasParameter("model"))
            {
                this->addConstant("model", "Modelica model", "-", "", mModel);
                this->addConstant("ports", "Number of ports", "-", "", mPorts);
                this->addConstant("parameters", "Parameter names", "-", "", mParameterNames);
                this->addConstant("defaults", "Default parameter values", "-", "", mParameterDefaults);
            }
            else
            {
                //Generate list of new ports, based on string parameter
                std::vector<HString> newPortNames;
                newPortNames.push_back("");
                for(size_t i=0; i<mPorts.size(); ++i)
                {
                    if(mPorts.at(i) != ',')
                    {
                        newPortNames[newPortNames.size()-1].append(mPorts.at(i));
                    }
                    else
                    {
                        newPortNames.push_back("");
                    }
                }

                //Remove old ports if they do not exist in list of new ports
                for(size_t p=0; p<getPortNames().size(); ++p)
                {
                    bool found=false;
                    for(size_t j=0; j<newPortNames.size(); ++j)
                    {
                        if(newPortNames.at(j) == getPortNames().at(p))
                        {
                            found = true;       //Old port exists in list of new ports, so keep it
                        }
                    }
                    if(!found)
                    {
                        removePort(getPortNames().at(p)); //Old port does not exist among new ports, so remove it
                        --p;
                    }
                }

                //Add new ports if they do not already exist
                for(size_t p=0; p<newPortNames.size(); ++p)
                {
                    if(!getPort(newPortNames.at(p)))
                    {
                        addReadPort(newPortNames.at(p), "NodeModelica", "");
                    }
                }

                //Generate list of constants
                std::vector<HString> newConstants;
                newConstants.push_back("");
                for(size_t i=0; i<mParameterNames.size(); ++i)
                {
                    if(mParameterNames.at(i) != ',')
                    {
                        newConstants[newConstants.size()-1].append(mParameterNames.at(i));
                    }
                    else
                    {
                        newConstants.push_back("");
                    }
                }

                //Generate list of constants default values
                //std::vector<double> newDefaults;
                //! @todo Implement default value support (need to parse CSV string to vector of doubles somehow)


                //Unregister old constants if they do not exist in list of new constants
                std::vector<HString> oldConstants;
                getParameterNames(oldConstants);
                size_t numOldConstants = 0;
                for(size_t c=0; c<oldConstants.size(); ++c)
                {
                    bool found=false;
                    for(size_t j=0; j<newConstants.size(); ++j)
                    {
                        if(newConstants[j] == oldConstants.at(c) || oldConstants.at(c) == "model" || oldConstants.at(c) == "ports" ||
                                oldConstants.at(c) == "parameters" || oldConstants.at(c) == "defaults")
                        {
                            found = true;       //Old port exists in list of new ports, so keep it
                            ++numOldConstants;
                        }
                    }
                    if(!found)
                    {
                        unRegisterParameter(oldConstants.at(c));
                    }
                }

                //Add new ports if they do not already exist
                for(size_t p=0; p<newConstants.size(); ++p)
                {
                    if(!newConstants[p].empty() && !getParameter(newConstants[p]))
                    {
                        mParametersValues[numOldConstants+p] = 0.0;
                        addConstant(newConstants[p], "-", "-", 0.0, mParametersValues[numOldConstants+p]);
                    }
                }
            }
        }


        void initialize()
        {
            // Do nothing
        }


        void simulateOneTimestep()
        {
            // Do nothing
        }
    };
}

#endif //MODELICACOMPONENT_HPP_INCLUDED
