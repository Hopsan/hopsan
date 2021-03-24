### Description
![Preloaded check valve picture](checkvalvepreload_user.svg)

Hydraulic pre-loaded check valve with turbulent flow model.

#### Input Variables
* **K_s** - Restrictor coefficient [-]
* **F_s** - Spring Pre-Load Tension [Pa]

#### Port Initial Conditions
No initial conditions can be set for Q-type blocks.

<!--- ### Tips--->

### Theory
The check valve will behave as a turbulent orifice when there is a positive pressure drop exceeding the spring pre-load tension, otherwise it will close to prevent flow. The turbulent flow through the orifice is proportional to the square root of the pressure difference according to equation EQREF{eq:flow}
<!---EQUATION LABEL=eq:flow q_{2} = \begin{cases}K_s \sqrt{p_{1}-p_{2}}, & p_1 > p_2+F_s \\0, & p_1 \le p_2+F_s\end{cases} --->

#### Hopsan TLM adaption
In Q components the positive flow direction is outwards from each port, in this case the TLM equations are
<!---EQUATION p_{1} = c_{1} + q_{1} Z_{c1} --->
<!---EQUATION p_{2} = c_{2} + q_{2} Z_{c2} --->
<!---EQUATION q_{1} = -q_{2} --->
Inserting this into equation EQREF{eq:flow} and taking negative pressure drops into account gives the implemented equation EQREF{eq:final}:

<!---EQUATION LABEL=eq:final q_2 = \begin{cases} K_s \left(\sqrt{c_1-c_2+\dfrac{(Z_{c1}+Z_{c2})^2K_s^2}{4}} - K_s\dfrac{Z_{c1}+Z_{c2}}{2}\right), & c_1 > c_2+F_s\\ 0, & c_1 \le c_2+F_s \end{cases} --->

