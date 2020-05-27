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

//!
//! @file   AeroFlightRecorder.hpp
//! @author Alejandro Sobron <alejandro.sobron@liu.se>
//! @date   2020-05-27
//!
//! @brief Contains a log-file generator component for geographical visualization
//! @ingroup AeroComponents
//!

#ifndef AEROFLIGHTRECORDER_HPP_INCLUDED
#define AEROFLIGHTRECORDER_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentSystem.h"
#include "ComponentUtilities.h"
#include "math.h"
#include <iostream>
#include <fstream>
#include <ctime>

namespace hopsan {

    class AeroFlightRecorder : public ComponentSignal
    {

    private:
    // Private member variables

        //inputVariables pointers
        double *mpLongitude;
        double *mpLatitude;
        double *mpAltitude;
        double *mpPhi;
        double *mpTheta;
        double *mpPsi;
        double *mpV;

        //inputParameters pointers
        double *mpDt;
        double *mpScaleX;
        double *mpScaleY;
        double *mpScaleZ;

        // Constants and other operating variables
        enum FileTypes {FlightPathKML, GroundPathKML, ThreeDmodelKML};
        int mFileType;
        HString mFilePath;
        HString mDaeModelPath;
        std::ofstream mFile; 
        double mLastLogTime;

    public:
    // The creator function that is registered when a component lib is loaded into Hopsan
        static Component *Creator()
        {
            return new AeroFlightRecorder();
        }

        void configure()
        {
            //Add ports to the component

            //Add inputVariables to the component
            addInputVariable("longitude","Longitude E","deg",15.577,&mpLongitude); //Note quantity would be "Angle"
            addInputVariable("latitude","Latitude N","deg",58.401,&mpLatitude); //Note quantity would be "Angle"
            addInputVariable("altitude","Altitude above MSL","Altitude",0,&mpAltitude);
            addInputVariable("phi","Roll","Angle",0,&mpPhi);
            addInputVariable("theta","Pitch","Angle",0,&mpTheta);
            addInputVariable("psi","Heading","Angle",0,&mpPsi);
            addInputVariable("v","Speed","Velocity",0,&mpV);

            // Add inputParameters
            addInputVariable("dt", "Logging interval (0 = log every sample)", "Time", 1, &mpDt);
            addInputVariable("scaleX", "X-axis scaling factor for .dae 3D model", "", 1, &mpScaleX);
            addInputVariable("scaleY", "Y-axis scaling factor for .dae 3D model", "", 1, &mpScaleY);
            addInputVariable("scaleZ", "Z-axis scaling factor for .dae 3D model", "", 1, &mpScaleZ);

            // Add conditional constant for choice of file type
            std::vector<HString> filetypes;
            filetypes.push_back("Flight path KML");
            filetypes.push_back("Ground path KML");
            filetypes.push_back("3D vehicle KML");
            addConditionalConstant("filetype", "Output file type", filetypes,0,mFileType);

            // Add constants
            addConstant("path", "Path where the output file should be placed", "", "", mFilePath);
            addConstant("daeModel", "File path to a .dae model for 3D vehicle representation", "", "", mDaeModelPath);
        }


        void initialize()
        {
            //Check if output file path has been specified
            if (mFilePath.empty()) {
                stopSimulation("Please specify a path for the output file.");
                return;
            }

            //Fetch parent system name (hmf filename is unknown)
            HString modelName = getSystemParent()->getName();

            #ifdef _WIN32
                const HString sep = "\\";
            #else
                const HString sep = "/";
            #endif

            //Open file and write file headers depending on file type
            if(mFileType == FlightPathKML) {
                mFile.open((mFilePath.append(sep+modelName+".kml")).c_str());
                if (!mFile.is_open()) {
                    stopSimulation("Could not find, open, or write an output file in the given path: "+ mFilePath);
                    return;
                }
                mFile << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                         "<kml xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">\n"
                         "<Document>\n"
                         "  <open>1</open>\n"
                         "  <Style id=\"yellowLineGreenPoly\">\n"
                         "    <PolyStyle>\n"
                         "      <color>7F00FF00</color>\n"
                         "      <colorMode>normal</colorMode>\n"
                         "    </PolyStyle>\n"
                         "    <LineStyle>\n"
                         "      <color>FF00A5FF</color>\n"
                         "      <colorMode>normal</colorMode>\n"
                         "      <width>4</width>\n"
                         "    </LineStyle>\n"
                         "  </Style>\n"
                         "  <Folder>\n"
                         "    <name>SimResults</name>\n"
                         "    <Placemark>\n"
                         "      <name>Flight Path</name>\n"
                         "      <styleUrl>#yellowLineGreenPoly</styleUrl>\n"
                         "      <LineString>\n"
                         "        <extrude>1</extrude>\n"
                         "        <altitudeMode>absolute</altitudeMode>\n" // alternatively: relativeToGround
                         "        <coordinates>\n";
            }

            else if(mFileType == GroundPathKML) {
                mFile.open((mFilePath.append(sep+modelName+".kml")).c_str());
                if (!mFile.is_open()) {
                    stopSimulation("Could not find, open, or write an output file in the given path: "+ mFilePath);
                    return;
                }
                mFile << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                         "<kml xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">\n"
                         "<Document>\n"
                         "  <open>1</open>\n"
                         "  <Style id=\"redLineRedPoly\">\n"
                         "    <PolyStyle>\n"
                         "      <color>4C0000FF</color>\n"
                         "      <colorMode>normal</colorMode>\n"
                         "    </PolyStyle>\n"
                         "    <LineStyle>\n"
                         "      <color>4C0000FF</color>\n"
                         "      <colorMode>normal</colorMode>\n"
                         "      <width>4</width>\n"
                         "    </LineStyle>\n"
                         "  </Style>\n"
                         "  <Folder>\n"
                         "    <name>SimResults</name>\n"
                         "    <Placemark>\n"
                         "      <name>Ground Path</name>\n"
                         "      <styleUrl>#redLineRedPoly</styleUrl>\n"
                         "      <LineString>\n"
                         "        <altitudeMode>clampToGround</altitudeMode>\n"
                         "        <coordinates>\n";
            }

            else if(mFileType == ThreeDmodelKML) {
                if (mDaeModelPath.empty()) {
                stopSimulation("A 3D vehicle KML is requested but no valid .dae model has been specified. Please read the documentation.");
                return;
                }
                mFile.open((mFilePath.append(sep+modelName+".kml")).c_str());
                if (!mFile.is_open()) {
                    stopSimulation("Could not find, open, or write an output file in the given path: "+ mFilePath);
                    return;
                }
                mFile << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                         "<kml xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">\n"
                         "<Document>\n"
                         "  <open>1</open>\n"
                         "  <Folder>\n"
                         "    <name>SimResults</name>\n"
                         "    <Folder>\n"
                         "      <name>3-D Aircraft</name>\n";
            }

            // Call per-step function
            mLastLogTime = mTime-(*mpDt);
            simulateOneTimestep();
        }


        void simulateOneTimestep()
        {
            //Check if it is time to run
            if(mTime < mLastLogTime+(*mpDt)) {
                return;
            }
            mLastLogTime = mTime;

            //Read variables from nodes
            double longitude = (*mpLongitude);
            double latitude = (*mpLatitude);
            double altitude = (*mpAltitude);

            //Write the data according to selected type
            if(mFileType == FlightPathKML) {
                mFile << "        " << longitude << "," << latitude << "," << altitude << "\n";
            }

            else if(mFileType == GroundPathKML) {
                mFile << "        " << longitude << "," << latitude << "," << altitude << "\n";
            }

            else if(mFileType == ThreeDmodelKML) {
                double roll = -rad2deg(*mpPhi);
                double pitch = rad2deg(*mpTheta);
                double heading = rad2deg(*mpPsi);
                double speed = (*mpV) * 3.6; //Convert from m/s to km/h
                double tilt = -rad2deg(*mpTheta);
                double scaleX = (*mpScaleX);
                double scaleY = (*mpScaleY);
                double scaleZ = (*mpScaleZ);
                mFile << "      <Placemark>\n"
                         "        <name>Time" << mTime << "s</name>\n"
                         "        <visibility>1</visibility>\n"
                         "        <description><![CDATA[Alt: " << altitude << "m Spd: " << speed << "km/h<br>\n"
                         "                     Roll: " << roll << " deg\n"
                         "                     Pitch: " << pitch << " deg\n"
                         "                     Hdg: " << heading << " deg<br>]]>\n"
                         "        </description>\n"
                         "        <Model>\n"
                         "          <altitudeMode>absolute</altitudeMode>\n" // alternatively: relativeToGround
                         "          <Location>\n"
                         "            <latitude>" << latitude << "</latitude>\n"
                         "            <longitude>" << longitude << "</longitude>\n"
                         "            <altitude>" << altitude << "</altitude>\n"
                         "          </Location>\n"
                         "          <Orientation>\n"
                         "            <heading>" << heading << "</heading>\n"
                         "            <tilt>" << tilt << "</tilt>\n"
                         "            <roll>" << roll << "</roll>\n"
                         "          </Orientation>\n"
                         "          <Scale>\n"
                         "            <x>" << scaleY << "</x>\n"
                         "            <y>" << scaleX << "</y>\n"
                         "            <z>" << scaleZ << "</z>\n"
                         "          </Scale>\n"
                         "          <Link>\n"
                         "            <href>" << mDaeModelPath.c_str() << "</href>\n"
                         "          </Link>\n"
                         "        </Model>\n"
                         "      </Placemark>\n";
            }
            
        }


        void finalize()
        {
            //Write endings of KML depending on file type
            if(mFileType == FlightPathKML) {
                mFile << "        </coordinates>\n"
                         "      </LineString>\n"
                         "    </Placemark>\n"
                         "  </Folder>\n"
                         "</Document>\n"
                         "</kml>";
            }

            else if(mFileType == GroundPathKML) {
                mFile << "        </coordinates>\n"
                         "      </LineString>\n"
                         "    </Placemark>\n"
                         "  </Folder>\n"
                         "</Document>\n"
                         "</kml>";
            }

            else if(mFileType == ThreeDmodelKML) {
                mFile << "    </Folder>\n"
                         "  </Folder>\n"
                         "</Document>\n"
                         "</kml>";
            }

            mFile.close();
        }
    };
}

#endif // AEROFLIGHTRECORDER_HPP_INCLUDED
