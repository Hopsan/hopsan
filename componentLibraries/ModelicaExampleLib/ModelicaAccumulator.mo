model ModelicaAccumulator "Accumulator"
    annotation(hopsanCqsType = "Q, linearTransform = "trapezoid")

    output Real Va;
    output Real pa;
    output Real xmp;
    output Real vmp;
    NodeHydraulic P1;

    parameter Real V0(unit="m^3")=0.001
    parameter Real Kca(unit="m^3/(sPa)")=1e-8
    parameter Real kappa(unit="")=1.2
    parameter Real p0(unit="Pa")=1e7

    Real SL(start=1);
    Real Ap;
    Real Bp;
    Real fg;
    
initial algorithm
    SL := 1;
    Ap := V0/SL;
    Bp := pow(Ap,2)/Kca;
 
algorithm
    fg := Ap*P1.p - Ap*pa;

equation       
    Bp*vmp = fg;
    Bp*der(xmp) = fg;
    P1.q = -Ap*vmp;
    pa*pow((SL-xmp)*Ap,kappa) = p0*pow((SL*Ap),kappa);
    Va = (SL-xmp)*Ap;
    P1.p = P1.c + P1.Zc*P1.q;
    limitVariable(xmp,vmp,0,SL);
    limitVariable(P1.p,0,1e100);
    
end ModelicaAccumulator;
