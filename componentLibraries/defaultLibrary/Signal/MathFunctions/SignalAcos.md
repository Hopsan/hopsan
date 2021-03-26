### Description
![SignalAcos picture](SignalAcos.svg)

Contains a signal acrcos function component with error control

#### Input Variables
* **in** - Input variable [-]

#### Output Variables
* **out** - Angle [-]
* **error** - Error flag [-]

### Theory
<!---EQUATION out = \begin{cases}\arccos(1), &input > 1\\ \arccos(input), &-1 \le input \le 1\\ \arccos(-1), &input < -1\end{cases}--->
<!---EQUATION error = \begin{cases}1, &input > 1\\ 0, &-1 \le input \le 1\\ 1, &input < -1\end{cases}--->

