model ModelicaElectricMotor "ModelicaElectricMotor"
    annotation(hopsanCqsType = "Q, linearTransform = "trapezoid")

    input Real Ke(start=0.05);
    input Real Kt(start=0.05);
    input Real L(start=1);
    input Real R(start=1);
    NodeElectric P1;
    NodeElectric P2;
    NodeMechanicRotational P3;



equation
    P1.U-P2.U = R*P2.I + L*der(P2.I) + Ke*P3.w;
    P1.I = -P2.I;
    P3.T = Kt*P2.I;
    
    // TLM equations
    P1.U = P1.c + P1.Zc*P1.I;
    P2.U = P2.c + P2.Zc*P2.I;
    P3.T = P3.c + P3.Zc*P3.w;
    
algorithm
    
end ModelicaElectricMotor;
