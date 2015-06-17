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

#include <string.h>



#if __GNUC__ >= 4
    #pragma GCC visibility push(default)
#endif

/* Standard FMI 2.0 ME and CS types */
#include <FMI2/fmi2Functions.h>

#include "fmu_hopsan.h"
#include "fmu2_model.h"

/* #define MODEL_IDENTIFIER FMU_DUMMY_CS_MODEL_IDENTIFIER */

#include "fmu2_model.c"



/* FMI 2.0 Common Functions */
FMI2_Export const char* fmi2GetVersion()
{
	return fmi_get_version();
}

FMI2_Export fmi2Status fmi2SetDebugLogging(fmi2Component c, fmi2Boolean loggingOn, size_t n, const fmi2String cat[])
{
	return fmi_set_debug_logging(c, loggingOn);
}

FMI2_Export fmi2Component fmi2Instantiate(fmi2String instanceName,
  fmi2Type fmuType, fmi2String GUID, fmi2String location,
  const fmi2CallbackFunctions* functions, fmi2Boolean visible,
  fmi2Boolean loggingOn)
{
    return fmi_instantiate(instanceName, fmuType, GUID, location, functions,
                           visible, loggingOn);
}

FMI2_Export void fmi2FreeInstance(fmi2Component c)
{
	fmi_free_instance(c);
}

FMI2_Export fmi2Status fmi2SetupExperiment(fmi2Component c, 
    fmi2Boolean toleranceDefined, fmi2Real tolerance,
    fmi2Real startTime, fmi2Boolean stopTimeDefined,
    fmi2Real stopTime)
{
    return fmi_setup_experiment(c, toleranceDefined, tolerance, startTime,
                                stopTimeDefined, stopTime);
}

FMI2_Export fmi2Status fmi2EnterInitializationMode(fmi2Component c)
{
    return fmi_enter_initialization_mode(c);
}

FMI2_Export fmi2Status fmi2ExitInitializationMode(fmi2Component c)
{
    return fmi_exit_initialization_mode(c);
}

FMI2_Export fmi2Status fmi2GetReal(fmi2Component c, const fmi2ValueReference vr[], size_t nvr, fmi2Real value[])
{
	return fmi_get_real(c, vr, nvr, value);
}

FMI2_Export fmi2Status fmi2GetInteger(fmi2Component c, const fmi2ValueReference vr[], size_t nvr, fmi2Integer value[])
{
	return fmi_get_integer(c, vr, nvr, value);
}

FMI2_Export fmi2Status fmi2GetBoolean(fmi2Component c, const fmi2ValueReference vr[], size_t nvr, fmi2Boolean value[])
{
	return fmi_get_boolean(c, vr, nvr, value);
}

FMI2_Export fmi2Status fmi2GetString(fmi2Component c, const fmi2ValueReference vr[], size_t nvr, fmi2String  value[])
{
	return fmi_get_string(c, vr, nvr, value);
}

FMI2_Export fmi2Status fmi2SetReal(fmi2Component c, const fmi2ValueReference vr[], size_t nvr, const fmi2Real value[])
{
	return fmi_set_real(c, vr, nvr, value);
}

FMI2_Export fmi2Status fmi2SetInteger(fmi2Component c, const fmi2ValueReference vr[], size_t nvr, const fmi2Integer value[])
{
	return fmi_set_integer(c, vr, nvr, value);
}

FMI2_Export fmi2Status fmi2SetBoolean(fmi2Component c, const fmi2ValueReference vr[], size_t nvr, const fmi2Boolean value[])
{
	return fmi_set_boolean(c, vr, nvr, value);
}

FMI2_Export fmi2Status fmi2SetString(fmi2Component c, const fmi2ValueReference vr[], size_t nvr, const fmi2String  value[])
{
	return fmi_set_string(c, vr, nvr, value);
}

/* FMI 2.0 CS Functions */
FMI2_Export const char* fmi2GetTypesPlatform()
{
	return fmi_get_types_platform();
}

FMI2_Export fmi2Status fmi2Terminate(fmi2Component c)
{
	return fmi_terminate(c);
}

FMI2_Export fmi2Status fmi2Reset(fmi2Component c)
{
	return fmi_reset(c);
}

FMI2_Export fmi2Status fmi2SetRealInputDerivatives(fmi2Component c, const fmi2ValueReference vr[], size_t nvr, const fmi2Integer order[], const fmi2Real value[])
{
	return fmi_set_real_input_derivatives(c, vr, nvr, order, value);
}

FMI2_Export fmi2Status fmi2GetRealOutputDerivatives(fmi2Component c, const fmi2ValueReference vr[], size_t nvr, const fmi2Integer order[], fmi2Real value[])
{
	return fmi_get_real_output_derivatives(c, vr, nvr, order, value);
}

FMI2_Export fmi2Status fmi2CancelStep(fmi2Component c)
{
	return fmi_cancel_step(c);
}

FMI2_Export fmi2Status fmi2DoStep(fmi2Component c, fmi2Real currentCommunicationPoint, fmi2Real communicationStepSize, fmi2Boolean newStep)
{
	return fmi_do_step(c, currentCommunicationPoint, communicationStepSize, newStep);
}

FMI2_Export fmi2Status fmi2GetStatus(fmi2Component c, const fmi2StatusKind s, fmi2Status*  value)
{
	return fmi_get_status(c, s, value);
}

FMI2_Export fmi2Status fmi2GetRealStatus(fmi2Component c, const fmi2StatusKind s, fmi2Real*    value)
{
	return fmi_get_real_status(c, s, value);
}

FMI2_Export fmi2Status fmi2GetIntegerStatus(fmi2Component c, const fmi2StatusKind s, fmi2Integer* value)
{
	return fmi_get_integer_status(c, s, value);
}

FMI2_Export fmi2Status fmi2GetBooleanStatus(fmi2Component c, const fmi2StatusKind s, fmi2Boolean* value)
{
	return fmi_get_boolean_status(c, s, value);
}

FMI2_Export fmi2Status fmi2GetStringStatus(fmi2Component c, const fmi2StatusKind s, fmi2String*  value)
{
	return fmi_get_string_status(c, s, value);
}
