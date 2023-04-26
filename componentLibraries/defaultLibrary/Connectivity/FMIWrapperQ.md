### Description
![FMIWrapper picture](FMIWrapper.svg)

This Q-type component encapsulates an imported FMU for co-simulation. All versions of FMI (1, 2 and 3) are supported. Select the FMU file using the "path" parameter. 
         
            

#### FMU Function Call Sequence
The FMU is instantiated once the "path" parameter is set and will remain instantiated until the parameter is changed or the component is removed. 

The FMU is initialized once before every simulation and reset after every simulation.

During simulation the doStep function is called exactly ones for every simulation step in Hopsan. Inputs are written before the step and outputs are read when the step is complete.

#### TLM Ports
Inputs and outputs can be grouped to TLM ports using the "ports" parameter. Variables are separated by commas and ports by semicolon. 
            
Example:

`NodeHydraulic,q1,p1,T1,c1,Zc1,Q1;NodeMechanic,v2,f2,x2,c2,Zc2,me2;NodeElectric,u3,i3,c3,Zc3;NodePneumatic,Qdot4,p4,c4,Zc4,mdot4,rho4,crho4,Zcrho4,T4`

**NodeHydraulic**

| Variable    | Description          | Unit      | Causality |
| :---------- | :------------------- | :-------- | :-------- |
| q           | Volume flow          | [m^3/s]   | out       |
| p           | Absolute pressure    | [Pa]      | out       |
| T           | Absolute temperature | [K]       | out       |
| c           | TLM wave variable    | [Pa]      | in        |
| Zc          | TLM impedance        | [sPa/m^3] | in        |
| Q1          | Heat flow            | [W]       | out       |

**NodeMechanic**

| Variable    | Description       | Unit   | Causality |
| :---------- | :---------------- | :------| :-------- |
| v           | Velocity          | [m/s]  | out       |     
| f           | Force             | [N]    | out       | 
| x           | Position          | [m]    | out       | 
| c           | TLM wave variable | [N]    | in        |
| Zc          | TLM impedance     | [Ns/m] | in        |
| me          | Equivalent mass   | [kg]   | out       |

**NodeElectric**

| Variable    | Description       | Unit  | Causality |
| :---------- | :---------------- | :-----| :-------- |
| u           | Voltage           | [V]   | out       |     
| i           | Current           | [A]   | out       | 
| c           | TLM wave variable | [N]   | in        |
| Zc          | TLM impedance     | [V/A] | in        |

**NodeElectric**

| Variable    | Description           | Unit     | Causality |
| :---------- | :-------------------- | :------- | :-------- |
| Qdot4       | Energy flow           | [J/s]    | out       |
| p4          | Pressure              | [Pa]     | out       |
| c4          | TLM wave variable     | [Pa]     | in        |
| Zc4         | TLM Impedance         | [s/m^3]  | in        |
| mdot4       | Mass flow             | [kg/s]   | out       |
| rho4        | Density               | [kg/m^3] | out       |
| crho4       | Density wave variable | [kg/m^3] | in        |
| Zcrho4      | Density impedance     | [s/m^3]  | in        |
| T4          | Temperature           | [K]      | out       |
