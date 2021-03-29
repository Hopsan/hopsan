### Description
![SignalSampleAndHold picture](SignalSampleAndHold.svg)

Sample-and-hold signal component

#### Input Variables
* **f_s** - Sampling Frequency [Hz]
* **in** - Input variable [-]

#### Output Variables
* **out** - Output variable [-]

### Theory
Output value is assigned with input value at specified sampling frequency. Between samples the output variable remain constant.
<pre>
if(t >= tnext)
{
    out = in
    tnext = tnext + 1/f_s
}
</pre>



