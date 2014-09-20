#ifndef _AS_SENSOR_H_
#define _AS_SENSOR_H_

void sensor_sigio_handler(int signal,siginfo_t* info, void* value);
void sensor_main(int pipe_in,int pipe_out);

#endif /* _AS_SENSOR_H_ */