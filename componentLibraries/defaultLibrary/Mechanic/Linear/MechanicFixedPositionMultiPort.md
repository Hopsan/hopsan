### Description
![MechanicFixedPositionMultiPort picture](fixpos_user.svg)

A mechanical fixed position component with multi-port

#### Input Variables
* **m_e** - Equivalent Mass [kg]

### Theory
Velocity is always zero. The force will then equal the wave variable. Equivalent mass is available in case the adjacent component needs it.
<!---EQUATION v_i = 0, i = 1,...,N --->
<!---EQUATION m_{e,i} = m_e, i=1,...,N --->
<!---EQUATION f_i = c_i, i=1,...,N --->
<i>N</i> is the number of connections in the multi-port.
