model Modelica22Valve "2/2 Valve"
    annotation(hopsanCqsType = "Q", linearTransform="bdf2")

    input Real x;
    NodeHydraulic P1;
    NodeHydraulic P2;

    output Real xv;
    Real vv(start=0);
    Real Ks(start=0);
    
    parameter Real omega(unit="")=1000
    parameter Real f(unit="")=1e-4
    parameter Real B(unit="Ns/m")=1
    parameter Real Cq(unit="")=0.67
    parameter Real rho(unit="kg/m^3")=880
    parameter Real d(unit="m")=0.001


equation
    // TLM equations
    der(vv)/omega/omega + vv*2*B/omega + xv = x;
    der(xv) = vv;
    Ks = Cq*d*3.1415*xv*sqrt(2.0/rho);
    P2.q = turbulentFlow(P1.c,P2.c,P1.Zc,P2.Zc,Ks);
    P1.q+P2.q=0;
    P1.p = P1.c + P1.Zc*P1.q;
    P2.p = P2.c + P2.Zc*P2.q;
    
algorithm
    
end Modelica22Valve;
