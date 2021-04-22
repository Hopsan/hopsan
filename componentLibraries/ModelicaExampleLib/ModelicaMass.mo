model ModelicaMass "ModelicaMass"
    annotation(hopsanCqsType = "Q", linearTransform="bdf2")

    NodeMechanic P1;
    NodeMechanic P2;

    parameter Real M(unit="")=0
    parameter Real B(unit="")=0
    parameter Real K(unit="")=0
    parameter Real xmax(unit="")=1
    parameter Real xmin(unit="")=0

equation
    der(P2.v)*M + P2.v*(B+P1.Zc+P2.Zc) + P2.x*K = P1.c - P2.c;
    der(P2.x) = P2.v;
    P1.v + P2.v = 0;
    P1.x + P2.x = 0;
    P1.f = P1.c + P1.Zc*P1.v;
    P2.f = P2.c + P2.Zc*P2.v;
    limitVariable(P2.x,P2.v,xmin,xmax);
        
end ModelicaMass;
