### Description
![Lossless connector picture](losslessconnector_user.svg)

A hydraulic lossless connector model.


#### Port Initial Conditions
No initial conditions can be set for Q-type blocks.

<!--- ### Tips--->

### Theory
This component is implemented as a laminar orifice with unrestricted flow. Using this between two volumes will cause the volumes behave as one large volume. A time delay will however be introduced, possibly reducing the numerical stability. Note that this component will not define the flow in the system. A system with only volumes and lossless connectors will result in undefined flows.

The pressure in both ports will always be the same. The flow will be computed as a consequence of this relationship.
<!---EQUATION LABEL=eq:pressures p_1 = p_2 --->
Cavitation is handled by forcing pressures to be greater than or equal to zero.
#### Hopsan TLM adaption
In Q components the positive flow direction is outwards from each port, in this case the TLM equations are
<!---EQUATION p_{1} = c_{1} + q_{1} Z_{c1} --->
<!---EQUATION p_{2} = c_{2} + q_{2} Z_{c2} --->
<!---EQUATION q_{1} = -q_{2} --->
The flow equation can now be derived in the following steps:
<!---EQUATION c_1 + q_1 Z_{c1} = c_2 + q_2 Z_{c2} --->
<!---EQUATION c_1 - q_2 Z_{c1} = c_2 + q_2 Z_{c2} --->
<!---EQUATION q_2\left(Z_{c1}+Z_{c2}\right) = c_1 - c_2 --->
Which finally becomes the implemented equation EQREF{eq:final}

<!---EQUATION LABEL=eq:final q_2 = \dfrac{c_1 - c_2}{Z_{c1}+Z_{c2}} --->

