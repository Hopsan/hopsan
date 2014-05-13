/*-----------------------------------------------------------------------------
 This source file is part of Hopsan NG

 Copyright (c) 2011 
    Mikael Axin, Robert Braun, Alessandro Dell'Amico, Björn Eriksson,
    Peter Nordin, Karl Pettersson, Petter Krus, Ingo Staack

 This file is provided "as is", with no guarantee or warranty for the
 functionality or reliability of the contents. All contents in this file is
 the original work of the copyright holders at the Division of Fluid and
 Mechatronic Systems (Flumes) at Linköping University. Modifying, using or
 redistributing any part of this file is prohibited without explicit
 permission from the copyright holders.
-----------------------------------------------------------------------------*/

//!
//! @file   ModelicaComponent.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2014-05-03
//!
//! @brief Contains an empty component to use for uncompiled Modelica components
//!
//$Id: DummyComponent.hpp 4622 2012-08-20 07:53:01Z petno25 $

#ifndef MODELICACOMPONENT_HPP_INCLUDED
#define MODELICACOMPONENT_HPP_INCLUDED

#include "ComponentEssentials.h"

namespace hopsan {

    class ModelicaComponent : public ComponentSignal
    {
    private:
        HString mModel;
        HString mPorts;
        HString mParameters;
        HString mDefaults;
        std::vector<double> mParametersValues;

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
                this->addConstant("parameters", "Parameter names", "-", "", mParameters);
                this->addConstant("defaults", "Default parameter values", "-", "", mDefaults);
            }
            else if(mPorts!="")
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
                            found = true;       //Old pöort exists in list of new ports, so keep it
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
                for(size_t i=0; i<mParameters.size(); ++i)
                {
                    if(mParameters.at(i) != ',')
                    {
                        newConstants[newConstants.size()-1].append(mParameters.at(i));
                    }
                    else
                    {
                        newConstants.push_back("");
                    }
                }

                //Geerate list of constants default values
                std::vector<double> newDefaults;
                //! @todo Implement default value support (need to parse CSV string to vector of doubles somehow)


                //Unregister old constants if they do not exist in list of new constants
                std::vector<HString> oldConstants;
                getParameterNames(oldConstants);
                for(size_t c=0; c<oldConstants.size(); ++c)
                {
                    bool found=false;
                    for(size_t j=0; j<newConstants.size(); ++j)
                    {
                        if(newConstants.at(j) == oldConstants.at(c) || oldConstants.at(c) == "model" || oldConstants.at(c) == "ports" ||
                                oldConstants.at(c) == "parameters" || oldConstants.at(c) == "defaults")
                        {
                            found = true;       //Old pöort exists in list of new ports, so keep it
                        }
                    }
                    if(!found)
                    {
                        unRegisterParameter(oldConstants.at(c));
                        --c;
                    }
                }

                //Add new ports if they do not already exist
                for(size_t p=0; p<newConstants.size(); ++p)
                {
                    if(!getParameter(newConstants.at(p)))
                    {
                        mParametersValues.push_back(0);
                        addConstant(newConstants.at(p), "", "-", 0, mParametersValues.at(mParametersValues.size()-1));
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
