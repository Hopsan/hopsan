/*-----------------------------------------------------------------------------

 Copyright 2017 Hopsan Group

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.


 The full license is available in the file LICENSE.
 For details about the 'Hopsan Group' or information about Authors and
 Contributors see the HOPSANGROUP and AUTHORS files that are located in
 the Hopsan source code root directory.

-----------------------------------------------------------------------------*/

#ifndef FMU2_HOPSAN_H_
#define FMU2_HOPSAN_H_

#ifdef __cplusplus
extern "C" {
#endif

int hopsan_instantiate();
int hopsan_initialize(double startT, double stopT);
void hopsan_simulate(double stopTime);
void hopsan_finalize();

double hopsan_get_real(int vr);
int hopsan_get_integer(int vr);
int hopsan_get_boolean(int vr);
const char* hopsan_get_string(int vr);

void hopsan_set_real(int vr, double value);
void hopsan_set_integer(int vr, int value);
void hopsan_set_boolean(int vr, int value);
void hopsan_set_string(int vr, const char* value);

#ifdef __cplusplus
}
#endif

#endif /* End of header FMU2_HOPSAN_H_ */
