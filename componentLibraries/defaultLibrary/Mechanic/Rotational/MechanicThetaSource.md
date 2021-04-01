### Description
![MechanicThetaSource picture](MechanicAngularVelocityTransformer.svg)

A mechanical angular velocity and angle source component.

#### Input Variables
* **thetain** - Angle [rad]
* **omega** - Angular Velocity [rad/s]

### Theory
Applies specified angle and angualr velocity. Parameter values will be used if their corresponding input port is not connected. If angualr velocity is connected but not angle, the angle will be integrated from the velocity. Connecting both inputs simultaneously or connecting only position input is possible, but the kinematic relationship between angle and angular velocity must then be enforced manually.
<!---EQUATION \omega = \omega_{in} --->
<!---EQUATION \theta = \begin{cases}\theta_{in}, & \mathrm{if\ }\theta_{in}\mathrm{\ is\ connected}\\\displaystyle\int \omega dt, & \mathrm{otherwise}\end{cases}--->

#### Hopsan TLM adaption
Torque is computed using the TLM boundary equation:
<!---EQUATION T_1 = c_1 + Z_{c1} \omega_1--->
