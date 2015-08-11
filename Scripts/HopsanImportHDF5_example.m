fileName = 'C:\data\InfLinActuator/testHopsan4.h5'

%% Read full file with import script
data = HopsanImportHDF5(fileName)


%% manualy reading parts of the file
h5disp(fileName)

info = h5info(fileName)

data = [];
data.time = h5read(fileName, '/results/Time');
%h5disp(fileName, '/results/Position_Transducer')
data.Position_Transducer = h5read(fileName, '/results/Position_Transducer/out/Value');

data
