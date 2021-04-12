### Description
![HydraulicPressureReliefValve picture](prv_user.svg)

A hydraulic pressure relief valve with first order dynamics, flow forces and hysteresis

#### Input Variables
* **k_cs** - Steady State Characteristic due to Spring [LeakageCoefficient]
* **k_cf** - Steady State Characteristic due to Flow Forces [LeakageCoefficient]
* **q_nom** - Flow with Fully Open Valve and pressure drop Pnom [m^3/s]
* **p_max** - Maximum opening pressure [Pa]
* **tao** - Time Constant of Spool [s]
* **p_h** - Hysteresis Width [Pa]

### Theory
The valve attempts to keep the upstream pressure at the reference level. A turbulent flow model is used. The flow coefficient is computed from steady-state characteristics.

Differentiating the flow with respect to the control pressure (<i>p<sub>1</sub></i> - <i>p<sub>ref</sub></i> yields:
<!---EQUATION \dfrac{q}{k_{cs}} + \dfrac{q}{k_{cf}}\dfrac{\Delta p}{p_1-p_2} = p_1-p_{max}--->

The relationship between nominal pressure (<i>p<sub>nom</sub></i> = 70 bar) and nominal flow is known:
<!---EQUATION q_{nom} = K_e\sqrt{p_{nom}} = \dfrac{p_1-p_{max}}{\dfrac{1}{k_{cs}} + \dfrac{1}{k_{cf}}\dfrac{p_1-p_2}{p_{nom}}} --->

This can be used to compute the turbulent flow coefficient:
<!---EQUATION K_e = \dfrac{p_1-p_{max}}{\dfrac{1}{k_{cs}} + \dfrac{1}{k_{cf}}\dfrac{p_1-p_2}{p_{nom}}}\dfrac{1}{\sqrt{p_{nom}}} --->

This gives the final expression for the turbulent flow:
<!---EQUATION q_2 = K_e \sqrt{p_1-p_2} = \dfrac{p_1-p_{max}}{\dfrac{1}{k_{cs}} + \dfrac{1}{k_{cf}}\dfrac{p_1-p_2}{p_{nom}}}\dfrac{\sqrt{p_1-p_2}}{\sqrt{p_{nom}}} --->

Finally, hysteresis and dynamics are applied using specified hysteresis width and time constant.

