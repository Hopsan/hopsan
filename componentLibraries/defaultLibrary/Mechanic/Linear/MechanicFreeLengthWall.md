### Description
![MechanicFreeLengthWall picture](freelengthwall_user.svg)

A mechanical fixed attachment with allowance for some free motion.

#### Input Variables
* **m_e** - Equivalent Mass [kg]
* **stop_pos** - The position of the stop [Position]
* **B** - Viscous Friction [Ns/m]

### Theory
The position and velocity is integrated using Newton's second law of motion, unless the end position is reached:
<!---EQUATION x_1 = \begin{cases}\dfrac{-F_1}{s B}, & x_1 > stop\_pos\\stop\_pos, & x_1 \le stop\_pos\end{cases}--->
<!---EQUATION v_1 = \begin{cases}\dfrac{-F_1}{B}, & x_1 > stop\_pos\\0, & x_1 \le stop\_pos\end{cases}--->

#### Hopsan TLM adaption
TLM boundary equation:
<!---EQUATION F_1 = \begin{cases}0, & x_1 > stop\_pos\\c_1 + Z_{c1}v_1, & x_1 \le stop\_pos\end{cases}--->

Inserted into original equation gives the implemented equations:
<!---EQUATION x_1 = \begin{cases}\dfrac{-c_1}{s (B+Z_{c1})}, & x_1 > stop\_pos\\stop\_pos, & x_1 \le stop\_pos\end{cases}--->
<!---EQUATION v_1 = \begin{cases}\dfrac{-c_1}{B+Z_{c1}}, & x_1 > stop\_pos\\0, & x_1 \le stop\_pos\end{cases}--->
