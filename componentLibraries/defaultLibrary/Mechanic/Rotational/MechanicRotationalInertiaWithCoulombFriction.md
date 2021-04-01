### Description
![MechanicRotationalInertiaWithCoulombFriction picture](inertiachelp.svg)

Contains a rotational inertia with coulumb friction and damper

#### Input Variables
* **t_s** - Static Friction Torque [Nm]
* **t_k** - Kinetic Friction Torque [Nm]
* **J** - Inertia [kgm^2]
* **B** - Viscous Friction Coefficient [Nms/rad]
* **a_min** - Minimum Angle of Port P2 [rad]
* **a_max** - Maximum Angle of Port P2 [rad]

### Theory
A rotational inertia modelled according to Euler's second law of motion. Coulomb friction is modelled like an ideal torque acting in the opposite direction of the movement. Limitation parameters apply to port P2.
<!---EQUATION J \dot{\omega_2} + B\omega_2 = T_1 - T_2 - T_f--->
<!---EQUATION \omega_2 = der(\theta_2)--->
<!---EQUATION \omega_1 = -\omega_2--->
<!---EQUATION \theta_1 = -\theta_2--->
<!---EQUATION T_f = \begin{cases}T_1-T_2, & |T_1-T_2| < T_s\\T_k, & T_1-T_2 > T_s\\-T_k, & T_2-T_1 > T_s\end{cases} --->

#### Hopsan TLM adaption
The torques are computed with the TLM boundary equations:
<!---EQUATION \begin{cases}T_1 = c_1 + Z_{c1}\omega_1\\T_2 = c_2 + Z_{c2}\omega_2\end{cases}--->

These equation inserted into the equation of motion yields:
<!---EQUATION J \dot{\omega_2} + B \omega_2 = c_1 - Z_{c1}\omega_2 - c_2 - Z_{c2}\omega_2 - T_f--->

The characteristic impedances can be moved to the right hand side and treated as viscous friction:
<!---EQUATION J \dot{\omega_2} + (B+Z_{c1}+Z_{c2})\omega_2 = c_1 - c_2 - T_f--->

The angle and the angular velocity are then integrated using the first and second order transfer functions:
<!---EQUATION \theta_2 = \dfrac{c_1 - c_2 - T_f}{J s^2 + (B+Z_{c1}+Z_{c2})s}--->

<!---EQUATION \omega_2 = \dfrac{c_1 - c_2 - T_f}{J s + B+Z_{c1}+Z_{c2}}--->
