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

## myinterp1

## Author: Peter Nordin <petno25@petno25-ws>
## Created: 2014-06-13

function [ ret ] = myinterp1 (x, i1, i2, v1, v2)

ret = v1 + (x-i1)*(v2-v1)/(i2-i1);
end
