### Description
![SignalPID picture](SignalPID.svg)

PID controller

#### Input Variables
* **yref** - Reference value [-]
* **y** - Actual value [-]
* **dy** - Differential of actual value [-]
* **Kp** - Proportional gain [-]
* **KI** - Integral gain [-]
* **Kd** - Differential gain [-]
* **umin** - Minimum output signal [-]
* **umax** - Maximum output signal [-]

#### Output Variables
* **u** - Control signal [-]
* **Ierr** - Limited error [-]
* **uI** - Control signal from integral part [-]

### Theory
<!---EQUATION u=K_p (y_{ref}-y) + K_I\dfrac{y_{ref}-y}{s} - K_d \dot{y} --->

