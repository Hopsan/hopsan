### Description
![Laminar orifice picture](laminarorifice_user.svg)

Hydraulic orifice modeling laminar flow.

#### Input Variables
* **Kc** - Pressure-Flow Coefficient [m^5/Ns]

#### Port Initial Conditions
No initial conditions can be set for Q-type blocks.

<!--- ### Tips--->

### Theory
The laminar flow through the orifice is proportional to the pressure difference according to equation EQREF{eq:flow}
<!---EQUATION LABEL=eq:flow q_{2} = K_{c} \left(p_{1}-p_{2}\right) --->
Cavitation is handled by forcing pressures to be greater than or equal to zero.
#### Hopsan TLM adaption
In Q components the positive flow direction is outwards from each port, in this case the TLM equations are
<!---EQUATION p_{1} = c_{1} + q_{1} Z_{c1} --->
<!---EQUATION p_{2} = c_{2} + q_{2} Z_{c2} --->
<!---EQUATION q_{1} = -q_{2} --->
The flow equation can now be rewritten in the following steps
<!---EQUATION q_{2} = K_{c} \left( c_{1} - q_{2} Z_{c1} - c_{2} + q_{2} Z_{c2} \right) --->
<!---EQUATION q_{2} = K_{c} \left( c_{1} - c_{2} \right) - K_{c} q_{2} \left( Z_{c1} + Z_{c2} \right) --->
<!---EQUATION q_{2} \left( 1 + K_{c} q_{2} \left( Z_{c1} + Z_{c2} \right)  \right) = K_{c} \left(  c_{1} - c_{2} \right) --->

Which finally becomes the implemented equation EQREF{eq:final}

<!---EQUATION LABEL=eq:final q_2 = \frac{K_{c}\left(c_{1}-c_{2}\right)}{1.0+K_{c}\left(Zc_{1}+Zc_{2}\right)} --->


