### Description
![MechanicPulley picture](MechanicPulley.svg)

A mechanical pulley component with inertia and viscous friction.

#### Input Variables
* **m** - Inertia [kg]
* **B** - Viscous Friction [Nms/rad]

### Theory
Position and speed are modelled using Newton's second law of motion together with a gear ratio of 2:
<!---EQUATION x_2 = \dfrac{2 F_1-F_2}{m s^2 + B s} --->
<!---EQUATION v_2 = \dfrac{2 F_1-F_2}{m s + B} --->
<!---EQUATION v_1 = -2 v_2 --->
<!---EQUATION x_1 = -2 x_2 --->

#### Hopsan TLM adaption
Forces are computed from the TLM boundary equations:
<!---EQUATION F_1 = c_1 + Z_{c1} v_1 --->
<!---EQUATION F_2 = c_2 + Z_{c2} v_2 --->

This gives the final equation system:
<!---EQUATION \begin{cases}x_2 = \dfrac{2 c_1 - 4 Z_{c1} v_2 - c_2 - Z_{c2} v_2}{m s^2 + \left(B+4Z_{c1}+Z_{c2}\right) s}\\v_2 = \dfrac{2 F_1-F_2}{m s + B+4Z_{c1}+Z_{c2})}\\v_1 = -2 v_2\\x_1 = -2 x_2\\F_1 = c_1 + Z_{c1} v_1\\F_2 = c_2 + Z_{c2} v_2 \end{cases}--->


