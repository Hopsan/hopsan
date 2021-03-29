### Description
![HydraulicTLMlossless picture](tlm_user.svg)

Hydraulic Lossless Transmission Line Component

#### Input Variables
* **deltat** - Time delay [s]
* **alpha** - Low pass coefficient [-]
* **Z_c** - Impedance [Pa s/m^3]

### Theory
This is a fictive component that delays the pressure and flow from one end to the other. Even though the icon looks like a pipe, it is not modeling a pipe. You need to add a restrictor to each end if you want to approximately model a pipe.
<!---EQUATION \begin{cases}c'_1(t) = p_2(t-\Delta t) + Z_c q_2(t-\Delta t)\\c'_2(t) = p_1(t-\Delta t) + Z_c q_1(t-\Delta t)\end{cases}--->
<!---EQUATION \begin{cases}c_1(t) = \alpha c_1(t-\Delta t_{step}) + (1-\alpha)c'_1(t)\\c_2(t) = \alpha c_2(t-\Delta t_{step}) + (1-\alpha)c'_2(t)\end{cases}--->

