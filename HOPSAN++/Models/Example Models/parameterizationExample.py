from math import *

stacks = {}

def addVerticalStack( stackname, basename, type, x, y, rot, dist, n ):
  stack = []
  for i in range(0, n):
    print(basename+str(i))
    print(hopsan.addComponent(basename+str(i), type, x,y-(n/2.0-0.5-i)*dist,rot))
    stack.append(basename+str(i))
  global stacks
  stacks[stackname]=stack

def addHorizontalStack( stackname, basename, type, x, y, rot, dist, n ):
  stack = []
  for i in range(0, n):
    print(basename+str(i))
    print(hopsan.addComponent(basename+str(i), type, x-(n/2.0-0.5-i)*dist,y,rot))
    stack.append(basename+str(i))
  global stacks
  stacks[stackname]=stack

def connectStackToComponent( stackname, port1, component, port2):
  for i in stacks[stackname]:
    hopsan.connect(i, port1, component, port2)

def connectStackToStack( stackname1, port1, stackname2, port2):
  for i in range(len(stacks[stackname1])):
    hopsan.connect(stacks[stackname1][i], port1, stacks[stackname2][i], port2)



def generateSprings( n ):
  global stacks
  stacks = {}

  hopsan.addComponent("My Subsystem", "Subsystem", 2500, 2500, 0)
  hopsan.enterSystem("My Subsystem")

  hopsan.addComponent("P1", "HopsanGUIContainerPort", 2300, 2500, 0)
  hopsan.addComponent("Mass1", "MechanicMultiPortTranslationalMass", 2400,2500,0)
  addVerticalStack("SpringStack", "Spring", "MechanicTranslationalSpring", 2600, 2500, 0, 50, n)
  hopsan.addComponent("Mass2", "MechanicMultiPortTranslationalMass", 2800,2500,0)
  hopsan.addComponent("P2", "HopsanGUIContainerPort", 2900, 2500, 0)

  hopsan.connect("P1", "P1", "Mass1", "P1")
  connectStackToComponent("SpringStack", "P1", "Mass1", "P2")
  connectStackToComponent("SpringStack", "P2", "Mass2", "P1")
  hopsan.connect("P2", "P2", "Mass2", "P2")

  hopsan.exitSystem()



def generateConstantPressureServo( n ):
  hopsan.clear()

  hopsan.addComponent("Tank 1",           "HydraulicTankC",                   2570, 2700, 0)
  hopsan.addComponent("Pump",             "HydraulicPressureControlledPump",  2555, 2600, 0)
  hopsan.addComponent("Volume",           "HydraulicVolumeMultiPort",         2570, 2500, 0)
  hopsan.addComponent("Pressure Source",  "HydraulicPressureSourceC",         2490, 2700, 0)

  addHorizontalStack("ValveStack",  "Valve",  "Hydraulic43Valve",          2500, 2400, 0,   300, n)
  addHorizontalStack("TankStack",   "Tank",   "HydraulicTankC",            2530, 2450, 0,   300, n)
  addHorizontalStack("PistonStack", "Piston", "HydraulicCylinderC",        2530, 2330, 0,   300, n)
  addHorizontalStack("MassStack",   "Mass",   "MechanicTranslationalMass", 2650, 2330, 0,   300, n)
  addHorizontalStack("ForceStack",  "Force",  "MechanicForceTransformer",  2720, 2330, 180, 300, n)
  
  hopsan.connect("Tank 1",          "P1", "Pump",   "P1")
  hopsan.connect("Pump",            "P2", "Volume", "P1")
  hopsan.connect("Pressure Source", "P1", "Pump",   "PREF")
  
  connectStackToComponent("ValveStack", "PP", "Volume", "P1")
  
  connectStackToStack("ValveStack",   "PT", "TankStack",    "P1")
  connectStackToStack("ValveStack",   "PA", "PistonStack",  "P1")
  connectStackToStack("ValveStack",   "PB", "PistonStack",  "P2")
  connectStackToStack("PistonStack",  "P3", "MassStack",    "P1")
  connectStackToStack("MassStack",    "P2", "ForceStack",   "P1")
  
  

