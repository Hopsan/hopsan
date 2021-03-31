### Description
![MechanicTranslationalSpring picture](springhelp.svg)

A mechanical spring component of C-type

#### Input Variables
* **alpha** - Low-pass filter cofficient [-]
* **k** - Spring constant [N/m]

### Theory
The spring force is modelled using Hooke's law:
<!---EQUATION F_2 = k(x_1 - x_2) --->

#### Hopsan TLM adaption
Delayed information is propagated from left to right side using wave variables:
<!---EQUATION \begin{cases}c'_1(t) = F_2(t-\Delta t) + Z_c v_2(t-\Delta t)\\c'_2(t) = F_1(t-\Delta t) + Z_c v_1(t-\Delta t)\end{cases} --->
The characteristic impedance of the volume is calculated using the spring constant, the time step and the low pass filter coefficient:
<!---EQUATION Z_c = \dfrac{k \Delta_t}{1-\alpha} --->
The pressure is then integrated using the TLM boundary equations:
<!---EQUATION \begin{cases}F_1(t) = c_1(t) + Z_c v_1(t)\\F_2(t) = c_2(t) + Z_c v_2(t)\end{cases} --->
Finally, the wave variables are low-pass filtered using the filter coefficient:
<!---EQUATION \begin{cases}c_1(t) = \alpha c_1(t-\Delta t) - (1-\alpha) c'_1(t)\\c_2(t) = \alpha c_2(t-\Delta t) - (1-\alpha) c'_2(t)\end{cases} --->

