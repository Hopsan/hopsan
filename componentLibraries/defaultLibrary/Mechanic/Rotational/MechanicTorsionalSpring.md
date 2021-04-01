### Description
![MechanicTorsionalSpring picture](torsionalspringhelp.svg)

A torsional spring component of C-type

#### Input Variables
* **k** - Spring constant [Nm/rad]

### Theory
The spring torque is modelled using Hooke's law:
<!---EQUATION T_2 = k(\theta_1 - \theta_2) --->

#### Hopsan TLM adaption
Delayed information is propagated from left to right side using wave variables:
<!---EQUATION \begin{cases}c_1(t) = T_2(t-\Delta t) + Z_c \omega_2(t-\Delta t)\\c_2(t) = T_1(t-\Delta t) + Z_c \omega_1(t-\Delta t)\end{cases} --->
The characteristic impedance of the volume is calculated as the product of the spring constant and the time step:
<!---EQUATION Z_c = k \Delta_t --->
The pressure is then integrated using the TLM boundary equations:
<!---EQUATION \begin{cases}T_1(t) = c_1(t) + Z_c \omega_1(t)\\T_2(t) = c_2(t) + Z_c \omega_2(t)\end{cases} --->
Which gives the implemented equations:
<!---EQUATION \begin{cases}c_1(t) = c_2(t-\Delta t) + 2 Z_c \omega_2(t-\Delta t)\\c_2(t) = c_1(t-\Delta t) + 2 Z_c \omega_1(t-\Delta t)\end{cases} --->


