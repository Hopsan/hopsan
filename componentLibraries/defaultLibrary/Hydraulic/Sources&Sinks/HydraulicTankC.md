### Description
![HydraulicTankC picture](tank_user.svg)

Hydraulic tank component of C-type.

#### Input Variables
* **p** - Default Pressure [Pa]

### Theory
This component is equal to the C-type pressure source component. It supplies the hydraulic port with the pressure obtained from signal port. Default pressure parameter value is used if signal port is not connected. Flow will be as large as is required. 
<!---EQUATION c_1 = p--->
<!---EQUATION Z_{c1} = 0--->

