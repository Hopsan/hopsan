### Description
![ElectricVoltageSourceMultiPortC picture](ElectricUsource.svg)

An electric voltage source of C-type with multi-port.

#### Input Variables
* **U** - Voltage [V]

### Theory
Supplies the electric port with the voltage obtained from signal port. Default voltage parameter value is used if signal port is not connected. Current will be as large as is required. 
<!---EQUATION c_i = U,\quad i=1,...,N--->
<!---EQUATION Z_{c,i} = 0, \quad i=1,...,N--->
Where <i>N</i> is the number of connections.
