### Description
![FMIWrapper picture](FMIWrapper.svg)

This signal type component encapsulates an imported FMU for co-simulation. All versions of FMI (1, 2 and 3) are supported. Select the FMU file using the "path" parameter. 
         
            

#### FMU Function Call Sequence
The FMU is instantiated once the "path" parameter is set and will remain instantiated until the parameter is changed or the component is removed. 

The FMU is initialized once before every simulation and reset after every simulation.

During simulation the doStep function is called exactly ones for every simulation step in Hopsan. Inputs are written before the step and outputs are read when the step is complete.
