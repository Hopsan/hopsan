### Description
![HydraulicPressureControlled22Valve2 picture](HydraulicPressureControlled22Valve2.svg)

Contains a pressure controlled hydraulic 2/2 valve of Q-type with default position open

#### Input Variables
* **omega_h** - Resonance frequency [Frequency]
* **delta_h** - Damping factor [-]
* **Fs_min** - Minimum pressure for opening the valve [Pa]
* **Fs_max** - Pressure for fully opening the valve [Pa]
* **C_q** - Flow coefficient [-]
* **rho** - Oil density [kg/m^3]
* **d** - Spool diameter [m]
* **f_pa** - Fraction of spool diameter that is opening P-A  [-]
* **f_bt** - Fraction of spool diameter that is opening B-T [-]
* **x_vmax** - Maximum spool position [-]

#### Output Variables
* **x_v** - Spool position [m]

### Theory
Dynamics is modelled as a low-pass filter with bandwidth and damping. A turbulent flow model is used. The opening area depends on the spool position, which is computed from the control pressure using second order dynamics.

<!---EQUATION q_{p\rightarrow a} = C_q A\sqrt{\dfrac{2}{\rho}\left(p_p-p_a\right)} --->
<!---EQUATION A=f\dfrac{d^2\pi}{4} x_v ---> 
<!---EQUATION x_{ref} = \begin{cases}\dfrac{p_{c}-F_{s,min}}{F_{s,max}-F_{s,min}}x_{v,max}, & p_c \ge F_{s,min}\\0,&p_c < F_{s,min}\end{cases} --->
<!---EQUATION x_v = \dfrac{x_{ref}}{\left(\dfrac{s^2}{\omega_h^2}+\dfrac{2\delta_h}{\omega_h}s+1\right)} --->
Cavitation is handled by forcing pressures to be greater than or equal to zero.
#### Hopsan TLM adaption
In Q components the positive flow direction is outwards from each port, in this case the TLM equations are
<!---EQUATION p_{p} = c_{p} + q_{p} Z_{cp} --->
<!---EQUATION p_{a} = c_{a} + q_{a} Z_{ca} --->
<!---EQUATION q_{a} = q_{p\rightarrow a} --->
<!---EQUATION q_{p} = -q_{p\rightarrow a} --->
Inserting this into equation EQREF{eq:flow} and taking negative pressure drops into account gives the implemented equation EQREF{eq:final}:

<!---EQUATION LABEL=eq:final q_{p\rightarrow a} = \begin{cases} K_s \left(\sqrt{c_p-c_a+\dfrac{(Z_{cp}+Z_{ca})^2K_s^2}{4}} - K_s\dfrac{Z_{cp}+Z_{ca}}{2}\right), c_p > c_a\\ K_s\left(K_s\dfrac{(Z_{cp}+Z_{ca})}{2} - \sqrt{c_a-c_p+\dfrac{(Z_{cp}+Z_{ca})^2 K_s^2}{4}}\right), c_p \le c_a \end{cases} --->

where

<!---EQUATION LABEL=eq:Ks K_s = C_q A \sqrt{\dfrac{2}{\rho}} --->
