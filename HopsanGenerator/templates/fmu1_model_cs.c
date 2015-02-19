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

/* Standard FMI 1.0 ME and CS types */
#include <FMI1/fmiPlatformTypes.h>
#include <FMI1/fmiFunctions.h>

#include "fmu1_model.h"

#define MODEL_IDENTIFIER FMU_CS_MODEL_IDENTIFIER

#ifdef _WIN32
#define DllExport __declspec(dllexport)
#endif


#include "fmu1_model.c"



/* FMI 1.0 Common Functions */
DllExport const char* fmiGetVersion()
{
	return fmi_get_version();
}

DllExport fmiStatus fmiSetDebugLogging(fmiComponent c, fmiBoolean loggingOn)
{
	return fmi_set_debug_logging(c, loggingOn);
}

DllExport fmiStatus fmiGetReal(fmiComponent c, const fmiValueReference vr[], size_t nvr, fmiReal value[])
{
	return fmi_get_real(c, vr, nvr, value);
}

DllExport fmiStatus fmiGetInteger(fmiComponent c, const fmiValueReference vr[], size_t nvr, fmiInteger value[])
{
	return fmi_get_integer(c, vr, nvr, value);
}

DllExport fmiStatus fmiGetBoolean(fmiComponent c, const fmiValueReference vr[], size_t nvr, fmiBoolean value[])
{
	return fmi_get_boolean(c, vr, nvr, value);
}

DllExport fmiStatus fmiGetString(fmiComponent c, const fmiValueReference vr[], size_t nvr, fmiString  value[])
{
	return fmi_get_string(c, vr, nvr, value);
}

DllExport fmiStatus fmiSetReal(fmiComponent c, const fmiValueReference vr[], size_t nvr, const fmiReal value[])
{
	return fmi_set_real(c, vr, nvr, value);
}

DllExport fmiStatus fmiSetInteger(fmiComponent c, const fmiValueReference vr[], size_t nvr, const fmiInteger value[])
{
	return fmi_set_integer(c, vr, nvr, value);
}

DllExport fmiStatus fmiSetBoolean(fmiComponent c, const fmiValueReference vr[], size_t nvr, const fmiBoolean value[])
{
	return fmi_set_boolean(c, vr, nvr, value);
}

DllExport fmiStatus fmiSetString(fmiComponent c, const fmiValueReference vr[], size_t nvr, const fmiString  value[])
{
	return fmi_set_string(c, vr, nvr, value);
}

/* FMI 1.0 CS Functions */
DllExport const char* fmiGetTypesPlatform()
{
	return fmi_get_types_platform();
}

DllExport fmiComponent fmiInstantiateSlave(fmiString instanceName, fmiString fmuGUID, fmiString fmuLocation, fmiString mimeType, fmiReal timeout, fmiBoolean visible, fmiBoolean interactive, fmiCallbackFunctions functions, fmiBoolean loggingOn)
{
	return fmi_instantiate_slave(instanceName, fmuGUID, fmuLocation, mimeType, timeout, visible, interactive, functions, loggingOn);
}

DllExport fmiStatus fmiInitializeSlave(fmiComponent c, fmiReal tStart, fmiBoolean StopTimeDefined, fmiReal tStop)
{
	return fmi_initialize_slave(c, tStart, StopTimeDefined, tStop);
}

DllExport fmiStatus fmiTerminateSlave(fmiComponent c)
{
	return fmi_terminate_slave(c);
}

DllExport fmiStatus fmiResetSlave(fmiComponent c)
{
	return fmi_reset_slave(c);
}

DllExport void fmiFreeSlaveInstance(fmiComponent c)
{
	fmi_free_slave_instance(c);
}

DllExport fmiStatus fmiSetRealInputDerivatives(fmiComponent c, const fmiValueReference vr[], size_t nvr, const fmiInteger order[], const fmiReal value[])
{
	return fmi_set_real_input_derivatives(c, vr, nvr, order, value);
}

DllExport fmiStatus fmiGetRealOutputDerivatives(fmiComponent c, const fmiValueReference vr[], size_t nvr, const fmiInteger order[], fmiReal value[])
{
	return fmi_get_real_output_derivatives(c, vr, nvr, order, value);
}

DllExport fmiStatus fmiCancelStep(fmiComponent c)
{
	return fmi_cancel_step(c);
}

DllExport fmiStatus fmiDoStep(fmiComponent c, fmiReal currentCommunicationPoint, fmiReal communicationStepSize, fmiBoolean newStep)
{
	return fmi_do_step(c, currentCommunicationPoint, communicationStepSize, newStep);
}

DllExport fmiStatus fmiGetStatus(fmiComponent c, const fmiStatusKind s, fmiStatus*  value)
{
	return fmi_get_status(c, s, value);
}

DllExport fmiStatus fmiGetRealStatus(fmiComponent c, const fmiStatusKind s, fmiReal*    value)
{
	return fmi_get_real_status(c, s, value);
}

DllExport fmiStatus fmiGetIntegerStatus(fmiComponent c, const fmiStatusKind s, fmiInteger* value)
{
	return fmi_get_integer_status(c, s, value);
}

DllExport fmiStatus fmiGetBooleanStatus(fmiComponent c, const fmiStatusKind s, fmiBoolean* value)
{
	return fmi_get_boolean_status(c, s, value);
}

DllExport fmiStatus fmiGetStringStatus(fmiComponent c, const fmiStatusKind s, fmiString*  value)
{
	return fmi_get_string_status(c, s, value);
}
