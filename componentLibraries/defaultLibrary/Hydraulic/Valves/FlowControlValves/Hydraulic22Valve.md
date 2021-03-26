### Description
![2/2 valve picture](22valvehelp.svg)

A hydraulic 2/2 flow control valve with second order dynamics. 

#### Input Variables
* **C_q** - Flow Coefficient [-]
* **rho** - Oil density [kg/m^3]
* **d** - Spool Diameter [m]
* **f** - Spool Fraction of the Diameter [-]
* **x_vmax** - Maximum Spool Displacement [m]
* **omega_h** - Resonance Frequency [Hz]
* **delta_h** - Damping Factor [-]

#### Port Initial Conditions
No initial conditions can be set for Q-type blocks.

<!--- ### Tips--->

### Theory
Dynamics is modelled as a low-pass filter with bandwidth and damping. A turbulent flow model is used. The opening area depends on the spool position, which is computed from the input signal using second order dynamics.

<!---EQUATION q_2 = C_q A\sqrt{\dfrac{2}{\rho}\left(p_1-p_2\right)} --->
<!---EQUATION A=f\dfrac{d^2\pi}{4}x_v ---> 
<!---EQUATION x_v = \dfrac{x_{in}}{\dfrac{s^2}{\omega_h^2}+\dfrac{2\delta_h}{\omega_h}s+1} --->
Cavitation is handled by forcing pressures to be greater than or equal to zero.
#### Hopsan TLM adaption
In Q components the positive flow direction is outwards from each port, in this case the TLM equations are
<!---EQUATION p_{1} = c_{1} + q_{1} Z_{c1} --->
<!---EQUATION p_{2} = c_{2} + q_{2} Z_{c2} --->
<!---EQUATION q_{1} = -q_{2} --->
Inserting this into equation EQREF{eq:flow} and taking negative pressure drops into account gives the implemented equation EQREF{eq:final}:

<!---EQUATION LABEL=eq:final q_2 = \begin{cases} K_s \left(\sqrt{c_1-c_2+\dfrac{(Z_{c1}+Z_{c2})^2K_s^2}{4}} - K_s\dfrac{Z_{c1}+Z_{c2}}{2}\right), c_1 > c_2\\ K_s\left(K_s\dfrac{(Z_{c1}+Z_{c2})}{2} - \sqrt{c_2-c_1+\dfrac{(Z_{c1}+Z_{c2})^2 K_s^2}{4}}\right), c_1 \le c_2 \end{cases} --->

where

<!---EQUATION LABEL=eq:Ks K_s = C_q A \sqrt{\dfrac{2}{\rho}} --->
