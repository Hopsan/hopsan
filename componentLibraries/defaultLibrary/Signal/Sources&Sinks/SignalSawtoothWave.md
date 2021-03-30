### Description
![SignalSawtoothWave picture](SignalSawtoothWave.svg)

Contains a sawtooth wave (train) signal generator

#### Input Variables
* **y_0** - Base Value [-]
* **y_A** - Amplitude [-]
* **t_start** - Start Time [Time]
* **dT** - Time Period [Time]
* **D** - Duty Cycle, (ratio 0<=x<=1) [-]

#### Output Variables
* **out** - PulseWave [-]

#### Port Initial Conditions
No initial conditions can be set for Q-type blocks.

<!--- ### Tips--->

### Theory
The dutycycle D, is the ratio of the PeroidTime dT when the signal ramping between the basevalue and the amplitude. Note that a negative amplitude will result in a decreasing ramp. 
<!---EQUATION --->

#### Hopsan TLM adaption
