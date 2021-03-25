### Description
![Q-type piston picture](cylinderq_user.svg)

A hydraulic cylinder of Q-type with second order dynamics.

#### Input Variables
* **Kc** - Pressure-Flow Coefficient [m^5/Ns]
* **A_1** - Piston Area 1 [m^2]
* **A_2** - Piston Area 2 [m^2]
* **B_p** - Viscous Friction Coefficient of Piston [Ns/m]
* **B_l** - Viscous Friction of Load [Ns/m]
* **k_l** - Stiffness of Load [N/m]
* **m_l** - Inertia Load [kg]
* **s_l** - Stroke [m]

#### Port Initial Conditions
No initial conditions can be set for Q-type blocks.

<!--- ### Tips--->

### Theory
This is a Q-type hydraulic linear actuator with second order dynamics. The capacitances in the chambers are neglected. Motion of the piston is determined by Newton's second law of motion according to EQREF{eq:flow}
<!---EQUATION LABEL=eq:flow \begin{cases}m_l\dot{v}_3 + \left(B_l+B_p\right)v_3 + k_l x_3 = p_1 A_1 - p_2 A_2 - F_3 \\v_3 = \dot{x}_3\end{cases}--->
Cavitation is handled by forcing pressures to be greater than or equal to zero.
#### Hopsan TLM adaption
In Q components the positive flow direction is outwards from each port, in this case the TLM equations are
<!---EQUATION p_{1} = c_{1} + q_{1} Z_{c1} --->
<!---EQUATION p_{2} = c_{2} + q_{2} Z_{c2} --->
<!---EQUATION F_{3} = c_{3} + v_{3} Z_{c3} --->
We also need to consider the continuity equations for the cylinder chambers
<!---EQUATION v_3 = -\dfrac{q_1}{A_1} --->
<!---EQUATION v_3 = -\dfrac{q_2}{A_2} --->

The first equation can now be rewritten in the following steps
<!---EQUATION m_l\dot{v}_3 + \left(B_l+B_p\right)v_3 + k_l x_3 = \left(c_{1} + q_1 Z_{c1}\right) A_1 - \left(c_2 + q_2 Z_{c2}\right) A_2 - c_{3} - v_{3} Z_{c3} --->
<!---EQUATION m_l\dot{v}_3 + \left(B_l+B_p\right)v_3 + k_l x_3 = \left(c_{1} - v_3 A_1 Z_{c1}\right) A_1 - \left(c_2 + v_3 A_2 Z_{c2}\right) A_2 - c_{3} - v_{3} Z_{c3} --->
<!---EQUATION m_l\dot{v}_3 + \left(B_l+B_p\right)v_3 + k_l x_3 = c_1 A_1 - v_3 A_1^2 Z_{c1} - c_2 A_2 - v_3 A_2^2 Z_{c2} - c_{3} - v_{3} Z_{c3} --->
<!---EQUATION m_l\dot{v}_3 + \left(B_l+B_p+A_1^2 Z_{c1}+A_2^2 Z_{c2}+Z_{c3}\right)v_3 + k_l x_3 = c_1 A_1 - c_2 A_2  - c_{3} --->

Position and velocity are finally integrated separately, which gives the implemented equation EQREF{eq:final}

<!---EQUATION LABEL=eq:flow \begin{cases}m_l\ddot{x}_3 + \left(B_l+B_p+A_1^2Z_{c1} + A_2^2Z_{c2}+Z_{c3}\right)\dot{x}_3 + k_l x_3 = c_1 A_1 - c_2 A_2 - c_3 \\m_l\dot{v}_3 + \left(B_l+B_p+A_1^2Z_{c1} + A_2^2Z_{c2}+Z_{c3}\right)v_3 = c_1 A_1 - c_2 A_2 - c_3\end{cases}--->

