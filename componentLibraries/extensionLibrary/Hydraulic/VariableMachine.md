### Description
![Illustration of flow paths within the component](leakageDescription.svg)

Hydraulic pump/motor with internal and external laminar leakage. The displacement setting is variable, but the setting includes no dynamics.

### Governing equations
The flows through the different ports are calculated as
<!---EQUATION q_{ideal} = \epsilon D \omega / \left(2 \pi \right) --->
<!---EQUATION q_{1} = -q_{ideal} + c_{li}\left(p_{2}-p_{1}\right)+c_{le}\left(p_{3}-p_{1}\right) --->
<!---EQUATION q_{2} = q_{ideal} - c_{li}\left(p_{2}-p_{1}\right)-c_{le}\left(p_{2}-p_{3}\right) --->
<!---EQUATION q_{3} = c_{le}\left(p_{1}+p_{2} - 2p_{3}\right) --->

The pressures are calculated acording to the TLM-equations
<!---EQUATION p_{1} = c_{1} + q_{1} Z_{c1} --->
<!---EQUATION p_{2} = c_{2} + q_{2} Z_{c2} --->
<!---EQUATION p_{3} = c_{3} + q_{3} Z_{c3} --->
