### Description
![SignalHysteresis picture](SignalHysteresis.svg)

Adds hysteresis of specified width to a signal

#### Input Variables
* **in** -  [-]
* **y_h** - Width of the Hysteresis [-]

#### Output Variables
* **out** -  [-]

### Theory
The absolute value of the difference between input and output will be kept at the hysteresis width whenever possible.

<pre>
if(in_{n-1} < in_n-y_h/2)
{
    out = in_n-y_h/2
}
else if (in_{n-1} > in_n+y_h/2.0)
{
    out = in_n+y_y/2
}
else
{
    out = in_{n-1}
}
</pre>

