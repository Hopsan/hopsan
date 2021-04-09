### Description
![Hydraulic22DirectionalCheckValve picture](Hydraulic22DirectionalCheckValve_user.svg)

A hydraulic on/off check valve of Q-type

#### Input Variables
* **omega_h** - Resonance Frequency [Frequency]
* **delta_h** - Damping Factor [-]
* **in** - <0.5 (check), >0.5 (open) [-]
* **Kv_open** - Pressure-Flow Coefficient in opened position [(m^3/s)/sqrt(Pa)]
* **Kv_check** - Pressure-Flow Coefficient in checked position [(m^3/s)/sqrt(Pa)]
* **F_s** - Spring Pre-Load Tension [Pa]

#### Output Variables
* **out** - <0.5 (check), >0.5 (open) [-]

### Theory
This code is a combination of Hydraulic22DirectionalValve.hpp and HydraulicCheckValvePreLoaded.hpp. It behaves like a regular turbulent orifice in one position and a pre-loaded checkvalve in the second.

<!---EQUATION q_2 = \begin{cases}K_{v,open}\sqrt{p_1-p_2}, & x_v > 0.5 \\ K_{v,closed}\sqrt{p_1-p_2}, & x_v \le 0.5\ \mathrm{and}\ p_1 - p_2 > F_s\\0, & \mathrm{otherwise}\end{cases}--->
<!---EQUATION q_1 = -q_2 --->
<!---EQUATION x_v = \dfrac{x_{ref}}{\dfrac{s^2}{\omega_h^2}+\dfrac{2\delta_h}{\omega_h}s+1} --->
<!---EQUATION x_{ref} = \begin{cases}0, & in \le 0.5\\1, & in > 0.5\end{cases} --->
Cavitation is handled by forcing pressures to be greater than or equal to zero.
#### Hopsan TLM adaption
In Q components the positive flow direction is outwards from each port, in this case the TLM equations are
<!---EQUATION p_{1} = c_{1} + q_{1} Z_{c1} --->
<!---EQUATION p_{2} = c_{2} + q_{2} Z_{c2} --->
<!---EQUATION q_{1} = -q_{2} --->
Inserting this into equation EQREF{eq:flow} and taking negative pressure drops into account gives the implemented turbulent flow equation EQREF{eq:final}:

<!---EQUATION LABEL=eq:final q_2 = \begin{cases} K_v \left(\sqrt{c_1-c_2+\dfrac{(Z_{c1}+Z_{c2})^2K_v^2}{4}} - K_v\dfrac{Z_{c1}+Z_{c2}}{2}\right), c_1 > c_2\\ K_v\left(K_v\dfrac{(Z_{c1}+Z_{c2})}{2} - \sqrt{c_2-c_1+\dfrac{(Z_{c1}+Z_{c2})^2 K_v^2}{4}}\right), c_1 \le c_2 \end{cases} --->

