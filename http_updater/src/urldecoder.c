#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

void urldecode(char *dst, const char *src)
{
        char a, b;
        while (*src) {
                if ((*src == '%') &&
                    ((a = src[1]) && (b = src[2])) &&
                    (isxdigit(a) && isxdigit(b))) {
                        if (a >= 'a')
                                a -= 'a'-'A';
                        if (a >= 'A')
                                a -= ('A' - 10);
                        else
                                a -= '0';
                        if (b >= 'a')
                                b -= 'a'-'A';
                        if (b >= 'A')
                                b -= ('A' - 10);
                        else
                                b -= '0';
                        *dst++ = 16*a+b;
                        src+=3;
                } else {
                        *dst++ = *src++;
                }
        }
        *dst++ = '\0';
}

int main(int argc,char* argv[])
{
    char buff[1024];
    char buff_in[1024];
    char* src;
    int c;
    
    if(argc>2)
    {
        printf("Usage: %s [string to decode]\nInput and result can't be longer than 1024\n",argv[0]);
        return 1;
    }
    
    if(argc==1)
    {
        c=fread(buff_in,1,1024,stdin);
        buff_in[c]='\0';
        src=buff_in;
    }
    else
    {
        src=argv[1];
    }
    
    urldecode(buff,src);
    printf("%s",buff);
}