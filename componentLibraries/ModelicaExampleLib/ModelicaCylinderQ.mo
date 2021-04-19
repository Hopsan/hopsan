model ModelicaCylinderQ "ModelicaCylinderQ"
    annotation(hopsanCqsType = "Q, linearTransform = "trapezoid")

    input Real B(start=1000);
    input Real k(start=0);
    input Real A1(start=0.001);
    input Real A2(start=0.001);
    input Real betae(start=1e9);
    input Real Vd1(start=0.0003);
    input Real Vd2(start=0.0003);
    input Real Kleak(start=1e-11);
    NodeHydraulic P1;
    NodeHydraulic P2;
    NodeMechanic P3;

    parameter Real m(unit="kg")=100
    parameter Real sl(unit="m")=1

    Real qleak(start=0);
    Real V1(start=0);
    Real V2(start=0.001);
    
algorithm
    V1 := Vd1 + P3.x*A1;
    V2 := Vd2 + (sl-P3.x)*A2;

equation
    P1.p = P1.c + P1.Zc*P1.q;
    P2.p = P2.c + P2.Zc*P2.q;
    P3.f = P3.c + P3.Zc*P3.v;
    
    der(P3.v)*m + der(P3.x)*B + P3.x*k = P1.p*A1 - P2.p*A2 - P3.f;
    der(P3.x) = P3.v;
    der(P1.p) = (-P1.q-qleak-A1*P3.v)*betae/V1;
    der(P2.p) = (-P2.q+qleak+A2*P3.v)*betae/V2;

    qleak = (P1.p-P2.p)*Kleak;
    limitVariable(P3.x,P3.v,0,sl);
    limitVariable(P1.p,0,1e100);
    limitVariable(P2.p,0,1e100);
    
end ModelicaCylinderQ;
