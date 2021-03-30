### Description
![SignalNoiseGenerator picture](SignalNoiseGenerator.svg)

Signal Noise Generator Component

#### Input Variables
* **std_dev** - Standard deviation [-]

#### Output Variables
* **out** -  [-]

### Theory
Source component that generates white gaussian noise.
<!---EQUATION out = std\_dev\cdot \sqrt{-2*log(R_1)}\cos(2\pi R_2)--->
where <i>R<sub>1</sub></i> and <i>R<sub>2</sub></i> are random numbers between 0 and 1.
