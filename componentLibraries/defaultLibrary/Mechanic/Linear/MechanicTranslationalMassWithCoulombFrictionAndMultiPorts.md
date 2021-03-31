### Description
![MechanicTranslationalMassWithCoulombFrictionAndMultiPorts picture](masschelp.svg)

Contains a translational mass with coulomb friction and damper using multi-ports (converted from version without multi-ports)

#### Input Variables
* **m** - Mass [kg]
* **b** - Viscous Friction Coefficient [Ns/m]
* **f_s** - Static Friction Force [N]
* **f_k** - Kinetic Friction Force [N]
* **x_min** - Lower Limit of Position of Port P2 [m]
* **x_max** - Upper Limit of Position of Port P2 [m]

#### Output Variables

#### Port Initial Conditions
No initial conditions can be set for Q-type blocks.

<!--- ### Tips--->

### Theory
A translational inertia modelled according to Euler's first law of motion. Coulomb friction is modelled like an ideal force acting in the opposite direction of the movement. Limitation parameters apply to port P2. This version use multi-ports, allowing several connections on each side. For better performance, use the version without multi-ports.
<!---EQUATION --->

#### Hopsan TLM adaption
