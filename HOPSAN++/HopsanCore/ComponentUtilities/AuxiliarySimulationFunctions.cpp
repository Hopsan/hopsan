//$Id$

//! Multiplies the value by two.
//! @param[in] input is the value to multiply.
//! @return the input value times two.
double multByTwo(double input)
{
    return 2.0*input;
}


//! @brief Limits a value so it is between min and max
//! @param &value Reference pointer to the value
//! @param min Lower limit of the value
//! @param max Upper limit of the value
void limit(double &value, double min, double max)
{
    if(min>max)
    {
        double temp;
        temp = max;
        max = min;
        min = temp;
    }
    if(value > max)
    {
        value = max;
    }
    else if(value < min)
    {
        value = min;
    }
}
