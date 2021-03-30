### Description
![HydraulicPressureSourceC picture](pressuresource_user.svg)

Contains a Hydraulic Pressure Source Component of C-type

#### Input Variables
* **p** - Set pressure [Pa]

### Theory
Supplies the hydraulic port with the pressure obtained from signal port. Default pressure parameter value is used if signal port is not connected. Flow will be as large as is required. This component is slightly faster than the one with multiport.
<!---EQUATION c_1 = p--->
<!---EQUATION Z_{c1} = 0--->

