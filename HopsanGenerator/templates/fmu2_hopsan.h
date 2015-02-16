#ifndef FMU2_HOPSAN_H_
#define FMU2_HOPSAN_H_

#ifdef __cplusplus
extern "C" {
#endif

void hopsan_instantiate();
void hopsan_initialize();
void hopsan_simulate(double stopTime);

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

#endif /* End of header FMU2_MODEL_H_ */
