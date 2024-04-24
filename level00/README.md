# Level00

That's an easy one. Looking at the source file, we can see that the binary waits for a password using scanf. If the provided password is correct, a shell is run. Decompiling the code, we can see that the password is hard coded and is equal to 5276

    level00@OverRide:~$ ./level00 123
    ***********************************
    * 	     -Level00 -		  *
    ***********************************
    Password:5276

    Authenticated!
    $ cd ../level01
    $ cat .pass
    uSq2ehEGT6c9S24zbshexZQBXUGrncxn5sD5QfGL
