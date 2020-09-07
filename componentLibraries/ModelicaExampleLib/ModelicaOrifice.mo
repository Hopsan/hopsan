model ModelicaOrifice "ModelicaOrifice"
    annotation(hopsanCqsType = "Q", linearTransform="bdf2")

    NodeHydraulic P1;
    NodeHydraulic P2;

    parameter Real Cq(unit="-")=0.67
    parameter Real A(unit="m^2")=1e-5
    parameter Real rho(unit="kg/m^3")=880
    
equation
    // TLM equations
    P2.q = Cq*A*sign(P1.p-P2.p)*sqrt(2/rho*abs(P1.p-P2.p));
    P1.q+P2.q=0;
    P1.p = max(0,P1.c + P1.Zc*P1.q);
    P2.p = max(0,P2.c + P2.Zc*P2.q);
        
end ModelicaOrifice;
