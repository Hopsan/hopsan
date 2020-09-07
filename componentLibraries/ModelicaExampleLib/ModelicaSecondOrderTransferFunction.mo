model ModelicaSecondOrderTransferFunction "Second Order Transfer Function"
    annotation(hopsanCqsType = "S", linearTransform="bdf2")

    input Real in;
    output Real out;

    Real din(start=0);
    Real dout(start=0);
    parameter Real a(unit="")=0
    parameter Real b(unit="")=2
    parameter Real c(unit="")=1
    parameter Real d(unit="")=0
    parameter Real e(unit="")=0
    parameter Real f(unit="")=1


equation
    din=der(in);
    dout=der(out);
    a*der(din)+b*din+c*in = d*der(dout)+e*dout+c*out;
    
end ModelicaSecondOrderTransferFunction;
