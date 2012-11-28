#This is a suggestion on how components, ports and variables can be wrapped into objects:

#Accepted characters in name strings:
letters=["A","B","C","D","E","F","G","H","I","J","K","L","M","N","O","P","Q","R","S","T","U","V","W","X","Y","Z","a","b","c","d","e","f","g","h","i","j","k","l","m","n","o","p","q","r","s","t","u","v","w","x","y","z","1","2","3","4","5","6","7","8","9","0","_"]

#Numbers, used to make sure first character in variable names is not a number
numbers=["0","1","2","3","4","5","6","7","8","9"]

#Translation between variable names and abreviations (to be extended)
variableNames = ["Value", "Pressure", "Flow", "Position", "Velocity", "Force"]
variableAliases = ["i", "p", "q", "x", "v", "f"]


#Wrapper classes for PyVariables

class variableWrapper:
    def __init__(self, name, parentPort):
        self.name = name        
        self.port = parentPort
    
    def data(self):
        #print 'Self is', hopsan.component(self.port.component.name).port(self.port.name).data(self.name) 
        #return hopsan.component(self.port.component.name).port(self.port.name).data(self.name)
        if self.port == 0:
            return hopsan.getLogDataHandler().data(self.name)
        else:
            return hopsan.getLogDataHandler().data(self.port.component.name+"#"+self.port.name+"#"+self.name)
               
        
    def __add__(self,other):
      thisname=self.fullName()
      if type(other) is int:
            return hopsan.getLogDataHandler().addVariablesWithScalar(thisname,other)
      else:
        othername=other.fullName()
        tempvarname = hopsan.getLogDataHandler().addVariables(thisname, othername)
        print "tempvarname is"+tempvarname
        return variableWrapper(tempvarname, 0)          
      
    def __mul__(self,other):
       thisname=self.fullName()
       if type(other) is int:
            return hopsan.getLogDataHandler().multVariablesWithScalar(thisname,other)
       else:
            othername=other.fullName()
            tempvarname = hopsan.getLogDataHandler().multVariables(thisname, othername)
            print "tempvarname is"+tempvarname
            return variableWrapper(tempvarname, 0)
        
    def __sub__(self,other):
       thisname=self.fullName()
       if type(other) is int:
            return hopsan.getLogDataHandler().subVariablesWithScalar(thisname,other)
       else:
            othername=other.fullName()
            tempvarname = hopsan.getLogDataHandler().subVariables(thisname, othername)
            print "tempvarname is"+tempvarname
            return variableWrapper(tempvarname, 0)
       
    def __div__(self,other):
       thisname=self.fullName()
       if type(other) is int:
            return hopsan.getLogDataHandler().divVariablesWithScalar(thisname,other)
       else:
            othername=other.fullName()
            tempvarname = hopsan.getLogDataHandler().divVariables(thisname, othername)
            print "tempvarname is"+tempvarname
            return variableWrapper(tempvarname, 0)
       
    def __ilshift__(self,other):
       thisname=self.fullName()
       othername=other.fullName()
       tempvarname = hopsan.getLogDataHandler().assignVariables(thisname, othername)
       print "tempvarname is"+tempvarname
       return variableWrapper(tempvarname, 0)
       
    def __del__(self):
       print "del is being called for "+self.fullName()
       hopsan.getLogDataHandler().delVariables(self.fullName())
       
    def fullName(self):
       if self.port is 0:
         return self.name
       else:
         return self.port.component.name+"#"+self.port.name+"#"+self.name
       
    def save(self):
       print  "nname is"+self.nname()
       tempvarname = hopsan.getLogDataHandler().saveVariables(self.fullName(), self.nname())
       #TODO check if tempvarname is empty and tehn give error
       if self.port is not 0:
         hopsan.getLogDataHandler().delVariables(self.fullName())
       self.port = 0
       self.name = tempvarname
       print "tempvarname is"+tempvarname
       return tempvarname
       
       
    def peek(self,index):
       thisname=self.port.component.name+"#"+self.port.name+"#"+self.name
       return hopsan.getLogDataHandler().peekVariables(thisname,index)
       
    def poke(self,index,value):
       return hopsan.getLogDataHandler().pokeVariables(self.fullName(), index, value)

       
    def names(self):
       for k,v in globals().iteritems():
        if v is self:
         print k
        
    def nname(self):
      for k,v in globals().iteritems():
          if v is self:
            return k

class portWrapper:
    def __init__(self, name, variableNames, parentComponent):
        self.name = name
        self.component = parentComponent
        for v in range(len(variableNames)):
            exec "self."+variableNames[v]+" = variableWrapper(variableNames[v], self)"

class componentWrapper:
    def __init__(self, name, portNames, portVariables):
        self.name = name
        for p in range(len(portNames)):
            portName = portNames[p]
            if portNames[p] == "in":
                portName = "input"
            if portNames[p] == "out":
                portName = "output"  
            exec "self."+portName+" = portWrapper(portNames[p], portVariables[p], self)"    


    
    
#PyVar = add(arg)    
#Var1 = PyVar;


#First we want to generate component wrappers. We loop through components, variables and ports to create and execute a command like this:
# My_Component = componentWrapper("My Component", ["P1", "P2"], [["Pressure", "Flow"], ["Force","Position","Velocity"]])

#Get component names
compNames = hopsan.componentNames()
#PyVar = (a)._add_(b)
#Step.output.Value + Step.output.Value

#Loop through them
for c in compNames:
    
    #Fix the component name so that it works as a Python variable
    c_fixed = c.replace(" ", "_")  
    c_fixed = filter(lambda c: c in letters, c_fixed)
    if c_fixed[0] in numbers:
        c_fixed = "_"+c_fixed

    #Initiate the command variables (executed later)
    cmd = c_fixed+" = componentWrapper(\""+c+"\", ["
    cmd_ports = ""
    cmd_vars = ""

    #Loop through the ports 
    portNames = hopsan.component(c).portNames()
    for p in portNames:
        cmd_ports = cmd_ports+"\""+p+"\","
        cmd_vars = cmd_vars+"["

        #Loop through variables in the port
        varNames = hopsan.component(c).port(p).variableNames()
        for v in varNames:
            cmd_vars = cmd_vars+"\""+v+"\","  
        if cmd_vars[-1:] == ",":        
            cmd_vars = cmd_vars[:-1]
        cmd_vars = cmd_vars+"],"
        
    cmd_vars = cmd_vars[:-1]
    cmd_ports = cmd_ports[:-1]

    #Print the generated command (for debug only)
    print cmd+cmd_ports+"], ["+cmd_vars+"])"

    #Execute the command:
    exec cmd+cmd_ports+"], ["+cmd_vars+"])"

    #Now we want to create aliases for each variable in each port, to make it easier to use. We ant commands like this:
    # MyComponent.P1.p = MyComponent.P1.Pressure.data()
    for p in portNames:
        varNames = hopsan.component(c).port(p).variableNames()
        for v in varNames:
            if v in variableNames:            
                v_alias = variableAliases[variableNames.index(v)]
                portName = p                
                if p == "in":
                    portName = "input"
                if p == "out":
                    portName = "output"
                print c_fixed+"."+portName+"."+v_alias+" = "+c_fixed+"."+portName+"."+v+".data()"
                exec c_fixed+"."+portName+"."+v_alias+" = "+c_fixed+"."+portName+"."+v+".data()"
         
   
#Now user can write short commands like this:
#my_flow = MyComponent.P1.q

