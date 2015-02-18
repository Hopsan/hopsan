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

/* This header is used to generate the FMU test DLL and in the C API test that uses the DLL */
#ifndef FMU1_MODEL_DEFINES_H_

#define FMU_CS_MODEL_IDENTIFIER <<<modelname>>>

#define BUFFER					1024

/* Sizes */
#define N_STATES				0
#define N_EVENT_INDICATORS		0
#define N_REAL					<<<n_reals>>>
#define N_INTEGER				0
#define N_BOOLEAN				0
#define N_STRING				0

#define N_INPUT_REAL			<<<n_inputs>>> /* CS only */
#define N_INPUT_REAL_MAX_ORDER	<<<n_inputs>>> /* CS only */
#define N_OUTPUT_REAL			<<<n_outputs>>> /* CS only */
#define N_OUTPUT_REAL_MAX_ORDER	<<<n_outputs>>> /* CS only */


#define FMI_VERSION			"1.0"
#if defined(FMI1_TYPES_H_)
#define FMI_PLATFORM_TYPE	fmi1_get_platform()
#elif defined(fmiModelTypesPlatform)
#define FMI_PLATFORM_TYPE	fmiModelTypesPlatform
#elif defined(fmiPlatform)
#define FMI_PLATFORM_TYPE	fmiPlatform
#else
#error "Either fmiPlatform or fmiModelTypesPlatform must be defined"
#endif
#define FMI_GUID			"<<<guid>>>"


#endif /* End of header FMU1_MODEL_DEFINES_H_ */
