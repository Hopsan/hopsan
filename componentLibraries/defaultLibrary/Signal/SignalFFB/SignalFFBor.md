### Description
![SignalFFBor picture](SignalFFBor.svg)

FFB OR component with multiple inputs and a single output port

#### Input Variables
* **in0** - Input 0 [-]
* **in1** - Input 1 [-]

#### Output Variables
* **state** - State activated [-]
* **exiting** - exiting to alt 0 [-]

### Theory
A rising flank on either <i>in0</i> or <i>in1</i> will cause a single pulse on ports <i>state</i> and <i>exiting</i>.

