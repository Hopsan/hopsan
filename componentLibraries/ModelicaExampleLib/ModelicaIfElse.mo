model ModelicaIfElse "ModelicaIfElse"
    annotation(hopsanCqsType = "S", linearTransform = "bdf3")

    input Real x;
    output Real z;

algorithm
  if x>0 then
    z := sin(mTime*3);
  else
    z := 0;
  end if
    
end ModelicaIfElse;
