## Copyright (C) 2014 Peter Nordin
## 
## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2 of the License, or
## (at your option) any later version.
## 
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
## 
## You should have received a copy of the GNU General Public License
## along with Octave; see the file COPYING.  If not, see
## <http://www.gnu.org/licenses/>.

## myinterp2Eval

## Author: Peter Nordin <petno25@petno25-ws>
## Created: 2014-06-13

%function [ ret ] = myinterp2Eval ()

rowIdx = [1,2,3,4,5];
colIdx = [1,2,3,4,5];

ctr=0;
mat = zeros(length(rowIdx), length(colIdx));
for ri=1:length(rowIdx)
    for ci=1:length(colIdx)
        mat(ri,ci) = ctr;
        ctr = ctr+1;
    end
end

mat
% Note! col=X row=Y so numbers are swapped compared to matrix row,col
interp2(colIdx, rowIdx, mat, 2.5, 2.5)
interp2(colIdx, rowIdx, mat, 3.8, 2.5)
interp2(colIdx, rowIdx, mat, 1.8345, 4.67789)
interp2(colIdx, rowIdx, mat, 5.0, 4.9)
interp2(colIdx, rowIdx, mat, 6.0, 4.9)


