### Description
![SignalSaturation picture](SignalSaturation.svg)

Contains a signal saturation component

#### Input Variables
* **in** - Input variable [-]
* **y_upper** - Upper Limit [-]
* **y_lower** - Lower Limit [-]

#### Output Variables
* **out** - Output variable [-]

### Theory
Limits (saturates) the variable to be within specified upper and lower limits.

<!---EQUATION out = \begin{cases}y_{upper}, & in > y_{upper}\\in, & y_{lower} \le in \le y_{upper}\\y_{lower}, & y < y_{lower}\end{cases}--->

