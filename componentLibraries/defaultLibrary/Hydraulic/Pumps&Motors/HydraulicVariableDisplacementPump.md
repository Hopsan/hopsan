### Description
![HydraulicVariableDisplacementPump picture](variablepump_user.svg)

//! @brief

#### Input Variables
* **eps** - Displacement setting [-]
* **omega_p** - Angular velocity [rad/s]
* **D_p** - Displacement [m^3/rev]
* **K_cp** - Leakage coefficient [(m^3/s)/Pa]

#### Output Variables
* **a** - Angle [-]

### Theory
Ideal flow is computed from the displacement setting, the angular velocity and the leakage flow:
<!---EQUATION LABEL=eq:flow q_2 = \dfrac{\varepsilon D_p}{2\pi} \omega_p - q_{leak}--->
<!---EQUATION q_{leak} = K_{cp}(p_2-p_1) --->
<!---EQUATION q_1 = -q_2 --->

#### Hopsan TLM adaption
The pressures are obtained using the TLM boundary equations:
<!---EQUATION p_1 = c_1 + Z_{c1}q_1--->
<!---EQUATION p_2 = c_2 + Z_{c2}q_2--->

The leakage flow then becomes:
<!---EQUATION LABEL=eq:leak q_{leak} = K_{cp}(c_2 + Z_{c2}q_2 - c_1 - Z_{c1}q_1) --->

Inserting equation EQREF{eq:leak} into equation EQREF{eq:flow} then yields the implemented equation:
<!---EQUATION q_2 = \dfrac{ \dfrac{\varepsilon D_p}{2\pi}\omega_p + K_{cp}(c_1-c_2) }{ (Z_{c1}+Z_{c2})C_{lp}+1 }--->

