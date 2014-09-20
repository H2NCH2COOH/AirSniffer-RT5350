#ifndef _AS_NET_H_
#define _AS_NET_H_

void net_sigio_handler(int signal,siginfo_t* info, void* value);
void net_main(int pipe_in,int pipe_out);

#endif /* _AS_NET_H_ */