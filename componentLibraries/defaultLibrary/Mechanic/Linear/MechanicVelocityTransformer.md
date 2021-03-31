### Description
![MechanicVelocityTransformer picture](MechanicVelocityTransformer.svg)

A mechanical velocity and position source component.

#### Input Variables
* **m_e** - Equivalent Mass [kg]
* **v** - Velocity input [m/s]
* **x** - Position input [m]

### Theory
Applies specified position and velocity. Parameter values will be used if their corresponding input port is not connected. If velocity is connected but not position, the position will be integrated from the velocity. Connecting both inputs simultaneously or connecting only position input is possible, but the kinematic relationship between position and velocity must then be enforced manually.
<!---EQUATION v_1 = v_{in} --->
<!---EQUATION x_1 = \begin{cases}x_{in}, & \mathrm{if\ }x_{in}\mathrm{\ is\ connected}\\\int v_1 dt, & \mathrm{otherwise}\end{cases}--->

#### Hopsan TLM adaption
Force is computed using the TLM boundary equation:
<!---EQUATION F_1 = c_1 + Z_{c1} v_1--->
