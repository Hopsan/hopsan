/*
Copyright (C) 2012 Modelon AB

This program is free software: you can redistribute it and/or modify
it under the terms of the BSD style license.


This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
FMILIB_License.txt file for more details.

You should have received a copy of the FMILIB_License.txt file
along with this program. If not, contact Modelon AB <http://www.modelon.com>.
*/

#include <stdio.h>
#include <string.h>

#include "fmu1_model.h"
#include "fmu_hopsan.h"

/* Model calculation functions */
static int calc_initialize(component_ptr_t comp)
{
    hopsan_initialize();

	return 0;
}

static int calc_get_derivatives(component_ptr_t comp)
{
	return 0;
}

static int calc_get_event_indicators(component_ptr_t comp)
{	
	return 0;
}

static int calc_event_update(component_ptr_t comp)
{	
    return 1; /* Should not call the event update */

}


/* FMI 1.0 Common Functions */
const char* fmi_get_version()
{
	return FMI_VERSION;
}

fmiStatus fmi_set_debug_logging(fmiComponent c, fmiBoolean loggingOn)
{
	component_ptr_t comp = (fmiComponent)c;
	if (comp == NULL) {
		return fmiFatal;
	} else {
		comp->loggingOn = loggingOn;
		return fmiOK;
	}
}

fmiStatus fmi_get_real(fmiComponent c, const fmiValueReference vr[], size_t nvr, fmiReal value[])
{
	component_ptr_t comp = (fmiComponent)c;
	if (comp == NULL) 
    {
		return fmiFatal;
	} 
    else 
    {
        value[0] = hopsan_get_real(vr[0]);
		return fmiOK;
	}
}

fmiStatus fmi_get_integer(fmiComponent c, const fmiValueReference vr[], size_t nvr, fmiInteger value[])
{
	component_ptr_t comp = (fmiComponent)c;
	if (comp == NULL) 
    {
		return fmiFatal;
	} 
    else 
    {
		value[0] = hopsan_get_integer(vr[0]);
		return fmiOK;
	}
}

fmiStatus fmi_get_boolean(fmiComponent c, const fmiValueReference vr[], size_t nvr, fmiBoolean value[])
{
	component_ptr_t comp = (fmiComponent)c;
	if (comp == NULL) 
    {
		return fmiFatal;
	} 
    else 
    {
		value[0] = hopsan_get_boolean(vr[0]);
		return fmiOK;
	}
}

fmiStatus fmi_get_string(fmiComponent c, const fmiValueReference vr[], size_t nvr, fmiString  value[])
{
	component_ptr_t comp = (fmiComponent)c;
	if (comp == NULL) 
    {
		return fmiFatal;
	} 
    else 
    {
		value[0] = hopsan_get_string(vr[0]);
		return fmiOK;
	}
}

fmiStatus fmi_set_real(fmiComponent c, const fmiValueReference vr[], size_t nvr, const fmiReal value[])
{
	component_ptr_t comp = (fmiComponent)c;
	if (comp == NULL) 
    {
		return fmiFatal;
	} 
    else 
    {
		hopsan_set_real(vr[0], value[0]);
		return fmiOK;
	}
}

fmiStatus fmi_set_integer(fmiComponent c, const fmiValueReference vr[], size_t nvr, const fmiInteger value[])
{
	component_ptr_t comp = (fmiComponent)c;
	if (comp == NULL) 
    {
		return fmiFatal;
	} 
    else 
    {
		hopsan_set_integer(vr[0], value[0]);
		return fmiOK;
	}
}

fmiStatus fmi_set_boolean(fmiComponent c, const fmiValueReference vr[], size_t nvr, const fmiBoolean value[])
{
	component_ptr_t comp = (fmiComponent)c;
	if (comp == NULL) 
    {
		return fmiFatal;
	} 
    else 
    {
		hopsan_set_boolean(vr[0], value[0]);
		return fmiOK;
	}
}

fmiStatus fmi_set_string(fmiComponent c, const fmiValueReference vr[], size_t nvr, const fmiString  value[])
{
	component_ptr_t comp = (fmiComponent)c;
	if (comp == NULL) 
    {
		return fmiFatal;
	} 
    else 
    {
		hopsan_set_string(vr[0], value[0]);
        return fmiOK;
	}
}

/* FMI 1.0 ME Functions */
const char* fmi_get_model_types_platform()
{
	return FMI_PLATFORM_TYPE;
}



/* static FILE* find_string(FILE* fp, char* str, int len) {

} */

fmiComponent fmi_instantiate_model(fmiString instanceName, fmiString GUID, fmiCallbackFunctions functions, fmiBoolean loggingOn)
{
    hopsan_instantiate();

	component_ptr_t comp;
	int k, p;

	comp = (component_ptr_t)functions.allocateMemory(1, sizeof(component_t));
	if (comp == NULL) 
    {
		return NULL;
	} 
    else if (strcmp(GUID, FMI_GUID) != 0) 
    {
		return NULL;
	} 
    else 
    {	
		sprintf(comp->instanceName, "%s", instanceName);
		sprintf(comp->GUID, "%s",GUID);
		comp->functions		= functions;
		comp->loggingOn		= loggingOn;

		comp->callEventUpdate = fmiFalse;

		/* Set default values */
		for (k = 0; k < N_STATES;			k++) comp->states[k]			= 0.0;
		for (k = 0; k < N_STATES;			k++) comp->states_prev[k]		= 0.0; /* Used in CS only */
		for (k = 0; k < N_STATES;			k++) comp->states_nom[k]		= 1.0;
		for (k = 0; k < N_STATES;			k++) comp->states_vr[k]			= k;
		for (k = 0; k < N_STATES;			k++) comp->states_der[k]		= 0.0;
		for (k = 0; k < N_EVENT_INDICATORS; k++) comp->event_indicators[k]	= 1e10;
		for (k = 0; k < N_REAL;				k++) comp->reals[k]				= 0.0;
		for (k = 0; k < N_INTEGER;			k++) comp->integers[k]			= 0;
		for (k = 0; k < N_BOOLEAN;			k++) comp->booleans[k]			= fmiFalse;
		for (k = 0; k < N_STRING;			k++) comp->strings[k]			= NULL;

		/* Used in CS only */
		for (k = 0; k < N_INPUT_REAL; k++) {
			for (p = 0; p < N_INPUT_REAL_MAX_ORDER + 1; p++) {
				comp->input_real[k][p] = 0.0;
			}
		}

		/* Used in CS only */
		for (k = 0; k < N_OUTPUT_REAL; k++) {
			for (p = 0; p < N_OUTPUT_REAL_MAX_ORDER + 1; p++) {
                comp->output_real[k][p] = 0.0;
			}
		}

		return comp;
	}
}

void fmi_free_model_instance(fmiComponent c)
{
	int i;
	component_ptr_t comp = (fmiComponent)c;
	for(i = 0; i < N_STRING; i++) {
		comp->functions.freeMemory((void*)(comp->strings[i]));
		comp->strings[i] = 0;
	}
	comp->functions.freeMemory(c);
}

fmiStatus fmi_set_time(fmiComponent c, fmiReal fmitime)
{
	component_ptr_t comp = (fmiComponent)c;
	if (comp == NULL) {
		return fmiFatal;
	} else {
		comp->fmitime = fmitime;
		return fmiOK;
	}
}

fmiStatus fmi_set_continuous_states(fmiComponent c, const fmiReal x[], size_t nx)
{
	component_ptr_t comp = (fmiComponent)c;
	if (comp == NULL) 
    {
		return fmiFatal;
	} 
    else 
    {
		size_t k;
		for (k = 0; k < nx; k++) 
        {
			comp->states[k] = x[k];
		}
		return fmiOK;
	}
}

fmiStatus fmi_completed_integrator_step(fmiComponent c, fmiBoolean* callEventUpdate)
{
	component_ptr_t comp = (fmiComponent)c;
	if (comp == NULL) 
    {
		return fmiFatal;
	} 
    else 
    {
		*callEventUpdate = comp->callEventUpdate;
		return fmiOK;
	}
}

fmiStatus fmi_initialize(fmiComponent c, fmiBoolean toleranceControlled, fmiReal relativeTolerance, fmiEventInfo* eventInfo)
{
	component_ptr_t comp = (fmiComponent)c;

	if (comp == NULL) {
		return fmiFatal;
	} else {
		comp->eventInfo.iterationConverged			= fmiFalse;
		comp->eventInfo.stateValueReferencesChanged = fmiFalse;
		comp->eventInfo.stateValuesChanged			= fmiFalse;
		comp->eventInfo.terminateSimulation			= fmiFalse;
		comp->eventInfo.upcomingTimeEvent			= fmiFalse;
		comp->eventInfo.nextEventTime				= -0.0;

		comp->toleranceControlled = toleranceControlled;
		comp->relativeTolerance = relativeTolerance;
		
		calc_initialize(comp);

		*eventInfo = comp->eventInfo;

		return fmiOK;
	}
}

fmiStatus fmi_get_derivatives(fmiComponent c, fmiReal derivatives[] , size_t nx)
{
	component_ptr_t comp = (fmiComponent)c;
	if (comp == NULL) {
		return fmiFatal;
	} else {
		size_t k;

		calc_get_derivatives(comp);

		for (k = 0; k < nx; k++) {
			derivatives[k] = comp->states_der[k];
		}
		return fmiOK;
	}
}

fmiStatus fmi_get_event_indicators(fmiComponent c, fmiReal eventIndicators[], size_t ni)
{
	component_ptr_t comp = (fmiComponent)c;
	if (comp == NULL) {
		return fmiFatal;
	} else {
		size_t k;

		calc_get_event_indicators(comp);

		for (k = 0; k < ni; k++) {
			eventIndicators[k] = comp->event_indicators[k];
		}
		return fmiOK;
	}
}

fmiStatus fmi_event_update(fmiComponent c, fmiBoolean intermediateResults, fmiEventInfo* eventInfo)
{
	component_ptr_t comp = (fmiComponent)c;
	if (comp == NULL) 
    {
		return fmiFatal;
	} 
    else 
    {
		calc_event_update(comp);

		*eventInfo = comp->eventInfo;
		return fmiOK;
	}
}

fmiStatus fmi_get_continuous_states(fmiComponent c, fmiReal states[], size_t nx)
{
	component_ptr_t comp = (fmiComponent)c;
	if (comp == NULL) 
    {
		return fmiFatal;
	} 
    else 
    {
		size_t k;

		for (k = 0; k < nx; k++) 
        {
			states[k] = comp->states[k];
		}
		return fmiOK;
	}
}

fmiStatus fmi_get_nominal_continuousstates(fmiComponent c, fmiReal x_nominal[], size_t nx)
{
	component_ptr_t comp = (fmiComponent)c;
	if (comp == NULL) 
    {
		return fmiFatal;
	} 
    else 
    {
		size_t k;
		for (k = 0; k < nx; k++) 
        {
			x_nominal[k] = comp->states_nom[k];
		}
		return fmiOK;
	}
}

fmiStatus fmi_get_state_value_references(fmiComponent c, fmiValueReference vrx[], size_t nx)
{
	component_ptr_t comp = (fmiComponent)c;
	if (comp == NULL) {
		return fmiFatal;
	} else {
		size_t k;
		for (k = 0; k < nx; k++) {
			vrx[k] = comp->states_vr[k];
		}
		return fmiOK;
	}
}

fmiStatus fmi_terminate(fmiComponent c)
{
	component_ptr_t comp = (fmiComponent)c;
	if (comp == NULL) {
		return fmiFatal;
	} else {
		return fmiOK;
	}
}

/* FMI 1.0 CS Functions */
const char* fmi_get_types_platform()
{
	return FMI_PLATFORM_TYPE;
}

fmiComponent fmi_instantiate_slave(fmiString instanceName, fmiString fmuGUID, fmiString fmuLocation, fmiString mimeType, fmiReal timeout, fmiBoolean visible, fmiBoolean interactive, fmiCallbackFunctions functions, fmiBoolean loggingOn)
{
	component_ptr_t comp;

	comp = fmi_instantiate_model(instanceName, fmuGUID, functions, loggingOn);
	if (comp == NULL) {
		return NULL;
	} else if (strcmp(fmuGUID, FMI_GUID) != 0) {
		return NULL;
	} else {	
		sprintf(comp->fmuLocation, "%s",fmuLocation);
		sprintf(comp->mimeType, "%s",mimeType);
		comp->timeout		= timeout;
		comp->visible		= visible;
		comp->interactive	= interactive;
		return comp;
	}
}

fmiStatus fmi_initialize_slave(fmiComponent c, fmiReal tStart, fmiBoolean StopTimeDefined, fmiReal tStop)
{
	component_ptr_t comp	= (fmiComponent)c;
	fmiReal relativeTolerance;
	fmiEventInfo eventInfo;
	fmiBoolean toleranceControlled;


	comp->tStart			= tStart;
	comp->StopTimeDefined	= StopTimeDefined;
	comp->tStop				= tStop;

	toleranceControlled = fmiTrue;
	relativeTolerance = 1e-4;

	return fmi_initialize((fmiComponent)comp, toleranceControlled, relativeTolerance, &eventInfo);
}

fmiStatus fmi_terminate_slave(fmiComponent c)
{
	return fmi_terminate(c);
}

fmiStatus fmi_reset_slave(fmiComponent c)
{
	return fmiOK;
}

void fmi_free_slave_instance(fmiComponent c)
{
	fmi_free_model_instance(c);
}

fmiStatus fmi_set_real_input_derivatives(fmiComponent c, const fmiValueReference vr[], size_t nvr, const fmiInteger order[], const fmiReal value[])
{

	component_ptr_t comp	= (fmiComponent)c;
	size_t k;

	for (k = 0; k < nvr; k++) {
		comp->input_real[vr[k]][order[k]] = value[k];
        if (value[k] != 0.0) {/* Tests that the value is set to MAGIC_TEST_VALUE */
			return fmiFatal;
		}
	}

	return fmiOK;
}

fmiStatus fmi_get_real_output_derivatives(fmiComponent c, const fmiValueReference vr[], size_t nvr, const fmiInteger order[], fmiReal value[])
{
	component_ptr_t comp	= (fmiComponent)c;
	size_t k;

	for (k = 0; k < nvr; k++) {
		value[k] = comp->output_real[vr[k]][order[k]];
	}

	return fmiOK;
}

fmiStatus fmi_cancel_step(fmiComponent c)
{
	return fmiOK;
}

fmiStatus fmi_do_step(fmiComponent c, fmiReal currentCommunicationPoint, fmiReal communicationStepSize, fmiBoolean newStep)
{
	component_ptr_t comp	= (fmiComponent)c;

	if (comp == NULL) 
    {
		return fmiFatal;
	} 
    else 
    {
		fmiReal tstart = currentCommunicationPoint;
		fmiReal tcur;
		fmiReal tend = currentCommunicationPoint + communicationStepSize;
		fmiReal hcur; 
		fmiReal hdef = 0.01;	/* Default time step length */
		fmiReal z_cur[N_EVENT_INDICATORS];
		fmiReal z_pre[N_EVENT_INDICATORS];
		fmiReal states[N_STATES];
		fmiReal states_der[N_STATES];
		fmiEventInfo eventInfo;
		fmiBoolean callEventUpdate;
		fmiBoolean intermediateResults = fmiFalse;
		fmiStatus fmistatus;	
		size_t k;
		size_t counter = 0;

		fmi_get_continuous_states(comp, states, N_STATES);
		fmi_get_event_indicators(comp, z_pre, N_EVENT_INDICATORS);

		tcur = tstart;
		hcur = hdef;
		callEventUpdate = fmiFalse;
		eventInfo = comp->eventInfo;
        
        hopsan_simulate(tend);

        fmi_set_time(comp, tcur);

		for (k = 0; k < N_STATES; k++) { /* Update states */
			comp->reals[k] = comp->states[k];
		}
		return fmiOK;
	}
}

fmiStatus fmi_get_status(fmiComponent c, const fmiStatusKind s, fmiStatus*  value)
{
	switch (s) {
		case fmiDoStepStatus:
			/* Return fmiPending if we are waiting. Otherwise the result from fmiDoStep */
			*value = fmiOK;
			return fmiOK;
		default: /* Not defined for status for this function */
			return fmiDiscard;
	}
}

fmiStatus fmi_get_real_status(fmiComponent c, const fmiStatusKind s, fmiReal*    value)
{
	switch (s) {
		case fmiLastSuccessfulTime:
			/* Return fmiPending if we are waiting. Otherwise return end time for last call to fmiDoStep */
			*value = 0.01;
			return fmiOK;
		default: /* Not defined for status for this function */
			return fmiDiscard;
	}
}

fmiStatus fmi_get_integer_status(fmiComponent c, const fmiStatusKind s, fmiInteger* value)
{
	switch (s) {
		default: /* Not defined for status for this function */
			return fmiDiscard;
	}
}

fmiStatus fmi_get_boolean_status(fmiComponent c, const fmiStatusKind s, fmiBoolean* value)
{
	switch (s) {
		default: /* Not defined for status for this function */
			return fmiDiscard;
	}
}

fmiStatus fmi_get_string_status(fmiComponent c, const fmiStatusKind s, fmiString*  value)
{
	switch (s) {
		case fmiPendingStatus:
			*value = "Did fmiDoStep really return with fmiPending? Then its time to implement this function";
			return fmiDiscard;
		default: /* Not defined for status for this function */
			return fmiDiscard;
	}
}
