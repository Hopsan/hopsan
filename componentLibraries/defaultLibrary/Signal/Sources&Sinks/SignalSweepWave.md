### Description
![SignalSweepWave picture](SignalSweepWave.svg)

Contains a sine wave sweep signal generator. Based upon the sine wave signal component.

#### Input Variables
* **f_input** - Frequency Change per second [Hz/s]
* **f** - Start Frequencty [Hz]
* **y_0** - Base Value [-]
* **y_A** - Amplitude [-]
* **y_offset** - (Phase) Offset [Time]
* **t_start** - Start Time [Time]
            //addInputVariable(]

#### Output Variables
* **out** - Sine Sweep wave output [-]
* **f_out** - Sine Sweep Frequency output [-]

#### Port Initial Conditions
No initial conditions can be set for Q-type blocks.

<!--- ### Tips--->

### Theory
The component creates a sine wave with a variable frequency. The change of frequency is compounded linearly at each simulation time step. A negative rate of change will be parsed as positive using the abs() function.
<!---EQUATION --->

#### Hopsan TLM adaption
