% Generates test data for 2D lookup table
% $Id

% Create row and column index value vectors (these can be negative and floating point values)
rowIdx = 1:1:10;
colIdx = 1:0.5:10;

% Get the length (number of rows and columns)
nR=length(rowIdx);
nC=length(colIdx);

% Generate a dummy data matrix for test
ctr=-2*nC;
mat = zeros(nR, nC);
for ri=1:nR
    for ci=1:nC
        mat(ri,ci) = ctr;
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

% Save data to CSV file as three columns,
% Also we add the number of data rows and data columns to the final line of the data
dlmwrite('test2d.csv', [out.r, nR; out.c, nC; out.d, 0]');
