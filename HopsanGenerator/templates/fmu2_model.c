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

#include "fmu2_model.h"
#include "fmu_hopsan.h"

void forward_message(const char* message, const char* type, void* userState)
{
    component_ptr_t comp = (component_ptr_t)userState;
    if (comp == NULL) {
        return;
    }

    if (comp->loggingOn == fmi2False) {
        return;
    }

    fmi2Status status = fmi2OK;
    if (strcmp(type, "warning") == 0)
    {
        status = fmi2Warning;
    }
    else if (strcmp(type, "error") == 0)
    {
        status = fmi2Error;
    }
    else if (strcmp(type, "fatal") == 0)
    {
        status = fmi2Fatal;
    }
    else if (strcmp(type, "debug") == 0)
    {
        //! @todo Make it possible to choose debug logs or not
        status = fmi2OK;
    }
    comp->functions->logger(comp->functions->componentEnvironment, comp->instanceName, status, type, message);
}

void get_all_hopsan_messages(component_ptr_t comp)
{
    while (hopsan_has_message() > 0)
    {
        hopsan_get_message(forward_message, (void*)comp);
    }
}

/* Model calculation functions */
static int calc_initialize(component_ptr_t comp)
{
    int initOK;
    double tStop;

    tStop = comp->tStart + 1.0; // Use a dummy stop value if one is not given
    if (comp->StopTimeDefined)
    {
        tStop = comp->tStop;
    }

    initOK = hopsan_initialize(comp->tStart, tStop);
    if (initOK)
    {
        return fmi2OK;
    }
    else
    {
        get_all_hopsan_messages(comp);
        return fmi2Error;
    }
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
    return 0;
}


/* FMI 2.0 Common Functions */
const char* fmi_get_version()
{
	return FMI_VERSION;
}

fmi2Status fmi_set_debug_logging(fmi2Component c, fmi2Boolean loggingOn)
{
	component_ptr_t comp = (fmi2Component)c;
	if (comp == NULL) {
		return fmi2Fatal;
	} else {
		comp->loggingOn = loggingOn;
		return fmi2OK;
	}
}

fmi2Status fmi_get_real(fmi2Component c, const fmi2ValueReference vr[], size_t nvr, fmi2Real value[])
{
	component_ptr_t comp = (fmi2Component)c;
	if (comp == NULL) 
    {
		return fmi2Fatal;
	} 
    else 
    {
        size_t i;
        for(i=0; i<nvr; ++i)
        {
            value[i] = hopsan_get_real(vr[i]);
        }
		return fmi2OK;
	}
}

fmi2Status fmi_get_integer(fmi2Component c, const fmi2ValueReference vr[], size_t nvr, fmi2Integer value[])
{
	component_ptr_t comp = (fmi2Component)c;
	if (comp == NULL) 
    {
		return fmi2Fatal;
	} 
    else 
    {
        size_t i;
        for(i=0; i<nvr; ++i)
        {
            value[i] = hopsan_get_integer(vr[i]);
        }
		return fmi2OK;
	}
}

fmi2Status fmi_get_boolean(fmi2Component c, const fmi2ValueReference vr[], size_t nvr, fmi2Boolean value[])
{
	component_ptr_t comp = (fmi2Component)c;
	if (comp == NULL) 
    {
		return fmi2Fatal;
	} 
    else 
    {
        size_t i;
        for(i=0; i<nvr; ++i)
        {
            value[i] = hopsan_get_boolean(vr[i]);
        }
		return fmi2OK;
	}
}

fmi2Status fmi_get_string(fmi2Component c, const fmi2ValueReference vr[], size_t nvr, fmi2String  value[])
{
	component_ptr_t comp = (fmi2Component)c;
	if (comp == NULL) 
    {
		return fmi2Fatal;
	} 
    else 
    {
        size_t i;
        for(i=0; i<nvr; ++i)
        {
            value[i] = hopsan_get_string(vr[i]);
        }
		return fmi2OK;
	}
}

fmi2Status fmi_set_real(fmi2Component c, const fmi2ValueReference vr[], size_t nvr, const fmi2Real value[])
{
	component_ptr_t comp = (fmi2Component)c;
	if (comp == NULL) 
    {
		return fmi2Fatal;
	} 
    else 
    {
        size_t i;
        for(i=0; i<nvr; ++i)
        {
            hopsan_set_real(vr[i], value[i]);
        }
		return fmi2OK;
	}
}

fmi2Status fmi_set_integer(fmi2Component c, const fmi2ValueReference vr[], size_t nvr, const fmi2Integer value[])
{
	component_ptr_t comp = (fmi2Component)c;
	if (comp == NULL) 
    {
		return fmi2Fatal;
	} 
    else 
    {
        size_t i;
        for(i=0; i<nvr; ++i)
        {
            hopsan_set_integer(vr[i], value[i]);
        }
		return fmi2OK;
	}
}

fmi2Status fmi_set_boolean(fmi2Component c, const fmi2ValueReference vr[], size_t nvr, const fmi2Boolean value[])
{
	component_ptr_t comp = (fmi2Component)c;
	if (comp == NULL) 
    {
		return fmi2Fatal;
	} 
    else 
    {
        size_t i;
        for(i=0; i<nvr; ++i)
        {
            hopsan_set_boolean(vr[i], value[i]);
        }
		return fmi2OK;
	}
}

fmi2Status fmi_set_string(fmi2Component c, const fmi2ValueReference vr[], size_t nvr, const fmi2String  value[])
{
	component_ptr_t comp = (fmi2Component)c;
	if (comp == NULL) 
    {
		return fmi2Fatal;
	} 
    else 
    {
        size_t i;
        for(i=0; i<nvr; ++i)
        {
            hopsan_set_string(vr[i], value[i]);
        }
		return fmi2OK;
	}
}

/* FMI 2.0 ME Functions */
const char* fmi_get_model_types_platform()
{
	return fmi2TypesPlatform;
}

/* static FILE* find_string(FILE* fp, char* str, int len) {

} */

fmi2Component fmi_instantiate(fmi2String instanceName, fmi2Type fmuType,
                              fmi2String fmuGUID, fmi2String fmuResourceLocation,
                              const fmi2CallbackFunctions *functions, fmi2Boolean visible,
                              fmi2Boolean loggingOn)
{
	component_ptr_t comp;
    int k, p, instantiateOK ;

	comp = (component_ptr_t)functions->allocateMemory(1, sizeof(component_t));
	if (comp == NULL) 
    {
		return NULL;
	} 
    else if (strcmp(fmuGUID, FMI_GUID) != 0) 
    {
		return NULL;
	} 
    else 
    {	
		sprintf(comp->instanceName, "%s", instanceName);
		sprintf(comp->GUID, "%s",fmuGUID);
        sprintf(comp->fmuResourceLocation, "%s",fmuResourceLocation);
        comp->functions		= functions;
		comp->loggingOn		= loggingOn;
        comp->visible		= visible;

		/* Set default values */
		for (k = 0; k < N_STATES;			k++) comp->states[k]			= 0.0;
		for (k = 0; k < N_STATES;			k++) comp->states_prev[k]		= 0.0; /* Used in CS only */
		for (k = 0; k < N_STATES;			k++) comp->states_nom[k]		= 1.0;
		for (k = 0; k < N_STATES;			k++) comp->states_der[k]		= 0.0;
		for (k = 0; k < N_EVENT_INDICATORS; k++) comp->event_indicators[k]	= 1e10;
		for (k = 0; k < N_REAL;				k++) comp->reals[k]				= 0.0;
		for (k = 0; k < N_INTEGER;			k++) comp->integers[k]			= 0;
		for (k = 0; k < N_BOOLEAN;			k++) comp->booleans[k]			= fmi2False;
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
				comp->output_real[k][p] = MAGIC_TEST_VALUE;
			}
		}
	
        instantiateOK = hopsan_instantiate(fmuResourceLocation);
        if (!instantiateOK)
        {
            get_all_hopsan_messages(comp);
            fmi_free_instance(comp);
            return NULL;
        }

		return comp;
	}
}

void fmi_free_instance(fmi2Component c)
{
	int i;
	component_ptr_t comp = (fmi2Component)c;
	for(i = 0; i < N_STRING; i++) {
		comp->functions->freeMemory((void*)(comp->strings[i]));
		comp->strings[i] = 0;
	}
	comp->functions->freeMemory(c);
}

fmi2Status fmi_setup_experiment(fmi2Component c, fmi2Boolean toleranceDefined,
                               fmi2Real tolerance, fmi2Real startTime,
                               fmi2Boolean stopTimeDefined,
                               fmi2Real stopTime)
{
    component_ptr_t comp = (fmi2Component)c;

    if (comp == NULL) 
    {
        return fmi2Fatal;
    } 
    else 
    {
        comp->toleranceControlled = toleranceDefined;
        comp->relativeTolerance = tolerance;

        comp->tStart = startTime;
        comp->StopTimeDefined = stopTimeDefined;
        comp->tStop = stopTime;

        return fmi2OK;
    }
}

fmi2Status fmi_enter_initialization_mode(fmi2Component c)
{
    if (c == NULL) 
    {
        return fmi2Fatal;
    } 
    else 
    {
        return calc_initialize(c);
    }
}

fmi2Status fmi_exit_initialization_mode(fmi2Component c)
{
    return fmi2OK;
}

fmi2Status fmi_enter_event_mode(fmi2Component c)
{
    return fmi2OK;
}

fmi2Status fmi_new_discrete_states(fmi2Component c, fmi2EventInfo* eventInfo)
{
	component_ptr_t comp = (fmi2Component)c;
	if (comp == NULL) 
    {
		return fmi2Fatal;
	} 
    else 
    {
		calc_event_update(comp);

		*eventInfo = comp->eventInfo;
		return fmi2OK;
	}
}

fmi2Status fmi_enter_continuous_time_mode(fmi2Component c)
{
    return fmi2OK;
}

fmi2Status fmi_set_time(fmi2Component c, fmi2Real fmitime)
{
	component_ptr_t comp = (fmi2Component)c;
	if (comp == NULL) 
    {
		return fmi2Fatal;
	} 
    else 
    {
		comp->fmitime = fmitime;
		return fmi2OK;
	}
}

fmi2Status fmi_set_continuous_states(fmi2Component c, const fmi2Real x[], size_t nx)
{
	component_ptr_t comp = (fmi2Component)c;
	if (comp == NULL) 
    {
		return fmi2Fatal;
	} 
    else 
    {
		size_t k;
		for (k = 0; k < nx; k++) 
        {
			comp->states[k] = x[k];
		}
		return fmi2OK;
	}
}

fmi2Status fmi_completed_integrator_step(fmi2Component c,
  fmi2Boolean noSetFMUStatePriorToCurrentPoint,
  fmi2Boolean* enterEventMode, fmi2Boolean* terminateSimulation)
{
	component_ptr_t comp = (fmi2Component)c;
	if (comp == NULL) 
    {
		return fmi2Fatal;
	} 
    else 
    {
		*enterEventMode = fmi2False;
		return fmi2OK;
	}
}

fmi2Status fmi_get_derivatives(fmi2Component c, fmi2Real derivatives[] , size_t nx)
{
	component_ptr_t comp = (fmi2Component)c;
	if (comp == NULL) 
    {
		return fmi2Fatal;
	} 
    else 
    {
		size_t k;

		calc_get_derivatives(comp);

		for (k = 0; k < nx; k++) {
			derivatives[k] = comp->states_der[k];
		}
		return fmi2OK;
	}
}

fmi2Status fmi_get_event_indicators(fmi2Component c, fmi2Real eventIndicators[], size_t ni)
{
	component_ptr_t comp = (fmi2Component)c;
	if (comp == NULL) 
    {
		return fmi2Fatal;
	} 
    else 
    {
		size_t k;

		calc_get_event_indicators(comp);

		for (k = 0; k < ni; k++) 
        {
			eventIndicators[k] = comp->event_indicators[k];
		}
		return fmi2OK;
	}
}

fmi2Status fmi_get_continuous_states(fmi2Component c, fmi2Real states[], size_t nx)
{
	component_ptr_t comp = (fmi2Component)c;
	if (comp == NULL) 
    {
		return fmi2Fatal;
	} 
    else 
    {
		size_t k;

		for (k = 0; k < nx; k++) 
        {
			states[k] = comp->states[k];
		}
		return fmi2OK;
	}
}

fmi2Status fmi_get_nominals_of_continuousstates(fmi2Component c, fmi2Real x_nominal[], size_t nx)
{
	component_ptr_t comp = (fmi2Component)c;
	if (comp == NULL) 
    {
		return fmi2Fatal;
	} 
    else 
    {
		size_t k;
		for (k = 0; k < nx; k++) 
        {
			x_nominal[k] = comp->states_nom[k];
		}
		return fmi2OK;
	}
}

fmi2Status fmi_terminate(fmi2Component c)
{
	component_ptr_t comp = (fmi2Component)c;
	if (comp == NULL) 
    {
		return fmi2Fatal;
	} 
    else 
    {
        hopsan_finalize();
        get_all_hopsan_messages(comp);
		return fmi2OK;
	}
}

/* FMI 2.0 CS Functions */
const char* fmi_get_types_platform()
{
	return fmi2TypesPlatform;
}

fmi2Status fmi_reset(fmi2Component c)
{
	return fmi2OK;
}

fmi2Status fmi_set_real_input_derivatives(fmi2Component c, const fmi2ValueReference vr[], size_t nvr, const fmi2Integer order[], const fmi2Real value[])
{
	component_ptr_t comp	= (fmi2Component)c;
	size_t k;

	for (k = 0; k < nvr; k++) {
		comp->input_real[vr[k]][order[k]] = value[k];
		if (value[k] != MAGIC_TEST_VALUE) 
        {/* Tests that the value is set to MAGIC_TEST_VALUE */
			return fmi2Fatal;
		}
	}

	return fmi2OK;
}

fmi2Status fmi_get_real_output_derivatives(fmi2Component c, const fmi2ValueReference vr[], size_t nvr, const fmi2Integer order[], fmi2Real value[])
{
	component_ptr_t comp	= (fmi2Component)c;
	size_t k;

	for (k = 0; k < nvr; k++) 
    {
		value[k] = comp->output_real[vr[k]][order[k]];
	}

	return fmi2OK;
}

fmi2Status fmi_cancel_step(fmi2Component c)
{
	return fmi2OK;
}

fmi2Status fmi_do_step(fmi2Component c, fmi2Real currentCommunicationPoint, fmi2Real communicationStepSize, fmi2Boolean newStep)
{
	component_ptr_t comp	= (fmi2Component)c;

	if (comp == NULL) 
    {
		return fmi2Fatal;
	} 
    else 
    {
		fmi2Real tstart = currentCommunicationPoint;
		fmi2Real tcur;
		fmi2Real tend = currentCommunicationPoint + communicationStepSize;
		fmi2Real hcur; 
		fmi2Real hdef = 0.01;	/* Default time step length */
		fmi2Real z_cur[N_EVENT_INDICATORS];
		fmi2Real z_pre[N_EVENT_INDICATORS];
		fmi2Real states[N_STATES];
		fmi2Real states_der[N_STATES];
		fmi2EventInfo eventInfo;
		fmi2Boolean callEventUpdate;
		fmi2Boolean terminateSimulation;
		fmi2Status fmi2Status;	
		size_t k;
		size_t counter = 0;

		fmi_get_continuous_states(comp, states, N_STATES);
		fmi_get_event_indicators(comp, z_pre, N_EVENT_INDICATORS);

		tcur = tstart;
		hcur = hdef;
		callEventUpdate = fmi2False;
		eventInfo = comp->eventInfo;

        hopsan_simulate(tend);
        //! @todo we should check if the step was completed OK, but the only way I can find is to check wasAborted, dont know if we want to do that at every time step
            
        fmi_set_time(comp, tend);
        
		for (k = 0; k < N_STATES; k++) 
        { /* Update states */
			comp->reals[k] = comp->states[k];
		}
		return fmi2OK;
	}
}

fmi2Status fmi_get_status(fmi2Component c, const fmi2StatusKind s, fmi2Status*  value)
{
	switch (s) 
    {
		case fmi2DoStepStatus:
			/* Return fmiPending if we are waiting. Otherwise the result from fmiDoStep */
			*value = fmi2OK;
			return fmi2OK;
		default: /* Not defined for status for this function */
			return fmi2Discard;
	}
}

fmi2Status fmi_get_real_status(fmi2Component c, const fmi2StatusKind s, fmi2Real*    value)
{
	switch (s) 
    {
		case fmi2LastSuccessfulTime:
			/* Return fmiPending if we are waiting. Otherwise return end time for last call to fmiDoStep */
			*value = 0.01;
			return fmi2OK;
		default: /* Not defined for status for this function */
			return fmi2Discard;
	}
}

fmi2Status fmi_get_integer_status(fmi2Component c, const fmi2StatusKind s, fmi2Integer* value)
{
	switch (s) 
    {
		default: /* Not defined for status for this function */
			return fmi2Discard;
	}
}

fmi2Status fmi_get_boolean_status(fmi2Component c, const fmi2StatusKind s, fmi2Boolean* value)
{
	switch (s) 
    {
		default: /* Not defined for status for this function */
			return fmi2Discard;
	}
}

fmi2Status fmi_get_string_status(fmi2Component c, const fmi2StatusKind s, fmi2String*  value)
{
	switch (s) 
    {
		case fmi2PendingStatus:
			*value = "Did fmi2DoStep really return with fmi2Pending? Then its time to implement this function";
			return fmi2Discard;
		default: /* Not defined for status for this function */
			return fmi2Discard;
	}
}
