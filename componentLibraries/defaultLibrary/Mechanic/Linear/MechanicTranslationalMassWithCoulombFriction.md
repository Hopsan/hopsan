### Description
![MechanicTranslationalMassWithCoulombFriction picture](masschelp.svg)

A translational mass with coulomb friction and viscous friction

#### Input Variables
* **m** - Mass [kg]
* **b** - Viscous Friction Coefficient [Ns/m]
* **f_s** - Static Friction Force [N]
* **f_k** - Kinetic Friction Force [N]
* **x_min** - Lower Limit of Position of Port P2 [m]
* **x_max** - Upper Limit of Position of Port P2 [m]

### Theory
A translational inertia modelled according to Euler's first law of motion. Coulomb friction is modelled like an ideal force acting in the opposite direction of the movement. Limitation parameters apply to port P2.
<!---EQUATION m \dot{v_2} + B v_2 = F_1 - F_2 - F_f--->
<!---EQUATION v_2 = der(x_2)--->
<!---EQUATION v_1 = -v_2--->
<!---EQUATION x_1 = -x_2--->
<!---EQUATION F_f = \begin{cases}F_1-F_2, & |F_1-F_2| < f_s\\f_k, & F_1-F_2 > f_s\\-f_k, & F_2-F_1 > f_s\end{cases} --->

#### Hopsan TLM adaption
The forces are computed with the TLM boundary equations:
<!---EQUATION \begin{cases}F_1 = \displaystyle\sum_{i=1}^{N_1}\left(c_{1,i} + Z_{c1,i}v_{1,i}\right), i=1,...,N_1\\\displaystyle\sum_{j=1}^{N_2}\left(c_{2,j} + Z_{c2,j}v_{2,j}\right), j=1,...,N_2\end{cases}--->

These equation inserted into the equation of motion yields:
<!---EQUATION m \dot{v_2} + B v_2 = c_1 - Z_{c1}v_2 - c_2 - Z_{c2}v_2 - F_f--->

The characteristic impedances can be moved to the right hand side and treated as viscous friction:
<!---EQUATION m \dot{v_2} + (B+Z_{c1}+Z_{c2})v_2 = c_1 - c_2 - F_f--->

The position <i>x<sub>2</sub></i> and velocity <i>v<sub>2</sub></i> are then integrated using the first and second order transfer functions:
<!---EQUATION x_2 = \dfrac{c_1 - c_2 - F_f}{m s^2 + (B+Z_{c1}+Z_{c2})s}--->

<!---EQUATION v_2 = \dfrac{c_1 - c_2 - F_f}{m s + B+Z_{c1}+Z_{c2}}--->
