### Description
![ElectricCapacitanceMultiPort picture](ElectricCapacitanceMultiPort.svg)

Contains an electric capacitance component of C-type

#### Input Variables
* **Capacitance** - Capacitance [Fa]
* **alpha** - Low pass coeficient to dampen standing delayline waves [-]

### Theory
Capacitance is defined as the relationship between the sum of all currents and the rate of change of the voltage:
<!---EQUATION \sum{i_{in}} = C \dfrac{dU}{dt} --->

#### Hopsan TLM adaption
The capacitance is modelled as a joint between multiple transmission line elements, where each element has a delay of half the simulation time step. This makes the total delay between any two connections equal to one whole time step. The wave variable for each connection then becomes:
<!---EQUATION c'_i(t) = \dfrac{2}{N}\displaystyle\sum_{j=1}^N\left(c_j(t-\Delta t)+2 Z_c q_j(t-\Delta t)\right) - c_i(t-\Delta t) - 2Z_c i_i(t-\Delta t),\quad i=1,...,N --->
where <i>N</i> is the number of connections.
Each wave variable is then low-pass filtered to avoid high-frequency oscillations:
<!---EQUATION c_i(t) = \alpha c_i(t-\Delta t) + (1-\alpha)c'_i(t) --->
The volume is distributed across all the transmission line elements. This yields the following expression for the characteristic impedance:
<!---EQUATION Z_c = \dfrac{N \Delta t}{2C(1-\alpha)} --->

