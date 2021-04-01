### Description
![MechanicRackAndPinion picture](MechanicRackAndPinion.svg)

A mechanic rack and pinion component of Q-type

#### Input Variables
* **k** - Spring Coefficient [Nm/rad]
* **omega** - Gear ratio [m/rad]
* **J** - Moment of Inertia [kgm^2]
* **B** - Viscous Friction [Nms/rad]

### Theory
Angular velocity is modelled using a second order transfer function:
<!---EQUATION \omega_2 = \dfrac{(F_1 \omega_{gear} + T_2) s}{J s^2 + B s + k} --->
Angle is integrated from velocity:
<!---EQUATION \theta_2 = \dfrac{\omega_2}{s} --->
Velocity and position is computed with the gear ratio:
<!---EQUATION v_1 = -\omega_2 \omega_{gear} --->
<!---EQUATION x_1 = -\theta_2 \omega_{gear} --->

#### Hopsan TLM adaption
Force and torque are computed using the TLM boundary equations:
<!---EQUATION F_1 = c_1 + Z_c v_1 --->
<!---EQUATION T_2 = c_2 + Z_c \omega_2 --->
Inserting this into the angular velocity equation gives the implemented equation:
<!---EQUATION \omega_2 = \dfrac{(c_1 \omega_{gear} + c_2) s}{J s^2 + (B+Z_{c1} \omega_{gear}^2 + Z_{c2}) s + k} --->

