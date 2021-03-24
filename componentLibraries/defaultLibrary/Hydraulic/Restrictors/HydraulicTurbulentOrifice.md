### Description
![Turbulent orifice picture](turbulentorifice_user.svg)

Hydraulic orifice with turbulent flow model.

#### Input Variables
* **A** - Orifice area [m^3]
* **C_q** - Flow coefficient [-]
* **rho** - Oil density [kg/m^3]

#### Port Initial Conditions
No initial conditions can be set for Q-type blocks.

<!--- ### Tips--->

### Theory
The turbulent flow through the orifice is proportional to the square root of the pressure difference according to equation EQREF{eq:flow}
<!---EQUATION LABEL=eq:flow q_{2} = C_{q} A \sqrt{\frac{2}{\rho}\left(p_{1}-p_{2}\right)} --->
Cavitation is handled by forcing pressures to be greater than or equal to zero.
#### Hopsan TLM adaption
In Q components the positive flow direction is outwards from each port, in this case the TLM equations are
<!---EQUATION p_{1} = c_{1} + q_{1} Z_{c1} --->
<!---EQUATION p_{2} = c_{2} + q_{2} Z_{c2} --->
<!---EQUATION q_{1} = -q_{2} --->
Inserting this into equation EQREF{eq:flow} and taking negative pressure drops into account gives the implemented equation EQREF{eq:final}:

<!---EQUATION LABEL=eq:final q_2 = \begin{cases} K_s \left(\sqrt{c_1-c_2+\dfrac{(Z_{c1}+Z_{c2})^2K_s^2}{4}} - K_s\dfrac{Z_{c1}+Z_{c2}}{2}\right), c_1 > c_2\\ K_s\left(K_s\dfrac{(Zc1+Zc2)}{2} - \sqrt{c_2-c_1+\dfrac{(Z_{c1}+Z_{c2})^2 K_s^2}{4}}\right), c_1 \le c_2 \end{cases} --->

where

<!---EQUATION LABEL=eq:Ks K_s = C_q A \sqrt{\frac{2}{\rho}} --->

