### Description
![HydraulicVolume picture](volume_user.svg)

A hydraulic volume component of c-type

#### Input Variables
* **V** - Volume [m^3]
* **Beta_e** - Bulkmodulus [Pa]
* **P_high** - High pressure (for animation) [Pa]
* **alpha** - Low pass coefficient to dampen standing delayline waves [-]

### Theory
The pressure in the volume is based on the continuity equation for a volume of fixed size:
<!---EQUATION \sum{q_{in}} = \dfrac{V}{\beta_e}\dfrac{dp}{dt} --->

#### Hopsan TLM adaption
Delayed information is propagated from left to right side using wave variables:
<!---EQUATION \begin{cases}c'_1(t) = p_2(t-\Delta t) + Z_c q_2(t-\Delta t)\\c'_2(t) = p_1(t-\Delta t) + Z_c q_1(t-\Delta t)\end{cases} --->
The characteristic impedance of the volume is calculated using the bulk modulus, the volume size and the low pass filter coefficient:
<!---EQUATION Z_c = \dfrac{\beta_e}{V}\dfrac{\Delta_t}{1-\alpha} --->
The pressure is then integrated using the TLM boundary equations:
<!---EQUATION \begin{cases}p_1(t) = c_1(t) + Z_c q_1(t)\\p_2(t) = c_2(t) + Z_c q_2(t)\end{cases} --->
Finally, the wave variables are low-pass filtered using the filter coefficient:
<!---EQUATION \begin{cases}c_1(t) = \alpha c_1(t-\Delta t) - (1-\alpha) c'_1(t)\\c_2(t) = \alpha c_2(t-\Delta t) - (1-\alpha) c'_2(t)\end{cases} --->

