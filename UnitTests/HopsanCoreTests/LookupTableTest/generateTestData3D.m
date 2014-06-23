% Generates test data for 3D lookup table
% Author: Peter Nordin
% $Id$

for choice=0:1

% Create Test data

%---------------------------------------
if choice==0
    ctr=0;
    doRand=0;
    rowIdx = 1:1:2;
    colIdx = 1:1:3;
    planeIdx = 1:1:4;
    testPts(1,:) = [1, 1, 1];
    testPts(end+1,:) = [1.99, 2.99, 3.99];

%---------------------------------------
elseif choice==1
    ctr=-20;
    doRand=1;
    rowIdx = -14.16:1.75:12;
    colIdx = -32:1.88:19;
    planeIdx = -3:0.012:-1.8;
    testPts(1,:) = [1, 1, -2.99999999999999];
    testPts(end+1,:) = [1.5, 1.5, -2.5];
    testPts(end+1,:) = [-2, 14, -2.1];
    testPts(end+1,:) = [-13.6, 12, -2.23];
    testPts(end+1,:) = [-13.5, 12, -2.25];
    testPts(end+1,:) = [8, 12, -2.23];
    testPts(end+1,:) = [10, 16, -2];

%---------------------------------------

end


% ---------- Automatic code starts here --------------------

% Get the length (number of rows, columns and planes)
nR=length(rowIdx);
nC=length(colIdx);
nP=length(planeIdx);

% Generate a dummy data matrix for test
mat = zeros(nR, nC, nP);
for ri=1:nR
    for ci=1:nC
        for pi=1:nP
            if (doRand==1)
                mat(ri,ci,pi) = ctr*rand(1);
            else
                mat(ri,ci,pi) = ctr;
            end
            ctr = ctr+1;
        end
    end
end
%rowIdx
%colIdx
%planeIdx
%mat

% Reshape the data into three different one dimensional vectors on the form rowVal, colVal, planeVal, dataVal
% Example for 2x2x2 matrix
% r1, c1, p1, v111
% r1, c1, p2, v112
% r1, c2, p1, v121
% r1, c2, p2, v122
% r2, c1, p1, v211
% r2, c1, p2, v212
% r2, c2, p1, v221
% r2, c2, p2, v222


% Pre-allocate memory for output vectors
out.r = zeros(1,nR*nC*nP);
out.c = out.r;
out.p = out.r;
out.d = out.r;
% Copy data into output vectors
for ri=1:nR
    for ci=1:nC
        for pi=1:nP
            i=(ri-1)*nC*nP+(ci-1)*nP+pi; % Calculate actual 1-d reshaped index (i)
            % Write row index value
            out.r(i) = rowIdx(ri);
            % Write column index value
            out.c(i) = colIdx(ci);
            % Write plane index value
            out.p(i) = planeIdx(pi);
            % Write matrix data value
            out.d(i) = mat(ri,ci,pi);
        end
    end
end

% Determine output file name
name = strcat('3DTestData',num2str(choice));

% Save data to CSV file as four columns,
% Also we add the number of data rows, columns and planes to the final line of the data
dlmwrite(strcat(name,'.csv'), [out.r, nR; out.c, nC; out.p, nP; out.d, 0]');

% Generate test data file for unit tests
fh = fopen(strcat(name, '_UT.dat'), 'w');
fprintf(fh, '%.10g ', rowIdx);     fprintf(fh, '\n');
fprintf(fh, '%.10g ', colIdx);     fprintf(fh, '\n');
fprintf(fh, '%.10g ', planeIdx);   fprintf(fh, '\n');
fprintf(fh, '%.10g ', out.d);      fprintf(fh, '\n');
% Now write some test cases and the results from the octave lookup function
for i=1:size(testPts,1)
    writeUTTestDataLine(fh, rowIdx, colIdx, planeIdx, mat, testPts(i,:));
end

% Close file
fclose(fh);

% Cleanup
clear testPts

end
