### Description
![HydraulicMachineC picture](machinec_user.svg)

A hydraulic positive displacement machine of C-type.

#### Input Variables
* **J_em** - Equivalent load of inertia [kg*m^2]
* **eps** - Displacement setting [-]
* **Beta_e** - Bulk modulus of oil [Pa]
* **V_1** - Volume at port 1 [m^3]
* **V_2** - Volume at port 2 [m^3]
* **D_m** - Displacement [m^3/rev]
* **C_lm** - Leakage coefficient [LeakageCoefficient]
* **B_m** - Viscous friction coefficient [Nms/rad]

### Theory
The effective displacement is the ideal dispalcement multiplied by the displacement setting. For simplicity it is also converted to SI-units (m^3/rad).
<!---EQUATION D_{me} = \dfrac{\varepsilon_m D_m}{2\pi} --->

Pressure in each chamber volume is computed according to the continuity equation:
<!---EQUATION \sum q_{in} = \dfrac{V}{\beta_e}\dfrac{dp}{dt}--->

Each volume has three input flows, one from the external connection, one from the internal leakage and one due to the machine displacement. This yields:
<!---EQUATION \begin{cases}q_1 + q_{leak} - q_m = \dfrac{V_1}{\beta_e}\dfrac{dp_1}{dt}\\q_2 - q_{leak} + q_m = \dfrac{V_2}{\beta_e}\dfrac{dp_2}{dt}\end{cases} --->

The displacement flow is the product of the effective displacement and the angular velocity:
<!---EQUATION q_m = D_{me} \omega_3 --->

The leakage flow is computed using a laminar flow model:
<!---EQUATION q_{leak} = (p_1 - p_2)c_{leak}--->

#### Hopsan TLM adaption
The volume is modelled as a joint between multiple transmission line elements, where each element has a delay of half the simulation time step. This makes the total delay between any two connections equal to one whole time step. The wave variable for each connection then becomes:
<!---EQUATION \begin{cases}c'_1 = \dfrac{1}{3}\left(c_{m,1} - 2Z_{c1}D_{me} \omega_3 + c_{leak,1} + 2Z_{c1}q_{leak} + c_1 + 2Z_{c1}q_1\right)\\c'_2 = \dfrac{1}{3}\left(c_{m,2} + 2Z_{c2}D_{me} \omega_3 + c_{leak,2} - 2Z_{c2}q_{leak} + c_2 + 2Z_{c2}q_2\right)\end{cases} --->

Each wave variable is then low-pass filtered to avoid high-frequency oscillations:
<!---EQUATION \begin{cases}c_1(t) = \alpha c_1(t-\Delta t) + (1-\alpha)c'_1(t)\\c_2(t) = \alpha c_2(t-\Delta t) + (1-\alpha)c'_2(t)\end{cases} --->

The characteristic impedances are computed based on current volume size, bulk modulus and time step:
<!---EQUATION \begin{cases}Z_{c,1} = \dfrac{3}{2}\dfrac{\beta_e}{V_1}\dfrac{\Delta t}{1-\alpha}\\Z_{c,2} = \dfrac{3}{2}\dfrac{\beta_e}{V_2}\dfrac{\Delta t}{1-\alpha}\end{cases} --->

The mechanical wave variable is computed by transforming the internal wave variables by multiplying with the effective displacement:
<!---EQUATION c_3 = D_{me}(c_{m,1} - c_{m,2}} --->

Similarly, the mechanical impedance is transformed from the hydraulic impedance variables. The damping parameter is also added. The actual damping force is then computed in the connected Q-type mechanical component.
<!---EQUATION Z_3 = D_{me}^2(Z_{c1} + Z_{c2}) + B_m --->



