# Level06

This binary asks for a login, then a serial key. A ptrace call inside the auth() function prevents us from using gdb to have a look at the real serial key. But no problem, we can simply copy-paste the disassembled code (sort of) and print the real serial key given a specific login input. Here is the clean code :

    #include <unistd.h>
    #include <string.h>
    #include <stdio.h>
    #include <stdlib.h>

    int auth(char *login)
    {
        login[strcspn(login, "\n")] = 0;
        int len = strnlen(login, 28);
        if (len <= 5)
        {
            printf("invalid size\n");
            return (1);
        }
        int serial = (login[3] ^ 4919) + 6221293;
        for (int i = 0; i < len; i++)
        {
            if (login[i] <= 31) return 1;
            serial += (serial ^ (unsigned int)login[i]) % 1337;
        }
        printf("serial = : %d\n", serial);
        return 0;
    }

    int main(int ac, char **av)
    {
        (void)ac;

        auth(av[1]);
        return (0);
    }

Now, let's run it with an input (we can see it requires at least 6 characters, and less than 28 since we messed up the sizes) :

    $ gcc getSerial.c
    
    $ ./a.out aaaaaa
    serial = 6231562

We have the real serial, we just have to pass it to our binary :

    level06@OverRide:~$ ./level06 
    ***********************************
    *		level06		  *
    ***********************************
    -> Enter Login: aaaaaa
    ***********************************
    ***** NEW ACCOUNT DETECTED ********
    ***********************************
    -> Enter Serial: 6231562
    Authenticated!
    $ cd ../level07
    $ cat .pass
    GbcPDRgsFK77LNnnuh7QyFYA2942Gp8yKj9KrWD8
