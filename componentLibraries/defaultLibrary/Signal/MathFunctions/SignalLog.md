### Description
![SignalLog picture](SignalLog.svg)

Contains a signal natural logarithm function component with error control

#### Input Variables
* **in** - Power [-]

#### Output Variables
* **out** - Exponent [-]
* **error** - Error [-]

### Theory
<!---EQUATION out = \begin{cases}0, & in \le 0 \\ \ln(in), &in > 0\end{cases}--->
<!---EQUATION error = \begin{cases}1, & in \le 0 \\ 0, &in > 0\end{cases}--->

