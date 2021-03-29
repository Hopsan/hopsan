### Description
![SignalAdditiveNoise picture](SignalAdditiveNoise.svg)

Adds additive gaussian white noise to the input signal.

#### Input Variables
* **in** - Input signal [-]
* **std_dev** - Amplitude Variance [-]

#### Output Variables
* **out** - Output signal [-]

### Theory
<!---EQUATION out = std\_dev \cdot \sqrt{(-2)\cdot log(R_1)} \cdot \cos(2\pi R_2)--->

where <i>R<sub>1</sub></i> and <i>R<sub>2</sub></i> are random numbers between 0 and 1.

#### Hopsan TLM adaption
