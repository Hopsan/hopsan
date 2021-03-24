### Description
![Pilot-controlled check valve picture](checkvalvepilot_user.svg)

Hydraulic pilot controlled check valve with turbulent flow model.

#### Input Variables
* **K_s** - Restrictor coefficient [-]
* **phi** - Pilot ratio [-]
* **p_f** - Cracking pressure [Pa]

#### Port Initial Conditions
No initial conditions can be set for Q-type blocks.

<!--- ### Tips--->

### Theory
The check valve will behave as a turbulent orifice when the sum of the positive pressure drop and the pilot pressure exceeds the cracking pressure, otherwise it will close to prevent flow. Pilot pressure works on a larger area than the regular pressure and is therefore multiplied by a pilot ratio. The turbulent flow through the orifice is proportional to the square root of the pressure difference according to equation EQREF{eq:flow}.
<!---EQUATION LABEL=eq:flow q_{2} = \begin{cases}K_s \sqrt{p_{1}-p_{2}}, & p_1 + p_{pilot}\varphi - p_2 > p_f \\0, & p_1 + p_{pilot}\varphi - p_2 \le p_f\end{cases} --->
Cavitation is handled by forcing pressures to be greater than or equal to zero.
#### Hopsan TLM adaption
In Q components the positive flow direction is outwards from each port, in this case the TLM equations are
<!---EQUATION p_{1} = c_{1} + q_{1} Z_{c1} --->
<!---EQUATION p_{2} = c_{2} + q_{2} Z_{c2} --->
<!---EQUATION q_{1} = -q_{2} --->
Inserting this into equation EQREF{eq:flow} and taking negative pressure drops into account gives the implemented equation EQREF{eq:final}:

<!---EQUATION LABEL=eq:final q_2 = \begin{cases} K_s \left(\sqrt{c_1-c_2+\dfrac{(Z_{c1}+Z_{c2})^2K_s^2}{4}} - K_s\dfrac{Z_{c1}+Z_{c2}}{2}\right), & c_1 + c_{pilot}\varphi - c_2 > p_f\\ 0, & c_1 + c_{pilot}\varphi - c_2 \le p_f \end{cases} --->

