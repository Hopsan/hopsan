### Description
![SignalLog10 picture](SignalLog10.svg)

Contains a signal base-10 logarithm function component with error control

#### Input Variables
* **in** - Power [-]

#### Output Variables
* **out** - Exponent [-]
* **error** - Error [-]

### Theory
<!---EQUATION out = \begin{cases}0, & in \le 0 \\ \log10(in), &in > 0\end{cases}--->
<!---EQUATION error = \begin{cases}1, & in \le 0 \\ 0, &in > 0\end{cases}--->

