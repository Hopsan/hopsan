### Description
![SignalSqrt picture](SignalSqrt.svg)

Contains a signal square root function component with error control

#### Input Variables
* **in** - Input value [-]

#### Output Variables
* **out** - Square root [-]
* **error** - Error [-]

### Theory
<!---EQUATION out = \begin{cases}\sqrt{in}, & in \ge 0 \\ 0, & in < 0\end{cases}--->
<!---EQUATION error = \begin{cases}0, & in \ge 0 \\ 1, & in < 0\end{cases}--->

