### Description
![Flight recorder picture](AeroFlightRecorder.svg)

A logger component that gathers location, altitude and attitude of a vehicle to write a trajectory in an output file that can be opened with geographical visualization software such us Google Earth.

#### Mandatory Inputs
* **filetype** - Output file type [ ] - Choice between different output file types.
* **path** - Output path [ ] - The local path where the output file should be placed. Example: C:\Users\Me\MyFolder
* **dt** - Logging interval [s] - Time between samples, where 0 means one sample per simulation step.
* **longitude** - Longitude [deg] - Longitude (E) in degrees. Example: 15.577
* **latitude** - Latitude [deg] - Latitude (N) in degrees. Example: 58.401
* **altitude** - Altitude [m] - Geometric altitude over mean sea level according to WGS84 reference system (default for Google Earth and other common viewers). Ultimately, the reference system used here should match that of the final tool where the output file will be opened.

#### Optional Inputs (if "3D vehicle KML" filetype is requested)
* **phi** - Roll [rad] - Vehicle's bank angle.
* **pitch** - Pitch [rad] - Vehicle's pitch angle.
* **psi** - Heading [rad] - Vehicle's direction or bearing.
* **v** - Speed [m/s] - Vehicle speed (ground- or air-speed).
* **daeModel** - File path to .dae model [ ] - Local path or URL address to the .dae 3D-model that should be used if "3D vehicle KML" filetype is requested. A basic aircraft model (AeroFlightRecorder.dae) is available inside this component's library (..\Hopsan\componentLibraries\defaultLibrary\Special\AeroComponents\AeroFlightRecorder.dae). Example: C:\Users\Me\MyFolder\MyModel.dae
* **scaleX** - X-axis scaling factor for 3D model [m] - Factor for scaling the X-dimensions of the selected .dae model.
* **scaleY** - Y-axis scaling factor for 3D model [m] - Factor for scaling the Y-dimensions of the selected .dae model.
* **scaleZ** - Z-axis scaling factor for 3D model [m] - Factor for scaling the Z-dimensions of the selected .dae model.

#### Outputs
* **file** - Output file [ ] - File in the requested format (filetype) placed in the requested folder (path) and with the corresponding Hopsan model name. Example: MyHopsanModel.kml

#### Port Initial Conditions
An output file path is required for execution. No other initial conditions are relevant.

<!--- ### Tips--->

### Tips
* Google Earth is the recommended software for visualization of .kml files.
* If a file with the same name and extension already exists in the output path, it will be overwritten without warning during the next execution. If you want to keep it, change its name or its location.
* A .dae 3D model is only needed if the user wants a KML file with a 3D vehicle representation. Keep in mind that one model per sample will be rendered and that geometrically complex models may take a long time to load.

<!---EQUATION  --->

#### References
This component reuses certain parts of code originally included in Robert Braun's "Logger" signal component.
