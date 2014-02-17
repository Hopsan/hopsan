# Create two python arrays (or maybe they are lists)
x=[1,2,3,4,5]
y=[3,2,3,4,2]
# ldh, is a reference to the LogDataHandler object for the current system
# Try ldh. in the python terminal for autocomplettion of all fucntions
ldh=hopsan.getLogDataHandler()

# Here we assign the Python array x and y to script variables "x" and "y" in Hopsan, if a variable does not exist ist is created
ldh.assignVariable("x",x)
ldh.assignVariable("y",y)

# Plot x,y
ldh.plot("x","y")
# You can also add two optional arguments, generation (-1 = latest) and axis (0=left 1=right)
# ldh.plot("x","y",-1,0)


# For existing variables in Hospan (after a simulation has been run), you can call the following:
# From the example model PositionServo, to plot that variable, notice the # separator sign, ComponentName#PortName#LongVariableName

# ldh.plot("Position_Sensor#out#Value")
# or
# ldh.plot("Position_Sensor#out#Value", -1, 0)
