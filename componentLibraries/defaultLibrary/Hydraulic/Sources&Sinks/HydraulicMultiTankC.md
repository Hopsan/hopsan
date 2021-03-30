### Description
![HydraulicMultiTankC picture](tank_user.svg)

Hydraulic multi-port tank C-type

#### Input Variables
* **p** - Default pressure [Pa]

### Theory
Supplies the hydraulic port with the pressure obtained from signal port. Default pressure parameter value is used if signal port is not connected. Flow will be as large as is required. This component is slightly slower than the one without multiport.
<!---EQUATION c_i = p,\quad i=1,...,N--->
<!---EQUATION Z_{c,i} = 0, \quad i=1,...,N--->
Where <i>N</i> is the number of connections.
