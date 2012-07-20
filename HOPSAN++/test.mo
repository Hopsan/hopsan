connector NodeHydraulic "Hydraulic Node"
   Real		p "Pressure"
   Real		q "Flow"
   Real 	c "Wave Variable"
   Real		Zc "Characteristic Impedance
end Pin;


model LaminarOrifice "Hydraulic Laminar Orifice"
   parameter Real Kc(unit="-")=1e-11 "Pressure-Flow Coefficient";
   NodeHydraulic P1, P2;
equation
   P2.q = Kc*(P1.p-P2.p);
   P1.q = -P2.q;
   P1.p = P1.c + P1.Zc*P1.q;
   P2.p = P2.c + P2.Zc*P2.q;
end LaminarOrifice;
