### Description
![HydraulicFixedDisplacementMotorQ picture](fixedmotorq_user.svg)

A hydraulic motor component with inertia load

#### Input Variables
* **D_m** - Displacement [m^3/rev]
* **B_m** - Viscous friction [Nm/rad]
* **C_lm** - Leakage coefficient [(m^3/s)/Pa]
* **J_m** - Inertia load [kg m^2]

### Theory
Displacement is used with radians instead of revolutions for simplicity:
<!---EQUATION D_{me} = \dfrac{D_{m}}{2\pi} --->

Ideal flow is computed from the displacement and angular velocity:
<!---EQUATION q_2 = D_{me} \omega_3 --->
<!---EQUATION q_1 = -q_2--->

Leakage flow is modelled as a laminar restriction:
<!---EQUATION q_{leak} = (p_2-p_1)C_{lm} --->

Angular velocity of the shaft is modelled using inertia and damping:
<!---EQUATION J_m\dot{\omega_3} + B_m\omega_3 = p_1 D_{me} - p_2 D_{me} - T_3--->
<!---EQUATION \omega_3 = \dot{\phi_3}--->

#### Hopsan TLM adaption
The pressures are obtained using the TLM boundary equations:
<!---EQUATION p_1 = c_1 + Z_{c1}q_1--->
<!---EQUATION p_2 = c_2 + Z_{c2}q_2--->

Wave variables are adjusted to take leakage into account:
<!---EQUATION c'_1 = (C_{lm} Z_{c2} + 1)\gamma c_1 + C_{lm} \gamma Z_{c1} c_2--->
<!---EQUATION c'_2 = (C_{lm} Z_{c1} + 1)\gamma c_2 + C_{lm} \gamma Z_{c2} c_1--->

Where
<!---EQUATION \gamma =  \dfrac{1}{C_{lm}(Z_{c1}+Z_{c2}+1}--->

Inserted into the shaft equation yields:
<!---EQUATION J_m\dot{\omega_3} + (B_m+Z_{c1}D_{me}^2+Z_{c2}D_{me}^2)\omega_3 = c'_1 D_{me} - c'_2 D_{me} - T_3--->

