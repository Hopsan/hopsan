### Description
![SignalFFBorIn picture](SignalFFBor.svg)

FFB OR component with a single input and two outputs

#### Input Variables
* **in0** - Input 0 [-]
* **divert** - Input 0 [-]

#### Output Variables
* **state** - State activated [-]
* **out0** - exiting to alt 0 [-]
* **out1** - exiting to alt 0 [-]

### Theory
* A rising flank on <i>in0</i> while <i>divert</i> is inactive will cause a single pulse on ports <i>out0</i> and <i>state</i>.
* A rising flank on <i>in0</i> while <i>divert</i> is active will cause a single pulse on ports <i>out1</i> and <i>state</i>.

