% author:  Martin.Hochwallner@liu.se
% license: BSD 

%%
fileName = 'testfile.h5'


%% Read full file with import script
% This reads the whole hfd5 file in a Matlab structure.
data = HopsanImportHDF5(fileName)


%% manualy reading parts of the file
% This example is based on the results from 'Sub System Example.hmf'.

% Display the content of the file (data sets and attributes but not the
% data itself. 
h5disp(fileName)

info = h5info(fileName)

% Reading single data sets
data = [];
data.time = h5read(fileName, '/results/Time');
%h5disp(fileName, '/results/Position_Transducer')
data.Position_Transducer = h5read(fileName, '/results/Position_Transducer/out/Value');

data

% eof