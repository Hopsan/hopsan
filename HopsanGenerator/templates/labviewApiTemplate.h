
#ifndef SITAPI_h
#define SITAPI_h 

#ifdef VXWORKS
	#define DLL_EXPORT
#else
	#define DLL_EXPORT __declspec(dllexport)
#endif

#define NI_OK		0
#define NI_ERROR	1

/* 
 * NOTE: This API can be used for both single rate and multirate models. 
 * number of tasks = # of tasks in the model including the scheduler task. 
 * For a single rate model, the number of tasks should be set to 1. 
 * 
 * 
 * For a multi rate model, the number of tasks should be set to the number of tasks that are not executed 
 * with the scheduler +1.
 * 
 * BUILD NOTES: the functions in this API should be thread safe and ideally be built with dynamic
 * linkage to C runtime.
 */
 
// Predefined attribute structures containing information on IO, Parameters, and Signals
// Note that removing fields from these structures may require additional modifications to codegen.c 
typedef struct {
  char*	name;		 // name of the external IO, e.g., "In1"
  int	dimX;		 // 1st dimension size
  int	dimY;		 // 2nd dimension size
} ExtIOAttributes;

typedef struct {
  char* paramname;	 // name of the parameter, e.g., "Amplitude"
  unsigned int addr; // offset of the parameter in the Parameters struct
  int datatype;		 // integer describing a user defined datatype. must have a corresponding entry in GetValueByDataType and SetValueByDataType
  int dimX; 		 // 1st dimension size
  int dimY;			 // 2nd dimension size
} ParameterAttributes;
 
typedef struct {
  	char*  blockname;// name of the block where the signals originates, e.g., "sinewave/sine"
  	int    portno;   // the port number of the block
  	char* signalname;// name of the signal, e.g., "Sinewave + In1"
  	void* addr;		 // address of the storage for the signal
  	int	 datatype;	 // integer describing a user defined datatype. must have a corresponding entry in GetValueByDataType
  	int dimX;		 // 1st dimension size
  	int dimY;		 // 2nd dimension size
} SignalAttributes;
 
extern "C" {

// Definition of user defined function for getting values of user defined types
double GetValueByDataType(void* ptr, int subindex, int type);

// Definition of user defined function for setting values of user defined types
long SetValueByDataType(void* ptr, int subindex, double value, int type);

// Definition of user defined function for doing work before model execution starts
long USER_Initialize();

// Definition of user defined function to be executed on every iteration of the baserate
void USER_TakeOneStep(double *inData, double *outData, double timestamp);

// Definition of user defined function for doing work after model execution has stopped
long USER_Finalize();

/* 
 * NIRT_InitializeModel
 * Initializes internal structures and prepares model for execution. 
 * 
 * Inputs:
 *		stoptime	: the final time till which the model should run.
 * Outputs:
 *		timestep: the base time step of the model scheduler.
 *		num_in	: # of external inputs
 *		num_out: # of external outputs
 *		num_tasks: # of Tasks (for multirate model). 
 * 
 * Return value: 0 if no error.
 */
DLL_EXPORT long NIRT_InitializeModel(double finaltime, double *outTimeStep, long *num_in, 
									long *num_out, long* num_tasks);
									
/* NIRT_PostOutputs
 * Pushes outport data out to LabVIEW for Multi-rate models.
 *
 * Outputs:
 *		outData: preallocated array of model outputs
 *
 * Return value: 0 if no error.
 */
DLL_EXPORT long NIRT_PostOutputs(double *outData);									

/* NIRT_ModelUpdate
 * Updates the internal state of the model. 
 *
 * NOTE: Each call to Schedule() or ScheduleTasks() must be followed by a call to NIRT_ModelUpdate before
 *		 the scheduling call can be made again. In the case of a multi-rate model, NIRT_ModelUpdate is 
 *		 called after NIRT_PostOutputs.
 */
DLL_EXPORT long NIRT_ModelUpdate();

/* 
 * NIRT_Schedule
 * Invokes the scheduler. This method is called every base time step. 
 * The model receives external inputs for the current time step, computes the schedule, and posts the outputs.
 * In addition, the model may choose to executes a base-rate task.
 * 
 * NOTE: 
 * 
 * Inputs: 
 *		in	: buffer with model external inputs values.
 * Outputs:
 * 		out	: model external outputs from last time step.
 *		time	: current simulation time.
 * 		dispatchlist	: list of tasks to be dispatched this time step (used for multirate models only)
 * 
 * Return value: 0 if no error.
 */
DLL_EXPORT long NIRT_Schedule(double *inData, double *outData, double *outTime, long *dispatchtasks);

/*
 * NIRT_TaskTakeOneStep
 * Advance a task one step. Called at the rate of the taskid. (for multirate models only)
 * 
 * Inputs: 
 *		taskid: Task ID of the task to be executed.
 * Return value: 0 if no error
 */
DLL_EXPORT long NIRT_TaskTakeOneStep(long taskid);

/*
 * NIRT_FinalizeModel
 * Called after simulation stops (to free up memory/resources)
 * 
 * Return value: 0 if no error
 */
DLL_EXPORT long NIRT_FinalizeModel(void);

/*
 * NIRT_ProbeSignals
 * returns the latest signal values. 
 *
 * Inputs: 
 *		sigindices: list of signal indices to be probed.
 * 		numsigs: length of sigindices array.
 * 	
 * NOTE: (Implementation detail) the sigindices array that is passed in is delimited by a value of -1. 
 * Thus the # of valid indices in the sigindices table is min(numsigs, index of value -1) - 1. 
 * Reason for subtracting 1, the first index in the array is used for bookkeeping and should be copied
 * into the 0th index in the signal values buffer. 
 * Also, the 1st index in the signal values buffer should contain the current timestamp. Hence valid 
 * signal data should be filled in starting from index 2. Any non-scalar signals should be added to the 
 * buffer in row-order. 
 * 	  
 * Input/Output
 * 		num	: length of sigvalues buffer (in), # of values returned in the buffer (out). 
 * Outputs: 
 *		sigvalues: list of signal values
 * 		Return value: 0 if no error.
 */
DLL_EXPORT long NIRT_ProbeSignals(long *sigindices, long numsigs, double *value, long* num);

/*
 * NIRT_SetParameter
 * Called either in scheduler loop or a background loop. 
 * Subject to change depending upon evolution of a Transaction mechanism.
 * 	
 * Inputs:
 *		index: index of the parameter (as specified in Parameter Information)
 *		subindex: index in the flattened array, if parameter is n-D array
 *		val: Value to set the parameter to
 * Return value: 0 if no error
 */
DLL_EXPORT long NIRT_SetParameter(long index, long subindex, double val);

/*
 * NIRT_GetParameter
 * Get the current value of a parameter.
 * 
 * Inputs: 
 *		index: index of the parameter
 *		subindex: index in the flattened array if parameter is n-D array
 * Outputs:
 *		val: value of the parameter 
 * Return value: 0 if no error
 */
DLL_EXPORT long NIRT_GetParameter(long index, long subindex, double* val);

/*
 * NIRT_ModelError
 * Check for any simulation errors. 
 * 
 * Input/Output:
 *		msglen: length of input string(in), length of output string (out) 
 * Output: 
 *		errmsg: string containing error message (if any occurred). 
 * Return value: 0 if no error
 */
DLL_EXPORT long NIRT_ModelError(char* errmsg, long* msglen);

/* 
 * NIRT_TaskRunTimeInfo
 * called in background loop. Returns number of overruns of tasks in the simulation 
 * 
 * overloading this function to set the HALT ON TASK OVERRUN flag
 * halt = 1: donot halt, 2: halt.
 *
 */
DLL_EXPORT long NIRT_TaskRunTimeInfo(long halt, long* overruns, long *numtasks);

/*
 * Functions for getting Model Information:
 * These functions are not necessary for the model to run and used to provide a better user experience.
 * 
 * The following functions may be called either in initialization phase, 
 * or from a background process while the simulation is running.
 * 
 * To refer to a parameter or signal in the model, 3 unique entities are used:
 * 1. Name (full_pathname for parameters or Path_to_SourceBlock:PortNumber for signals)
 * 2. ID
 * 3. A 32-bit integer index
 * 
 * Name and ID are both character arrays and may be identical. The ID is not seen by the user and 
 * may include some coded information to make access faster.
 * 
 * An index is an integer that is used as a key into tables for contant time access in RT.
 * Index may have any value > 0. With -1 being the special invalid index.
 * 
 * On RT, the parameter/signal information is extracted from the DLL using their corresponding IDs.
 * The function should return in addition to datatype/dimension etc information, the index for the ID.
 * This index is used subsequently when probing a signal value or setting value of a parameter.
 * 
 * The list of IDs may be either read in from a file generated when building the DLL or another 
 * function may be exported to generate the list.
 */

/* 
 * NIRT_GetBuildInfo
 * Get the DLL versioning etc information.
 * 
 * Output:
 *		detail: String containing model build information.
 *		len   : length of the string.
 * Return value:	1 if RTW (Mathworks) DLL, 
 *					2 if Autocode (MatrixX) DLL
 * Other values are currently unused.
 */
DLL_EXPORT long NIRT_GetBuildInfo(char* detail, long* len);

/*
 * NIRT_GetModelSpec
 * Get the model information without initializing.
 *
 * Outputs: 
 *		name: name of the model
 *		namelen: length of name 
 *		baseStep: base time step of the model
 *		NumInports: # of external inputs
 *		NumOutports: # of external outputs
 *		NumTasks: # of Tasks (including the base rate task)
 * Return value: 0 if no error.  
 */
DLL_EXPORT long NIRT_GetModelSpec(char* name, long *namelen, double *baseTimeStep, long *outNumInports, 
								  long *outNumOutports, long *numtasks);

/*
 * NIRT_GetParameterIndices
 * Returns the array populated with list of all valid parameters indices in the model.
 */
DLL_EXPORT long NIRT_GetParameterIndices(long* indices, long* len);

/*
 * NIRT_GetParameterSpec
 * (subject to change) 
 * If index == -1, 
 * Lookup parameter by the ID.
 * If ID_len==0 || ID==null, return number of parameters in model.
 *      Else lookup parameter by index, and return the information. 
 *
 * Input/Output
 *		index: index of the parameter(in/out)
 *		ID: ID of parameter to be looked up (in), ID of parameter at index (out)
 *		ID_len: length of input ID (in), length of ID (out)
 *		pnlen: length of buffer paramname(in), length of the returned paramname (out)
 *		numdim: length of buffer dimension(in), length of output dimension (out) 
 *Outputs: 
 *		paramname: Name of the parameter
 *		datatype: datatype of the parameter (currently assuming 0: double, .. )
 *		dimension: array of dimensions 
 * Return value: 0 if no error. (if paramidx == -1, number of tunable parameters)
 */
DLL_EXPORT long NIRT_GetParameterSpec(long* paramidx, char* ID, long* ID_len, char* paramname, long *pnlen, 
									  long *dattype, long* dims, long* numdim);


/*
 * NIRT_GetSignalSpec
 * (functionality is same as for parameter case)
 * Input/Output
 *		index: index of the signal (in/out) 
 * 		ID: ID of signal to be looked up (in), ID of signal at index (out)
 *		ID_len: length of input ID(in), length of ID (out)
 *		bnlen: length of buffer blkname (in), length of output blkname (out)
 *		snlen: length of buffer signame (in), length of output signame (out)
 * Outputs:  
 *		blkname: Name of the source block for the signal
 *		portnum: port number of the block that is the source of the signal (0 indexed)
 *		signame: Name of the signal
 *		datatype: same as with parameters. Currently assuming all data to be double. 
 *		dims: pre-allocated array of the signal dimensions
 *		numdim: should always be 2
 * Return value: 0 if no error. (if sigidx == -1, number of signals)
 */
DLL_EXPORT long NIRT_GetSignalSpec(long* sigidx, char* ID, long* ID_len, char* blkname, long* bnlen, long *portnum, 
								   char* signame, long* snlen, long *datatype, long* dims, long* numdim);

/*
 * NIRT_GetTaskSpec
 *
 * Inputs: 
 *		index: index of the task
 * Outputs: 
 *		tid: task id of the task. Note that it may be different from the index of task in task list.
 *		tstep: time step for this task
 *		offset: 
 * Return value: 0 if no error. (if index == -1, return number of tasks in the model) 
 */
DLL_EXPORT long NIRT_GetTaskSpec(long index, long* tid, double *tstep, double *offset);


/*
 * NIRT_GetExtIOSpec
 *
 * Inputs: 
 *		index: index of the task
 * Outputs: 
 *		idx:  idx of the IO.
 *		name: Name of the IO block
 *		tid: task id
 *		type: EXT_IN or EXT_OUT
 *		dimX: size of 0th dimension
 *		dimY: size of 1th dimension
 * Return value: 0 if no error. (if index == -1, return number of tasks in the model) 
 */
DLL_EXPORT long NIRT_GetExtIOSpec(long index, long *idx, char* name, long* tid, long *type, long *dims, long* numdims);

}

#endif
