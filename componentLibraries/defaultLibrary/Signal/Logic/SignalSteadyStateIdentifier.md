### Description
![SignalSteadyStateIdentifier picture](SignalSteadyStateIdentifier.svg)

Contains a steady-state identifier component, which checks if the input is at steady state.

#### Input Variables
* **in** - Input value [-]
* **method** - Which identification method to use [-]
* **tol** - Tolerance [-]
* **wl** - Length of sliding window (for methods 1 and 2) [-]
* **sd** - Standard deviation of white noise (for methods 2 and 3) [-]
* **l1** - Filter factor 1 (for method 3) [-]     
* **l2** - Filter factor 2 (for method 3) [-]       
* **l3** - Filter factor 3 (for method 3) [-]       

#### Output Variables
* **out** - Output value (1 if steady-state, else 0) [-]

### Theory

## Method 1: Rectangular Sliding Window
The rectangular sliding window is a simple method.
The signal is considered to be at steady-state if the minimum and maximum value within a sliding window is smaller than the tolerance:
<!---EQUATION \begin{align}SS(t_i) = \max\left(y(t)\right)-\min\left(y(t)\right) < y_{tol}\\\forall t \in t_i-\Delta t_w ... t_i \end{align} --->

## Method 2: Variance Ratio Test (F-test)
Steady-state can be calculated using the fraction of two differently estimated variances. If the signal is noise free, this fraction will equal one.

<!---EQUATION SS = \frac{\sigma_1^2}{\sigma_2^2} < tol --->

The first variance is estimated using the mean square deviations from the average value:
<!---EQUATION \sigma_1^2(y) =\frac{1}{n-1} \sum_{i=1}^n(y_i-\mu)^2 --->

The second variances is instead estimated using the mean of squared difference of successive data:
<!---EQUATION \sigma_2^2(y) =\frac{1}{n-1} \sum_{i=1}^{n-1}(y_{i+1}-y_i)^2 --->

## Method 3: Exponentially weighted moving average (R-test)
The performance of the F-test method can be improved by using a moving average instead of a sliding window. The R-test method also defines steady-state as the quotient of two variances:

<!---EQUATION SS = \frac{\sigma_1^2}{\sigma_2^2} < tol --->

The first variance is calcualted using the mean square deviation,

<!---EQUATION \sigma_1^2(y)= \frac{2-\lambda_1}{2}v_{f,i}^2 = \frac{2-\lambda_1}{2}\lambda_2\left(y_i-y_{f,i-1}\right)^2 v_{f,i-1}^2 --->

where

<!---EQUATION y_{f,i} = \lambda_1 y_i+\left(1-\lambda_1\right)y_{f,i-1} --->

The second variance is calculated using the mean square difference:

<!---EQUATION \sigma_2^2(y)= \frac{\delta_{f,i}^2}{2}=\frac{\lambda_3\left(y_i-y_{i-1}\right)^2+\left(1-\lambda_3\right)\delta_{f,i-1}^2}{2} --->
