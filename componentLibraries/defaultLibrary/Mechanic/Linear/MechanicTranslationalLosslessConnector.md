### Description
![MechanicTranslationalLosslessConnector picture](losslessconnector_user.svg)

A mechanic translational lossless connector

#### Port Initial Conditions
No initial conditions can be set for Q-type blocks.

<!--- ### Tips--->

### Theory
This component is implemented as a rigid body with zero inertia. It can for example be used to connect two springs directly with no inertia in-between. A time delay will however be introduced, possibly reducing the numerical stability. Note that this component will not define any velocity in the system. A system with only springs and lossless connectors will result in undefined movements.

The forces in both ports will always be the same. The flow will be computed as a consequence of this relationship.
<!---EQUATION LABEL=eq:pressures F_1 = F_2 --->

#### Hopsan TLM adaption
In Q components the positive velocity direction is outwards from each port, in this case the TLM equations are
<!---EQUATION F_{1} = c_{1} + v_{1} Z_{c1} --->
<!---EQUATION F_{2} = c_{2} + v_{2} Z_{c2} --->
<!---EQUATION v_{1} = -v_{2} --->
<!---EQUATION x_{1} = -x_{2} --->
The equation of motion can now be derived in the following steps:
<!---EQUATION c_1 + v_1 Z_{c1} = c_2 + v_2 Z_{c2} --->
<!---EQUATION c_1 - v_2 Z_{c1} = c_2 + v_2 Z_{c2} --->
<!---EQUATION v_2\left(Z_{c1}+Z_{c2}\right) = c_1 - c_2 --->
Which finally becomes the implemented equation EQREF{eq:final}

<!---EQUATION LABEL=eq:final v_2 = \dfrac{c_1 - c_2}{Z_{c1}+Z_{c2}} --->

