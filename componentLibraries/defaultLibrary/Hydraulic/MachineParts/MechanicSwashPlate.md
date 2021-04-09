### Description
![MechanicSwashPlate picture](svg/swashplate.svg)

A mechanic swash plate component

#### Input Variables
* **r** - Swivel Radius [m]
* **theta_offset** - Angle Offset [m]
* **angle** - Angle [rad]
* **movement** Angular velocity [rad/s]

#### Output Variables
* **torque** - Torque [Nm]

### Theory
Total torque from all pistons (N = number of connections):
<!---EQUATION T_1 = \displaystyle\sum_{i=1}^N \left(p_{1,i}\tan(angle)r\cos(-\theta_2-\theta_{offset}-2\pi\dfrac{i}{N}\right)--->

#### Hopsan TLM adaption
Torques and forces are calculated according to the TLM boundary equations:
<!---EQUATION F_{1,i} = c_{1,i} + Z_{c1,i}v_{1,i},\quad i=1,...,N --->

