model Modelica22Valve "2/2 Valve"
    annotation(hopsanCqsType = "Q", linearTransform="trapezoid")

    input Real x;
    NodeHydraulic P1;
    NodeHydraulic P2;

    output Real xv;
    Real Ks(start=0);
    Real K1(start=0);
        
    parameter Real omega(unit="")=1000
    parameter Real f(unit="")=1e-4
    parameter Real B(unit="Ns/m")=1
    parameter Real Cq(unit="")=0.67
    parameter Real rho(unit="kg/m^3")=880
    parameter Real d(unit="m")=0.001

initial algorithm
    K1 := Cq*d*3.1415*sqrt(2.0/rho);

equation
    // TLM equations
    Ks = K1*xv;
    der(der(xv))/omega/omega + der(xv)*2*B/omega + xv = x;
    P2.q = turbulentFlow(P1.c,P2.c,P1.Zc,P2.Zc,Ks);
    P1.q+P2.q=0;
    P1.p = P1.c + P1.Zc*P1.q;
    P2.p = P2.c + P2.Zc*P2.q;
    preventCavitation(P1.p,P1.c,P1.Zc);
    preventCavitation(P2.p,P2.c,P2.Zc);
    
end Modelica22Valve;
