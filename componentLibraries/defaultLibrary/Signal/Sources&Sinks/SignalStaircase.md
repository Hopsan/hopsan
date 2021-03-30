### Description
![SignalStaircase picture](SignalStaircase.svg)

Contains a signal staircase function source

#### Input Variables
* **T_start** - Start Time [Time]
* **H_step** - Step Height [-]
* **W_step** - Step Width [Time]

#### Output Variables
* **out** - Stair case output [-]

### Theory
Generates a staircase signal with specified step width and height, starting at specified start time.
<!---EQUATION out = H_{step}\lfloor\dfrac{\max(0, t-T_{start}+0.5\min(t_{step},W_{step}))}{W_{step}}\rfloor--->

