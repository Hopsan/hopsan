### Description
![SignalFFBand picture](SignalFFBand.svg)

FFB AND component with multiple inputs and a single output

#### Input Variables
* **in0** - Input 0 [-]
* **in1** - Input 1 [-]

#### Output Variables
* **state** - State activated [-]
* **exiting** - exiting to alt 0 [-]

### Theory
When a rising flank has been received on both inputs (<i>in0</i> and <i>in1</i>) a single pulse is sent to the <i>exiting</i> port. The <i>state</i> port becomes active when the first input receives a positive flank, and is deactivated by a flank on the second input.

