### Description
![MechanicMotor picture](MechanicMotor.svg)

A simple motor component implemented as a torque source with angular velocity feedback control 

#### Input Variables
* **omega_ref** - Desired Angular Velocity [-]
* **K_p** - Proportional Controller Gain [-]
* **K_i** - Integrating Controller Gain [-]
* **K_d** - Derivating Controller Gain [-]


### Theory
<!---EQUATION \omega_{error} = \omega_{ref} - \omega_1 --->
<!---EQUATION T_{motor} = \omega_{error} K_p + \dfrac{\omega_{error}}{s} K_i + \omega_{error} s K_d --->

#### Hopsan TLM adaption
The motor is assumed to have zero impedance. Hence, the impedance variable set to zero and the wave variable will equal the torque:
<!--- c_1 = T_{motor} --->
<!--- Z_{c1} = 0 --->
