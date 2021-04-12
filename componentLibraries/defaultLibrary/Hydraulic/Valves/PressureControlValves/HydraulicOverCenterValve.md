### Description
![HydraulicOverCenterValve picture](overcentervalve_user.svg)

A hydraulic over center valve with first order dynamics, flow forces and hysteresis

#### Input Variables
* **tao** - Time Constant of Spool [s]
* **k_cs** - Steady State Characteristic due to Spring [LeakageCoefficient]
* **k_cf** - Steady State Characteristic due to Flow Forces [LeakageCoefficient]
* **q_nom** - Flow with Fully Open Valve and pressure drop p_nom [m^3/s]
* **p_nom** - Nominal pressure drop [Pa]
* **p_ref** - Reference Opening Pressure [Pa]
* **p_h** - Hysteresis Width [Pa]
* **a_ratio** - Area ratio [-]

#### Output Variables
* **xv** - Equivalent spool position [-]

### Theory
The valve is opened either by the upstream pressure or by the external control port. The control port pressure acts on a larger area, as specified by the ratio parameter. A turbulent flow model is used. The flow coefficient is computed from steady-state characteristics.

Differentiating the flow with respect to the control pressure (<i>p<sub>1</sub></i> - <i>p<sub>ref</sub></i> yields:
<!---EQUATION \dfrac{q}{k_{cs}} + \dfrac{q}{k_{cf}}\dfrac{\Delta p}{p_1-p_2} = p_1 - p_{ref} + a_{ratio}p_{control}--->

The relationship between nominal pressure and nominal flow is known:
<!---EQUATION q_{nom} = K_e\sqrt{p_{nom}} = \dfrac{p_1 - p_{ref} + a_{ratio}p_{control}}{\dfrac{1}{k_{cs}} + \dfrac{1}{k_{cf}}\dfrac{p_1-p_2}{p_{nom}}} --->

This can be used to compute the turbulent flow coefficient:
<!---EQUATION K_e = \dfrac{p_1 - p_{ref} + a_{ratio}p_{control}}{\dfrac{1}{k_{cs}} + \dfrac{1}{k_{cf}}\dfrac{p_1-p_2}{p_{nom}}}\dfrac{1}{\sqrt{p_{nom}}} --->

This gives the final expression for the turbulent flow:
<!---EQUATION q_2 = K_e \sqrt{p_1-p_2} = \dfrac{p_1 - p_{ref} + a_{ratio}p_{control}}{\dfrac{1}{k_{cs}} + \dfrac{1}{k_{cf}}\dfrac{p_1-p_2}{p_{nom}}}\dfrac{\sqrt{p_1-p_2}}{\sqrt{p_{nom}}}--->

Finally, hysteresis and dynamics are applied using specified hysteresis width and time constant.

