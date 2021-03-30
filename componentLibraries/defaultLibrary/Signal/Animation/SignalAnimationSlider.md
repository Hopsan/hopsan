### Description
![SignalAnimationSlider picture](slider_user.svg)

Signal adjustable slider. Used to control variable values by dragging the mouse during interactive animation.

#### Input Variables
* **min** - Minimum input value [-]
* **max** - Minimum output value [-]
* **in** - Input signal (between 0 and 1) [-]

#### Output Variables
* **out** - Output signal [-]

### Theory
In non-interactive simulation the output will be computed based on the input value and the minimum and maximum limits. Input is assumed to be between 0 and 1.
<!---EQUATION out = min + (max - min)\cdot in--->

