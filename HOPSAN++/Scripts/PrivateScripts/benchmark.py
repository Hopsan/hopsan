import random

def runBenchmarking(iterations, threads, maxload):

    hopsan.turnOffProgressBar()

    file_list = []
    for i in range(threads+1):
        print "output"+str(i)+".csv"
        file = open("output"+str(i)+".csv", 'w+')
        file_list.append(file)

    for i in range(iterations):
        t = i%(threads+1)
        print(t)        
        if (t == 0):
            hopsan.useSingleCore()
        else:
            hopsan.useMultiCore()
            hopsan.setNumberOfThreads(t)

        load = maxload*random.random()
        print(load)
        hopsan.setSystemParameter("load", load)

        hopsan.simulate()
        time = hopsan.getSimulationTime()
  
        file_list[t].write(str(time)+", "+str(load)+"\n")

