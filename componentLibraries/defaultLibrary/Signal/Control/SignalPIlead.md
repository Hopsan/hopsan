### Description
![SignalPIlead picture](SignalPIlead.svg)

PI-controler with lead filter in feadback path

#### Input Variables
* **yref** - Reference value [-]
* **kx** - Break frequency [rad/s]
* **y** - Actual value [-]
* **wa** - Break frequency [rad/s]
* **da** - Relative damping [-]
* **umin** - Minimum output signal [-]
* **umax** - Maximum output signal [-]

#### Output Variables
* **u** - Control signal [-]
* **err** - Adjusted error signal [-]
* **Ierr** - Limited adjusted error signal [-]
* **uI** - Control signal from integral part [-]

### Theory
Local expressions:
<!---EQUATION \small\begin{cases}k_1=\dfrac{\omega_a}{k_x}\\ \omega_{11} = \omega_a\\ \omega_{22}=\dfrac{\omega_a}{2 d_a}\end{cases} --->
Equations:
<!---EQUATION err=y_{ref}-\dfrac{\dfrac{s}{w_{21}}+1}{\dfrac{s}{w_{22}}+1}y --->
<!---EQUATION u=k_1 err + u_I --->
<!---EQUATION I_{err} = err --->
<!---EQUATION u_I = k_1\dfrac{\omega_{11}}{s}I_{err} --->