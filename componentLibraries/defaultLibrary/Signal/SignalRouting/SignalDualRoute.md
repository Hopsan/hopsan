### Description
![SignalDualRoute picture](SignalDualRoute.svg)

Signal routing component with two inputs

#### Input Variables
* **limit** - Limit value [-]
* **in1** -  [-]
* **in2** -  [-]
* **route** - Input selection [-]

#### Output Variables
* **out** - Selected input [-]

### Theory
Forwards input 1 to the output if route is smaller than the limit, otherwise input 2.
<!---EQUATION out = \begin{cases}in_1, & route < limit\\in_2, & route \ge limit\end{cases}--->

