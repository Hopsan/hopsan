### Description
![SignalCounter picture](SignalCounter.svg)

Contains a signal flank counter component

#### Input Variables
* **r** - Count rising flags [-]
* **f** - Count falling flags [-]
* **in** -  [-]

#### Output Variables
* **out** - Number of counted flanks [-]

### Theory
Counts the number of rising and/or falling flanks. A rising flank is defined as going from a value above 0.5 to a value below. A falling flank is defined as going from a value below 0.5 to a value above.
