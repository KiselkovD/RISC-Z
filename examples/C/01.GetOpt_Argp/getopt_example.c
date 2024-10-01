#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

int
main(int argc, char *argv[])
{
    int flags, opt;
    int nsecs, tfnd;

   nsecs = 0;
    tfnd = 0;
    flags = 0;
    int opto = 0;
    char *optov = 0;
    while ((opt = getopt(argc, argv, "nt:o:")) != -1) {
        printf("optind = %d\n", optind);
        switch (opt) {
        case 'n':
            flags = 1;
            break;
        case 't':
            nsecs = atoi(optarg);
            tfnd = 1;
            break;
        case 'o':
           opto = 1;
           optov = optarg;
           break;
        default: /* '?' */
            fprintf(stderr, "Usage: %s [-t nsecs] [-o p] [-n] name \n",
                    argv[0]);
            exit(EXIT_FAILURE);
        }
    }

   printf("flags=%d; tfnd=%d; optind=%d, opto=%d\n", flags, tfnd, optind, opto);
   if(opto)
   {
     printf("optov=%s\n", optov);
   }

   if (optind >= argc) {
        fprintf(stderr, "Expected argument after options\n");
        exit(EXIT_FAILURE);
    }

   printf("name argument = %s\n", argv[optind]);

   /* Other code omitted */

   exit(EXIT_SUCCESS);
}
