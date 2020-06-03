classdef hopsan
    methods
        function checkMessages(obj)
            bufsize=1000;
            buf=libpointer('string',blanks(bufsize));
            status = 0;
            while (status == 0)
                [status,msg] = calllib('hopsanc','getMessage',buf,bufsize);    
                disp(msg);
            end
        end
        function obj = hopsan()
            if ispc
                path = fullfile(getenv("HOPSANHOME"), '\bin\hopsanc.dll');
                complibpath = fullfile(getenv("HOPSANHOME"), '\componentLibraries\defaultLibrary\defaultcomponentlibrary.dll');
            elseif isunix
                path = fullfile(getenv("HOPSANHOME"), '\bin\libhopsanc.so');
                complibpath = fullfile(getenv("HOPSANHOME"), '\componentLibraries\defaultLibrary\libdefaultcomponentlibrary.so');
            end
            hpath = fullfile(getenv("HOPSANHOME"), '\hopsanc\include\hopsanc.h');
            loadlibrary(path,hpath);
            if isfile(complibpath)
                calllib('hopsanc','loadLibrary',complibpath);
            end
            obj.checkMessages();
        end

        function setStartTime(obj,t)
            calllib('hopsanc','setStartTime',t);
            obj.checkMessages();
        end
        function setTimeStep(obj,t)
            calllib('hopsanc','setTimeStep',t);
            obj.checkMessages();
        end
        function setStopTime(obj,t)
            calllib('hopsanc','setStopTime',t);
            obj.checkMessages();
        end
        function setLogSamples(obj,t)
            calllib('hopsanc','setNumberOfLogSamples',t);
            obj.checkMessages();
        end
        function loadModel(obj,path)
            calllib('hopsanc','loadModel',path);
            obj.checkMessages();
        end
        function loadLibrary(obj,path)
            calllib('hopsanc','loadLibrary',path);
            obj.checkMessages();
        end
        function simulate(obj)
            calllib('hopsanc','simulate');
            obj.checkMessages();
        end
        function t = getTimeVector(obj)
            nSamples = calllib('hopsanc','getNumberOfLogSamples');
            buf = zeros(nSamples,1);
            [status,t] = calllib('hopsanc','getTimeVector',buf);
            obj.checkMessages();
        end
        function x = getDataVector(obj,name)
            nSamples = calllib('hopsanc','getNumberOfLogSamples');
            buf = zeros(nSamples,1);
            [status,name,x] = calllib('hopsanc','getDataVector',name,buf);
            obj.checkMessages();
        end
        function setParameter(obj,name,value)
            calllib('hopsanc','setParameter',name,value);
            obj.checkMessages();
        end
        function delete(obj)
            unloadlibrary('hopsanc');
        end
    end
end
