/*
Copyright (C) 2012 Modelon AB

This program is free software: you can redistribute it and/or modify
it under the terms of the BSD style license.

the Free Software Foundation, version 3 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
FMILIB_License.txt file for more details.

You should have received a copy of the FMILIB_License.txt file
along with this program. If not, contact Modelon AB <http://www.modelon.com>.
*/

#ifndef FMU2_MODEL_H_
#define FMU2_MODEL_H_
#include <FMI2/fmi2Functions.h>

#include "fmu2_model_defines.h"
#ifndef FMI2_Export
	#define FMI2_Export DllExport
#endif
typedef struct {
	/*************** FMI ME 2.0 ****************/
	fmi2Real					states			[N_STATES];
	fmi2Real					states_nom		[N_STATES];
	fmi2Real					states_der		[N_STATES];
	fmi2Real					event_indicators[N_EVENT_INDICATORS];
	fmi2Real					reals			[N_REAL];
	fmi2Integer				integers		[N_INTEGER];
	fmi2Boolean				booleans		[N_BOOLEAN];
	fmi2String				strings			[N_STRING];

	/* fmiInstantiateModel */
	fmi2Boolean				loggingOn;
	char					instanceName	[BUFFER];
	char					GUID			[BUFFER];
	const fmi2CallbackFunctions*	functions;

	/* fmiSetTime */
	fmi2Real					fmitime;

	/* fmiInitializeModel */
	fmi2Boolean				toleranceControlled;
	fmi2Real					relativeTolerance;
	fmi2EventInfo			eventInfo;

	/*************** FMI CS 2.0. Depends on the ME fields above and functions ****************/
	fmi2Real					states_prev		[N_STATES];

	/* fmiInstantiateSlave */
	char					fmuLocation		[BUFFER];
	fmi2Boolean				visible;

	/* fmiInitializeSlave */
	fmi2Real					tStart;
	fmi2Boolean				StopTimeDefined;
	fmi2Real					tStop;

	/* fmiSetRealInputDerivatives */
	fmi2Real					input_real		[N_INPUT_REAL][N_INPUT_REAL_MAX_ORDER + 1];

	/* fmiGetRealOutputDerivatives */
	fmi2Real					output_real		[N_OUTPUT_REAL][N_OUTPUT_REAL_MAX_ORDER + 1];

} component_t;

typedef component_t* component_ptr_t;

/* FMI 2.0 Common Functions */
const char*		fmi_get_version();

fmi2Status		fmi_set_debug_logging(
													fmi2Component c,
													fmi2Boolean loggingOn);

fmi2Component fmi_instantiate (
    fmi2String instanceName,
    fmi2Type fmuType,
    fmi2String fmuGUID,
    fmi2String fmuLocation,
    const fmi2CallbackFunctions* functions,
    fmi2Boolean visible,
    fmi2Boolean loggingOn);

void fmi_free_instance(
    fmi2Component c);

fmi2Status fmi_setup_experiment(fmi2Component c,
    fmi2Boolean toleranceDefined, fmi2Real tolerance,
    fmi2Real startTime, fmi2Boolean stopTimeDefined,
    fmi2Real stopTime);
fmi2Status		fmi_enter_initialization_mode(fmi2Component c);
fmi2Status		fmi_exit_initialization_mode(fmi2Component c);

fmi2Status		fmi_terminate(fmi2Component c);

fmi2Status		fmi_reset(
													fmi2Component c);


fmi2Status		fmi_get_real(			
													fmi2Component c,
													const fmi2ValueReference vr[],
													size_t nvr, fmi2Real value[]);

fmi2Status		fmi_get_integer(	
													fmi2Component c,
													const fmi2ValueReference vr[],
													size_t nvr,
													fmi2Integer value[]);
fmi2Status		fmi_get_boolean(
													fmi2Component c,
													const fmi2ValueReference vr[],
													size_t nvr,
													fmi2Boolean value[]);

fmi2Status		fmi_get_string(
													fmi2Component c,
													const fmi2ValueReference vr[],
													size_t nvr,
													fmi2String  value[]);

fmi2Status		fmi_set_real(
													fmi2Component c,
													const fmi2ValueReference vr[],
													size_t nvr,
													const fmi2Real value[]);
fmi2Status		fmi_set_integer(
													fmi2Component c,
													const fmi2ValueReference vr[],
													size_t nvr,
													const fmi2Integer value[]);

fmi2Status		fmi_set_boolean(
													fmi2Component c,
													const fmi2ValueReference vr[],
													size_t nvr,
													const fmi2Boolean value[]);

fmi2Status		fmi_set_string(
													fmi2Component c,
													const fmi2ValueReference vr[],
													size_t nvr,
													const fmi2String  value[]);

/* FMI 2.0 ME Functions */
const char*		fmi_get_model_types_platform();

fmi2Status		fmi_enter_event_mode(fmi2Component c);
fmi2Status		fmi_new_discrete_states(fmi2Component c, fmi2EventInfo* eventInfo);
fmi2Status		fmi_enter_continuous_time_mode(fmi2Component c);

fmi2Status		fmi_set_time(
													fmi2Component c,
													fmi2Real fmitime);

fmi2Status		fmi_set_continuous_states(
													fmi2Component c,
													const fmi2Real x[],
													size_t nx);

fmi2Status fmi_completed_integrator_step(
    fmi2Component c,
    fmi2Boolean noSetFMUStatePriorToCurrentPoint,
    fmi2Boolean* enterEventMode, fmi2Boolean* terminateSimulation);

fmi2Status		fmi_get_derivatives(
													fmi2Component c,
													fmi2Real derivatives[],
													size_t nx);

fmi2Status		fmi_get_event_indicators(
													fmi2Component c,
													fmi2Real eventIndicators[],
													size_t ni);

fmi2Status		fmi_get_continuous_states(
													fmi2Component c,
													fmi2Real states[],
													size_t nx);

fmi2Status		fmi_get_nominals_of_continuousstates(	
													fmi2Component c,
													fmi2Real x_nominal[],
													size_t nx);


/* FMI 2.0 CS Functions */
#ifdef fmi2Functions_h

const char*		fmi_get_types_platform();

void			fmi_free_slave_instance(
													fmi2Component c);

fmi2Status		fmi_set_real_input_derivatives(
													fmi2Component c,
													const fmi2ValueReference vr[],
													size_t nvr,
													const fmi2Integer order[],
													const fmi2Real value[]);

fmi2Status		fmi_get_real_output_derivatives(
													fmi2Component c,
													const fmi2ValueReference vr[],
													size_t nvr,
													const fmi2Integer order[],
													fmi2Real value[]);

fmi2Status		fmi_cancel_step(
													fmi2Component c);
fmi2Status		fmi_do_step(
													fmi2Component c,
													fmi2Real currentCommunicationPoint,
													fmi2Real communicationStepSize,
													fmi2Boolean newStep);

fmi2Status		fmi_get_status(
													fmi2Component c,
													const fmi2StatusKind s,
													fmi2Status*  value);

fmi2Status		fmi_get_real_status(
													fmi2Component c,
													const fmi2StatusKind s,
													fmi2Real*    value);

fmi2Status		fmi_get_integer_status(
													fmi2Component c,
													const fmi2StatusKind s,
													fmi2Integer* value);

fmi2Status		fmi_get_boolean_status(
													fmi2Component c,
													const fmi2StatusKind s,
													fmi2Boolean* value);

fmi2Status		fmi_get_string_status(
													fmi2Component c,
													const fmi2StatusKind s,
													fmi2String*  value);

#endif /* End of fmi2Functions_h */
#endif /* End of header FMU2_MODEL_H_ */
