### Description
![SignalTimeDelay picture](SignalTimeDelay.svg)

Contains a Signal Time Delay Component

#### Input Variables
* **deltat** - Time delay [s]
* **in** - Input variable [-]

#### Output Variables
* **out** - Output variable [-]

### Theory
Will delay the signal with specified time delay. The delay will be rounded to the closest even multiple of the simulation time step. Minimum delay is one time step.
<!---EQUATION out(t) = in(t-\Delta t)--->


