### Description
![HydraulicVolume picture](volume_user.svg)

A hydraulic multi-port volume component of c-type

#### Input Variables
* **V** - Volume [m^3]
* **Beta_e** - Bulkmodulus [Pa]
* **P_high** - High pressure (for animation) [Pa]
* **alpha** - Low pass coefficient to dampen standing delayline waves [-]

### Theory
The pressure in the volume is based on the continuity equation for a volume of fixed size:
<!---EQUATION \sum{q_{in}} = \dfrac{V}{\beta_e}\dfrac{dp}{dt} --->

#### Hopsan TLM adaption
The volume is modelled as a joint between multiple transmission line elements, where each element has a delay of half the simulation time step. This makes the total delay between any two connections equal to one whole time step. The wave variable for each connection then becomes:
<!---EQUATION c'_i(t) = \dfrac{2}{N}\displaystyle\sum_{j=1}^N\left(c_j(t-\Delta t)+2 Z_c q_j(t-\Delta t)\right) - c_i(t-\Delta t) - 2Z_c q_i(t-\Delta t),\quad i=1,...,N --->
where <i>N</i> is the number of connections.
Each wave variable is then low-pass filtered to avoid high-frequency oscillations:
<!---EQUATION c_i(t) = \alpha c_i(t-\Delta t) + (1-\alpha)c'_i(t) --->
The volume is distributed across all the transmission line elements. This yields the following expression for the characteristic impedance:
<!---EQUATION Z_c = \dfrac{N \beta_e}{2V}\dfrac{\Delta t}{1-\alpha} --->

