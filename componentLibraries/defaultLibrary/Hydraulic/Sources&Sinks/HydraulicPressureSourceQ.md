### Description
![HydraulicPressureSourceQ picture](pressuresource_user.svg)

Hydraulic pressure source component of Q-type

#### Input Variables
* **p** - Set pressure [Pa]

### Theory
Supplies the hydraulic port with the pressure obtained from signal port. Default pressure parameter value is used if signal port is not connected. Flow will be as large as is required. Warning! Connecting a Q-type pressure source directly to a fixed size volume component is physically incorrect, because the flow would be undefined.
<!---EQUATION q_1 = (p - c_1)/Z_{c1} --->
<!---EQUATION p_1 = p --->

