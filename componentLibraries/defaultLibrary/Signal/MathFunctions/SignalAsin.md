### Description
![SignalAsin picture](SignalAsin.svg)

Contains a signal arcsin function component with error control

#### Input Variables
* **in** - Input variable [-]

#### Output Variables
* **out** - Angle [-]
* **error** - Error flag [-]

### Theory
<!---EQUATION out = \begin{cases}\arcsin(1), &input > 1\\ \arcsin(input), &-1 \le input \le 1\\ \arcsin(-1), &input < -1\end{cases}--->
<!---EQUATION error = \begin{cases}1, &input > 1\\ 0, &-1 \le input \le 1\\ 1, &input < -1\end{cases}--->

