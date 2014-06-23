% Author: Peter Nordin <petno25@petno25-ws>
% Created: 2014-06-13

function [ ret ] = myinterp1 (x, i1, i2, v1, v2)
    ret = v1 + (x-i1)*(v2-v1)/(i2-i1);
end
