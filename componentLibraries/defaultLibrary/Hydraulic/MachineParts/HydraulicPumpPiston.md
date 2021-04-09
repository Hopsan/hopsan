### Description
![HydraulicPumpPiston picture](svg/piston.svg)

Contains a hydraulic piston with one chamber of C type

#### Input Variables
* **A_1** - Piston area [m^2]
* **s_l** - Stroke [m]
* **V_1** - Dead volume in chamber 1 [m^3]
* **B_p** - Viscous friction [Ns/m]
* **Beta_e** - Bulk modulus [Pa]
* **c_leak** - Leakage coefficient [LeakageCoefficient]
* **F_0** - Spring force [N]

#### Port Initial Conditions
Volumes are initialized based on the position start value. 
Initial wave variables are computed based on intial pressures and flows.

### Theory
This is a C-type hydraulic spring-loaded piston with one chamber, dead volume and viscous friction. End of stroke limitation is not included, and must be handled by the adjacent Q-type component.

The component is modelled as a single voluems of variable size. 
Pressure in theh chamber volume is computed according to the continuity equation:
<!---EQUATION \sum q_{in} = \dfrac{V}{\beta_e}\dfrac{dp}{dt}--->

The volume has three input flows, one from the external connection, one from the internal leakage and one due to the moving piston. This yields:
<!---EQUATION q_1 - q_{leak} + A_1 v_3 = \dfrac{V_1}{\beta_e}\dfrac{dp_1}{dt} --->

Volume size is computed based on current piston position, provided by the connected Q-type mechanical component:
<!---EQUATION \begin{cases}V_1 = V_{dead,1} - x_3 A_1 --->

The leakage flow is computed using a laminar flow model:
<!---EQUATION q_{leak} = p_1 c_{leak}--->

#### Hopsan TLM adaption
The volume is modelled as a joint between multiple transmission line elements, where each element has a delay of half the simulation time step. This makes the total delay between any two connections equal to one whole time step. The wave variable for each connection then becomes:
<!---EQUATION c'_1 = \dfrac{1}{3}\left(c_{piston,1} + 2Z_{c1}v_3 A_1 + c_{leak,1} - 2Z_{c1}q_{leak} + c_1 + 2Z_{c1}q_1\right) --->

The wave variable is then low-pass filtered to avoid high-frequency oscillations:
<!---EQUATION c_1(t) = \alpha c_1(t-\Delta t) + (1-\alpha)c'_1(t) --->

The characteristic impedances are computed based on current volume size, bulk modulus, time step, filter coefficient and number of connections:
<!---EQUATION Z_{c,1} = \dfrac{3}{2}\dfrac{\beta_e}{V_1}\dfrac{\Delta t}{1-\alpha} --->

The mechanical wave variable is computed by transforming the internal wave variables by multiplying with each piston area. The spring force is also added to the wave variable, so that the actual spring force will be computed in the adjacent Q-type component.

<!---EQUATION c_3 = A_1 c_{piston,1} + F_s --->

Similarly, the mechanical impedance is transformed from the hydraulic impedance variables. The damping parameter is also added. The actual damping force is then computed in the connected Q-type mechanical component.
<!---EQUATION Z_3 = A_1^2*Z_{c1} + b_p --->

