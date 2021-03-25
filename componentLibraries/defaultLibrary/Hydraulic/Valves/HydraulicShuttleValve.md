### Description
![Shuttle valve picture](shuttlevalve_user.svg)

A hydraulic lossless shuttle valve model.


#### Port Initial Conditions
No initial conditions can be set for Q-type blocks.

<!--- ### Tips--->

### Theory
This component is implemented as a lossless two-way connection. The pressure in the two connected ports will always be the same. The third port will be closed (i.e. produce zero flow).

<!---EQUATION LABEL=eq:pressures \begin{cases}p_1 = p_3, q_1=-q_3, q_2=0 & p_1>p_2\\ p_2 = p_3, q_2 = -q_3, q_1 = 0 & p_1 \le p_2\end{cases} --->
Cavitation is handled by forcing pressures to be greater than or equal to zero.
#### Hopsan TLM adaption
In Q components the positive flow direction is outwards from each port, in this case the TLM equations are
<!---EQUATION p_1 = c_1 + q_1 Z_{c1} --->
<!---EQUATION p_2 = c_2 + q_2 Z_{c2} --->
<!---EQUATION p_3 = c_3 + q_3 Z_{c3} --->
The flow equation can now be derived in the following steps:
<!---EQUATION \begin{cases}c_1 + q_1 Z_{c1} = c_3 + q_3 Z_{c3}, & q_2 = 0 & p_1 > p_2 \\ c_2 + q_2 Z_{c2} = c_3 + q_3 Z_{c3}, & q_1 = 0 & p_1 \le p_2 \end{cases}--->
<!---EQUATION \begin{cases}c_1 - q_3 Z_{c1} = c_3 + q_3 Z_{c3}, & q_1=-q_3, & q_2 = 0 & p_1 > p_2 \\ c_2 - q_3 Z_{c2} = c_3 + q_3 Z_{c3}, & q_2 = -q_3, & q_1 = 0 & p_1 \le p_2 \end{cases}--->
<!---EQUATION \begin{cases}q_3\left(Z_{c1}+Z_{c3}\right) = c_1 - c_3, & q_1 = -q_3, & q_2 = 0 & p_1 > p_2 \\ q_3\left(Z_{c2}+Z_{c3}\right) = c_2 - c_3, & q_2 = -q_3, & q_1 = 0 & p_1 \le p_2 \end{cases}--->
Which finally becomes the implemented equation EQREF{eq:final}

<!---EQUATION \begin{cases}q_3 = \dfrac{c_1 - c_3}{Z_{c1}+Z_{c3}}, & q_1=-q_3, & q_2 = 0 & p_1 > p_2 \\ q_3 = \dfrac{c_2 - c_3}{Z_{c2}+Z_{c3}} , & q_2=-q_3, & q_1 = 0 & p_1 \le p_2 \end{cases}--->

