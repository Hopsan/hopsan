### Description
![MechanicRotationalInertiaMultiPort picture](inertiahelp.svg)

A mechanical rotational inertia component with multi-ports.

#### Input Variables
* **J** - Inertia [kgm^2]
* **B** - Viscous Friction [Nms/rad]
* **a_min** - Minimum Angle of Port P2 [rad]
* **a_max** - Maximum Angle of Port P2 [rad]

### Theory
Angular velocity and angle are modelled using a second order transfer function. Limitation parameters apply to port P2.
<!---EQUATION \theta_2 = \dfrac{T_1 - T_2}{J s^2 + B s} --->
<!---EQUATION \omega_2 = \dfrac{T_1 - T_2}{J s + B} --->
Velocity and position is computed with the gear ratio:
<!---EQUATION \omega_1 = -\omega_2 --->
<!---EQUATION \theta_1 = -\theta_2 --->


#### Hopsan TLM adaption
Force and torque are computed using the TLM boundary equations:
<!---EQUATION T_1 = c_1 + Z_c \omega_1 --->
<!---EQUATION T_2 = c_2 + Z_c \omega_2 --->
Inserting this into the angle and angular velocity equation gives the implemented equations:
<!---EQUATION \theta_2 = \dfrac{c_1 - c_2}{J s^2 + (B+Z_{c1} + Z_{c2}) s} --->
<!---EQUATION \omega_2 = \dfrac{c_1 - c_2}{J s + B+Z_{c1} + Z_{c2}} --->
Wave variables and characteristic impedances are summed up from all connections to each port.
