### Description
![SignalPID2 picture](SignalPID.svg)

PID controller using standard form parameters

#### Input Variables
* **e** - Control error [-]
* **de** - Derivative signal input [-]
* **K** - Gain [-]
* **Ti** - Integral time [s]
* **Tt** - Anti-windup tracking constant [s]
* **Td** - Derivative time [s]
* **Umin** - Lower output for anti-windup [-]
* **Umax** - Upper output for anti-windup [-]
* **Uoutmin** - Minimum output limit [-]
* **Uoutmax** - Maximum output limit [-]

#### Output Variables
* **u** - Control signal [-]

### Theory
Manual implementation of a PID controller using standard form parameters. If you are familiar with other parameters, convert according to: K=Kp, Ti=K/Ki, Td=Kd/K. Optional port "de" can be activated and if connected, the derivative signal from that port will be used instead of the actual derivative of the control error.

PID controller equation:
<!---EQUATION u(t) = K e(t) + I(t) + \dfrac{K T_d}{t_{step}} \dfrac{de(t)}{dt} --->

Integrator value is numerically integrated using the value from the previous step:
<!---EQUATION I(t) = I'(t-t_{step}) + K \dfrac{t_{step}}{T_i} e(t) --->

Integrator value is limited when anti-windup limits are exceeded:
<!---EQUATION I'(t) = \begin{cases}I(t), &u_{min}\le u(t) \le u_{max}\\ I(t) + \dfrac{t_{step}}{T_t}(u_{max}-u(t)), & u(t)>u_{max}\\ I(t) + \dfrac{t_{step}}{T_t}(u_{min}-u(t)), & u(t)<u_{min}\end{cases} --->

If the derivative signal input is not connected, the derivative is approximated numerically:
<!---EQUATION \dfrac{de(t)}{dt} \approx e(t)-e(t-t_{step}) --->
