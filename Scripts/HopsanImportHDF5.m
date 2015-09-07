% hopsanData = HopsanImportHDF5(fileName)
%
% The function hopsanData = HopsanImportHDF5(fileName) reads the whole 
% hfd5 file in a Matlab structure.
% This function may also with any HDF5 file not related to Hopsan.
%
% author:  Martin.Hochwallner@liu.se
% license: BSD 

function hopsanData = HopsanImportHDF5(fileName)
    fileInfo = h5info(fileName);
    hopsanData = readGroup(fileName, fileInfo);
end

function hopsanData = readGroup(fileName, grp)
    hopsanData = [];
    for i = 1:length(grp.Attributes)
        attr = grp.Attributes(i);
        hopsanData.(attr.Name) = attr.Value;
        hopsanData.([attr.Name '__Name']) = attr.Name;
    end
    for i = 1:length(grp.Datasets)
        ds = grp.Datasets(i);
        hopsanData.(ds.Name) = h5read(fileName, [grp.Name '/' ds.Name]);
        hopsanData.([ds.Name '__Name']) = ds.Name;
        for j = 1:length(ds.Attributes)
            attr = ds.Attributes(j);
            hopsanData.([ds.Name '__' attr.Name]) = attr.Value;            
        end
    end
    for i = 1:length(grp.Groups)
        gr = grp.Groups(i);
        grName = strsplit(gr.Name,'/');
        grName = grName(end);        
        grName = grName{1};
        grName1 = grName;
        if isstrprop(grName(1), 'digit')
             grName =  ['x' grName];
        end
        hopsanData.([grName '__Name']) = gr.Name;
        hopsanData.([grName '__Name1']) = grName1;
        hopsanData.(grName) = readGroup(fileName, gr);
    end    
end
%eof