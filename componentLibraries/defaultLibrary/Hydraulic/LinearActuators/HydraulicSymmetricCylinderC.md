### Description
![HydraulicCylinderC picture](HydraulicSymmetricCylinderCHelp.svg)

A symmetric hydraulic cylinder of C type

#### Input Variables
* **use_sl** - Use end stops (stroke limitation) [-]
* **A_1** - Piston area 1 [m^2]
* **A_2** - Piston area 2 [m^2]
* **s_l** - Stroke [m]
* **V_1** - Dead volume in chamber 1 [m^3]
* **V_2** - Dead volume in chamber 2 [m^3]
* **B_p** - Viscous friction [Ns/m]
* **Beta_e** - Bulk modulus [Pa]
* **c_leak** - Leakage coefficient [LeakageCoefficient]

#### Output Variables
* **q_leak** - Internal Leakage Flow [Flow]

#### Port Initial Conditions
Volumes are initialized based on the position start value. 
Initial wave variables are computed based on intial pressures and flows.

### Theory
This is a C-type hydraulic linear actuator with dead volumes, viscous friction and internal leakage. End of stroke must be handled by adjacent component, and should be from zero to stroke length.

The component is modelled as two voluems of variable size. 
Pressure in each chamber volume is computed according to the continuity equation:
<!---EQUATION \sum q_{in} = \dfrac{V}{\beta_e}\dfrac{dp}{dt}--->

Each volume has three input flows, one from the external connection, one from the internal leakage and one due to the moving piston. This yields:
<!---EQUATION \begin{cases}q_1 - q_{leak} + A_1 v_3 = \dfrac{V_1}{\beta_e}\dfrac{dp_1}{dt}\\q_2 + q_{leak} - A_2 v_3 = \dfrac{V_2}{\beta_e}\dfrac{dp_1}{dt}\end{cases} --->

Volume sizes are computed based on current piston position, provided by the connected Q-type mechanical component:
<!---EQUATION \begin{cases}V_1 = V_{dead,1} - x_3 A_1 \\V_2 = V_{dead,2}+(s_l+x_3)A_2\end{cases}--->

The leakage flow is computed using a laminar flow model:
<!---EQUATION q_{leak} = (p_1 - p_2)c_{leak}--->

#### Hopsan TLM adaption
The volume is modelled as a joint between multiple transmission line elements, where each element has a delay of half the simulation time step. This makes the total delay between any two connections equal to one whole time step. The wave variable for each connection then becomes:
<!---EQUATION \begin{cases}c'_1 = \dfrac{1}{2+N_1}\left(c_{piston,1} + 2Z_{c1}v_3 A_1 + c_{leak,1} - 2Z_{c1}q_{leak} + \displaystyle\sum_{j=1}^{N_1}\left(c_{1,j} + 2Z_{c1}q_{1,j}\right)\right)\\c'_2 = \dfrac{1}{2+N_2}\left(c_{piston,2} - 2Z_{c2}v_3 A_2 + c_{leak,2} + 2Z_{c2}q_{leak} + \displaystyle\sum_{j=1}^{N_2}\left(c_{2,j} + 2Z_{c2}q_{2,j}\right)\right)\end{cases} --->

where <i>N<sub>1</sub></i> and <i>N<sub>2</sub></i> are the number of connections to port 1 and port 2, respectively.
Each wave variable is then low-pass filtered to avoid high-frequency oscillations:
<!---EQUATION \begin{cases}c_1(t) = \alpha c_1(t-\Delta t) + (1-\alpha)c'_1(t)\\c_2(t) = \alpha c_2(t-\Delta t) + (1-\alpha)c'_2(t)\end{cases} --->

The characteristic impedances are computed based on current volume size, bulk modulus, time step, filter coefficient and number of connections:
<!---EQUATION \begin{cases}Z_{c,1} = \dfrac{N_1+2}{2}\dfrac{\beta_e}{V_1}\dfrac{\Delta t}{1-\alpha}\\Z_{c,2} = \dfrac{N_2+2}{2}\dfrac{\beta_e}{V_2}\dfrac{\Delta t}{1-\alpha}\end{cases} --->

The mechanical wave variable is computed by transforming the internal wave variables by multiplying with each piston area:
<!---EQUATION c_3 = A_1 c_{piston,1} - A_2*c_{piston,2} --->

Similarly, the mechanical impedance is transformed from the hydraulic impedance variables. The damping parameter is also added. The actual damping force is then computed in the connected Q-type mechanical component.
<!---EQUATION Z_3 = A_1^2*Z_{c1} + A_2^2 Z_{c2} + b_p --->


### End of stroke limitation ###
End of stroke limitation is implemented as an extra spring force added to the piston whenever it is outside of its range. Hence, it is allowed to be outside the range for a short time. The force is added by increasing the mechanical wave variable and characteristic impedance. The extra impedance is computed as follows:

<!---EQUATION Z_{3,extra} = \dfrac{0.1 m_e}{\Delta t(1 - \alpha)} --->

The etra wave variable depends on which end of the range that has been execeeded:

<!---EQUATION \begin{cases}c_{3,extra} = \dfrac{0.1 m_e}{\Delta t^2(x_3 + s_l)} + Z_{3,extra} v_3\\c_{3,extra} = \dfrac{0.1 m_e}{\Delta t^2 x_3} + Z_{3,extra} v_3, &-x_3 > s_l, & -x_3<0\end{cases} --->


