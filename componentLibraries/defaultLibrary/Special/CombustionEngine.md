### Description
![CombustionEngine picture](CombustionEngine.svg)

Simple combustion engine component of C-type

#### Input Variables
* **P_max** - Max Power [W]
* **Characteristics** - Torque-Speed Characteristics [-]
* **in** - Input signal [-]

### Theory

Maximum torque is a function of the velocity, and provided by a lookup table:
<!---EQUATION T_{max} = f_{table}(\omega) --->

Desired (reference) torque is the maximum power over the velocity multiplied by the input signal:
<!---EQUATION T_{ref} = -in \dfrac{P_{max}}{\omega}--->

Output torque is finally limited to be between zero and maximum torque:
<!---EQUATION T = \begin{cases}0,&T_{ref} < 0\\T_{ref}, & 0 \le T_{ref} \le T_{max}\\T_{max},& T_{ref} > T_{max}\end{cases} --->

#### Hopsan TLM adaption
The component is modelled as an ideal torque source of C-type. This means that the wave variable will equal the torque, and the characteristic impedance will be zero:
<!---EQUATION c = T --->
<!---EQUATION Z_c = 0 --->
