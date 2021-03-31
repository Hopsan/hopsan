### Description
![MechanicMultiPortTranslationalMass picture](multimasshelp.svg)

A mechanical multiport translational mass with mass, viscous friction and end position limitations.

#### Input Variables
* **m** - Mass [kg]
* **B** - Viscous Friction [Ns/m]
* **x_min** - Minimum Position of Port P2 [m]
* **x_max** - Maximum Position of Port P2 [m]

### Theory
A translational inertia modelled according to Euler's first law of motion. Maximum and minimum positions are relative to the first connection on port P2. This version allows an arbitrary number of connections. Start positions can be different, but start velocities must be the same for all connections. 
<!---EQUATION \begin{cases}m \dot{v_2} + B v_2 = F_1 - F_2\\v_2 = der(x_2)\\v_1 = -v_2\\x_1 = -x_2\end{cases}--->

#### Hopsan TLM adaption
The forces are computed with the TLM boundary equations:
<!---EQUATION \begin{cases}F_1 = c_1 + Z_{c1}v_1 = c_1 - Z_{c1}v_2\\F_2 = c_2 + Z_{c2}v_2\end{cases}--->

These equation inserted into the equation of motion yields:
<!---EQUATION m \dot{v_2} + B v_2 = c_1 - Z_{c1}v_2 - c_2 - Z_{c2}v_2--->

The characteristic impedances can be moved to the right hand side and treated as viscous friction:
<!---EQUATION m \dot{v_2} + (B+Z_{c1}+Z_{c2})v_2 = c_1 - c_2--->

The position <i>x<sub>2</sub></i> and velocity <i>v<sub>2</sub></i> are then integrated using the first and second order transfer functions:
<!---EQUATION x_2 = \dfrac{c_1 - c_2}{m s^2 + (B+Z_{c1}+Z_{c2})s}--->

<!---EQUATION v_2 = \dfrac{c_1 - c_2}{m s + B+Z_{c1}+Z_{c2}}--->
