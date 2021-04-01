### Description
![MechanicAngularVelocityTransformer picture](MechanicAngularVelocityTransformer.svg)

An angular velocity source component

#### Input Variables
* **omega** - Generated angular velocity [rad/s]

### Theory
Angular velocity in the mechanical port is controlled by the input variable.
<!---EQUATION \omega = \omega_{in}--->
Angle is integrated from the velocity:
<!---EQUATION \theta = \displaystyle\int \omega dt --->

#### Hopsan TLM adaption
Torque is computed using the TLM boundary equation:
<!---EQUATION T = c + Z_c \omega --->
