### Description
![MechanicBallScrew picture](MechanicBallScrew.svg)

Contains a mechanic rack and pinion component

#### Input Variables
* **k** - Spring Coefficient [Nm/rad]
* **L** - Screw Lead [m]
* **ny** - Screw Efficiency [-]
* **ny2** - Reverse Efficiency [-]
* **J** - Moment of Inertia [kgm^2]
* **B** - Viscous Friction [Nms/rad]

#### Output Variables

#### Port Initial Conditions
No initial conditions can be set for Q-type blocks.

<!--- ### Tips--->

### Theory
The efficiency parameter will affect the torque-force relationship. It is assumed that no slip occurs. Hence, the relationship between angle and position will not be affected by the efficiencies.

Ideal gear ratio is computed from the lead:
<!---EQUATION G_{ideal} = \dfrac{L}{2\pi} --->
Torque-force gear ratio is computed by multiplying or dividing with the corresponding efficiency depending on the mode of operation:
<!---EQUATION G = \begin{cases}\dfrac{G_{ideal}}{\mu}, & T_2 > F_1 G_{ideal}\\G_{ideal}\mu_2, & T_2 \le F_1 G_{ideal}\end{cases} --->
Angular velocity is modelled using a second order transfer function:
<!---EQUATION \omega_2 = \dfrac{(F_1 G + T_2) s}{J s^2 + B s + k} --->
Angle is integrated from velocity:
<!---EQUATION \theta_2 = \dfrac{\omega_2}{s} --->
Velocity and position is computed with the ideal gear ratio:
<!---EQUATION v_1 = -\omega_2 G_{ideal} --->
<!---EQUATION x_1 = -\theta_2 G_{ideal} --->

#### Hopsan TLM adaption
Force and torque are computed using the TLM boundary equations:
<!---EQUATION F_1 = c_1 + Z_c v_1 --->
<!---EQUATION T_2 = c_2 + Z_c \omega_2 --->
Inserting this into the angular velocity equation gives the implemented equation:
<!---EQUATION \omega_2 = \dfrac{(c_1 G + c_2) s}{J s^2 + (B+Z_{c1} G^2 + Z_{c2}) s + k} --->
