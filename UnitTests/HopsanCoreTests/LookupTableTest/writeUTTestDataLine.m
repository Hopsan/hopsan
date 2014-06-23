function writeUTTestDataLine(fid, arg2, arg3, arg4, arg5, arg6)
% Help function for generating test data for lookuptables
% Author: Peter Nordin
% $Id$
% Syntax:
%        writeUTTestDataLine(fid, idxRow, idxCol, idxPlane, dataMatrix, point)
%        writeUTTestDataLine(fid, idxRow, idxCol, dataMatrix, point)
%        writeUTTestDataLine(fid, idxRow, dataMatrix, point)

% Note! col=X row=Y, plane=Z so numbers are swapped compared to matrix row,col
if nargin==6
    point = arg6;
    point(4) = interp3(arg3, arg2, arg4, arg5, point(2), point(1), point(3));
    printf('%.10g %.10g %.10g = %.10g\n', point)
elseif nargin==5
    point = arg5;
    point(3) = interp2(arg3, arg2, arg4, point(2), point(1));
    printf('%.10g %.10g = %.10g\n', point)
elseif nargin==4
    point = [arg4, 0];
    point(2) = interp1(arg2, arg3, point(1));
    printf('%.10g = %.10g\n', point)
end

fprintf(fid, '%.10g ', point)
fprintf(fid, '\n')
