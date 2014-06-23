% Generates test data for 2D lookup table
% Author: Peter Nordin
% $Id$

% Create Test data
for choice=0:1

%---------------------------------------
if choice==0
    ctr=0;
    doRand=0;
    rowIdx = [1,2,3,4,5];
    colIdx = [1,2,3,4,5];

    testPts(1,:) = [2.5, 2.5];
    testPts(end+1,:) = [2.5, 3.8];
    testPts(end+1,:) = [4.67789, 1.8345];
    testPts(end+1,:) = [4.9, 5.0];
    %testPts(end+1,:) = [6.0, 4.9];

%---------------------------------------
elseif choice==1
    ctr=-20;
    doRand=1;
    rowIdx = 1:1:10;
    colIdx = 1:0.5:10;

    testPts(1,:) = [2.5, 2.5];
    testPts(end+1,:) = [2.5, 3.8];
    testPts(end+1,:) = [4.67789,1.8345];
    testPts(end+1,:) = [4.9,5.0];
    testPts(end+1,:) = [4.9,6.0];
%---------------------------------------

end

% ---------- Automatic code starts here --------------------

% Create row and column index value vectors (these can be negative and floating point values)


% Get the length (number of rows and columns)
nR=length(rowIdx);
nC=length(colIdx);

% Generate a dummy data matrix for test
mat = zeros(nR, nC);
for ri=1:nR
    for ci=1:nC
        if (doRand==1)
            mat(ri,ci) = ctr*rand(1);
        else
            mat(ri,ci) = ctr;
        end
        ctr = ctr+1;
    end
end
rowIdx
colIdx
mat

% Reshape the data into three different one dimensional vectors on the form rowVal, colVal, dataVal
% Example for 2x3 matrix
% r1, c1, v11
% r1, c2, v12
% r1, c3, v13
% r2, c1, v21
% r2, c2, v22
% r2, c3, v23

% Pre-allocate vectors
out.r = zeros(1,nR*nC);
out.c = out.r;
out.d = out.r;
% Copy data into output vectors
for ri=1:nR
    for ci=1:nC
        i=(ri-1)*nC+ci; % Calculate actual 1-d reshaped index (i)
        % Write row index value
        out.r(i) = rowIdx(ri);
        % Write column index value
        out.c(i) = colIdx(ci);
        % Write matrix data value
        out.d(i) = mat(ri,ci);
    end
end

% Determine output file name
name = strcat('2DTestData',num2str(choice));

% Save data to CSV file as three columns,
% Also we add the number of data rows and data columns to the final line of the data
dlmwrite(strcat(name,'.csv'), [out.r, nR; out.c, nC; out.d, 0]');

% Generate test data file for unit tests
fh = fopen(strcat(name, '_UT.dat'), 'w');
fprintf(fh, '%.10g ', rowIdx); fprintf(fh, '\n');
fprintf(fh, '%.10g ', colIdx); fprintf(fh, '\n');
fprintf(fh, '%.10g ', out.d); fprintf(fh, '\n');
% Now write some test cases and the results from the octave lookup function
for i=1:size(testPts,1)
    writeUTTestDataLine(fh, rowIdx, colIdx, mat, testPts(i,:));
end

% Close file
fclose(fh);

% Cleanup
clear testPts

end
