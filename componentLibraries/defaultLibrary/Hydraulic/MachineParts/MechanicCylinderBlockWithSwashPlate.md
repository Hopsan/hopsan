### Description
![MechanicCylinderBlockWithSwashPlate picture](svg/swashplate.svg)

Contains a mechanic cylinder block with swash plate component

#### Input Variables
* **J** - Moment of Inertia of Cylinder Block [kgm^2]
* **m_p** - Mass of each Piston [kg]
* **r_p** - Piston Radius [m]
* **B** - Viscous Friction [Nms/rad]
* **r** - Swivel Radius [m]
* **theta_offset** - Angle Offset [m]
* **angle** - Angle [rad]

#### Output Variables
* **torque** - Torque [Nm]
* **movement** - ? [?]

### Theory
Total torque from all pistons (N = number of connections):
<!---EQUATION T_1 = \displaystyle\sum_{i=1}^N \left(p_{1,i}\tan(angle)r\cos(-\theta_2-\theta_{offset}-2\pi\dfrac{i}{N}\right)--->

Angle and angular velocity are computed from the torques:
<!---EQUATION LABEL=eq:angle \theta_2 = \dfrac{T_1 - T_2}{Js^2 + Bs} --->
<!---EQUATION LABEL=eq:speed \omega_2 = \dfrac{T_1 - T_2}{Js + B} --->

Position and velocity of each connected piston is computed with trigonometry:
<!---EQUATION v_{1,i} = -r\tan(angle)\cos\left(-\theta_2-\theta_{offset}-2 \pi\dfrac{i}{N}\right)\omega_2,\quad i = 1,...,N --->
<!---EQUATION x_{1,i} = x_{1,i} = x_{0,i}+r\tan(angle)\sin\left(-\theta_2-\theta_{offset}-2 \pi\dfrac{i}{N}\right),\quad i=1,...,N --->

#### Hopsan TLM adaption
Torques and forces are calculated according to the TLM boundary equations:
<!---EQUATION F_{1,i} = c_{1,i} + Z_{c1,i}v_{1,i},\quad i=1,...,N --->
<!---EQUATION T_2 = c_2 + Z_{c2}\omega_2 --->

Equations EQREF{eq:angle} and EQREF{eq:speed} then become:
<!---EQUATION \theta_2 = \dfrac{c_1 - c_2}{Js^2 + (B+Z_{c1}+Z_{c2})s} --->
<!---EQUATION \omega_2 = \dfrac{c_1 - c_2}{Js + (B+Z_{c1}+Z_{c2})} --->

Where the impedance from the multiport is:
<!---EQUATION Z_{c1} = \displaystyle\sum_{i=1}^N\left(Z_{c1,i}\tan^2(angle)r^2\cos^2\left(-\theta_2-\theta_{offset}-2 \pi\dfrac{i}{N}\right)\right) --->
