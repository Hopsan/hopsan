model ModelicaOrifice "ModelicaOrifice"
    annotation(hopsanCqsType = "Q", linearTransform="bdf2")

    NodeHydraulic P1;
    NodeHydraulic P2;

    parameter Real Cq(unit="-")=0.67
    parameter Real A(unit="m^2")=1e-5
    parameter Real rho(unit="kg/m^3")=880
    Real Ks(start=0);

initial algorithm
    Ks := Cq*A*sqrt(2.0/rho);

algorithm
    P1.q := -P2.q;

equation
    // TLM equations
    P2.q = turbulentFlow(P1.c,P2.c,P1.Zc,P2.Zc,Ks);
    P1.p = P1.c - P1.Zc*P2.q;
    P2.p = P2.c + P2.Zc*P2.q;
    preventCavitation(P1.p,P1.c,P1.Zc);
    preventCavitation(P2.p,P2.c,P2.Zc);
        
end ModelicaOrifice;
