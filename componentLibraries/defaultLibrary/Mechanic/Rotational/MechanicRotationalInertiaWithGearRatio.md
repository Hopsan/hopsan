### Description
![MechanicRotationalInertiaWithGearRatio picture](inertiawgearhelp.svg)

A mechanical rotational gear ratio with inertia component

#### Input Variables
* **J** - Moment of Inertia [kgm^2]
* **omega** - Gear ratio [-]
* **B** - Viscous Friction [Nms/rad]

### Theory
Angular velocity and angle are modelled using a second order transfer function:
<!---EQUATION \omega_2 = \dfrac{-T_1 \omega_{gear} - T_2}{J s + B} --->
<!---EQUATION \theta_2 = \dfrac{-T_1 \omega_{gear} - T_2}{J s^2 + B s} --->
Velocity and position is computed with the gear ratio:
<!---EQUATION \omega_1 = \omega_2 \omega_{gear} --->
<!---EQUATION \theta_1 = \theta_2 \omega_{gear} --->

#### Hopsan TLM adaption
Force and torque are computed using the TLM boundary equations:
<!---EQUATION T_1 = c_1 + Z_c \omega_1 --->
<!---EQUATION T_2 = c_2 + Z_c \omega_2 --->
Inserting this into the angle and angular velocity equation gives the implemented equations:
<!---EQUATION \omega_2 = \dfrac{-c_1 \omega_{gear} - c_2}{J s + B+Z_{c1} \omega_{gear}^2 + Z_{c2}} --->
<!---EQUATION \theta_2 = \dfrac{-c_1 \omega_{gear} - c_2}{J s^2 + (B+Z_{c1} \omega_{gear}^2 + Z_{c2}) s} --->

