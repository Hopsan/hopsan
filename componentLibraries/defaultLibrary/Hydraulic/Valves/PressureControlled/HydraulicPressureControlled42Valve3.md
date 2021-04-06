### Description
![HydraulicPressureControlled42Valve3 picture](HydraulicPressureControlled42Valve3.svg)

Contains a pressure controlled hydraulic 4/2 valve of Q-type with no closed position

#### Input Variables
* **omega_h** - Resonance frequency [Frequency]
* **delta_h** - Damping factor [-]
* **Fs_min** - Minimum pressure for opening the valve [Pa]
* **Fs_max** - Pressure for fully opening the valve [Pa]
* **C_q** - Flow coefficient [-]
* **rho** - Oil density [kg/m^3]
* **d** - Spool diameter [m]
* **f_pa** - Fraction of spool diameter that is opening P-A [-]
* **f_pb** - Fraction of spool diameter that is opening P-B [-]
* **f_bt** - Fraction of spool diameter that is opening B-T [-]
* **f_at** - Fraction of spool diameter that is opening A-T [-]
* **x_vmax** - Maximum spool position [-]

#### Output Variables
* **x_v** - Spool position [m]

### Theory
Dynamics is modelled as a low-pass filter with bandwidth and damping. A turbulent flow model is used. The opening area depends on the spool position, which is computed from the control pressure using second order dynamics.

<!---EQUATION q_{i\rightarrow j} = C_q A_{i\rightarrow j}\sqrt{\dfrac{2}{\rho}\left(p_i-p_j\right)},\quad i,j \in \{a,b,p,t\} --->
<!---EQUATION A_{p\rightarrow a}=A_{b\rightarrow t}=f\dfrac{d^2\pi}{4} (x_{v,max} - x_v) ---> 
<!---EQUATION A_{p\rightarrow b}=A_{a\rightarrow t}=f\dfrac{d^2\pi}{4} x_v ---> 
<!---EQUATION x_{ref} = \begin{cases}\dfrac{p_{c}-F_{s,min}}{F_{s,max}-F_{s,min}}x_{v,max}, & p_c \ge F_{s,min}\\0,&p_c < F_{s,min}\end{cases} --->
<!---EQUATION x_v = \dfrac{x_{ref}}{\left(\dfrac{s^2}{\omega_h^2}+\dfrac{2\delta_h}{\omega_h}s+1\right)} --->
Cavitation is handled by forcing pressures to be greater than or equal to zero.
#### Hopsan TLM adaption
In Q components the positive flow direction is outwards from each port, in this case the TLM equations are
<!---EQUATION p_{i} = c_{i} + q_{i} Z_{ci} ,\quad i \in \{a,b,p,t\} --->
<!---EQUATION q_{a} = q_{p\rightarrow a} - q_{a\rightarrow t} --->
<!---EQUATION q_{p} = -q_{p\rightarrow a} - q_{p\rightarrow b}--->
<!---EQUATION q_{t} = q_{a\rightarrow t} + q_{b\rightarrow t} --->
<!---EQUATION q_{b} = -q_{b\rightarrow t} + q_{p\rightarrow b}--->
Inserting this into equation EQREF{eq:flow} and taking negative pressure drops into account gives the implemented equation EQREF{eq:final}:

<!---EQUATION LABEL=eq:final q_{i\rightarrow j} = \begin{cases} K_s \left(\sqrt{c_i-c_j+\dfrac{(Z_{ci}+Z_{cj})^2K_s^2}{4}} - K_s\dfrac{Z_{ci}+Z_{cj}}{2}\right), c_i > c_j\\ K_s\left(K_s\dfrac{(Z_{ci}+Z_{cj})}{2} - \sqrt{c_j-c_i+\dfrac{(Z_{ci}+Z_{cj})^2 K_s^2}{4}}\right), c_i \le c_j \end{cases},\quad i,j\in \{a,b,p,t\} --->

where

<!---EQUATION LABEL=eq:Ks K_s = C_q A_{i \rightarrow j} \sqrt{\dfrac{2}{\rho}} --->
