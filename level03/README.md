# Level03

This level is purely about understanding the code. We need to input a password (converted to an int) that will be sent to a test() function, then a decrypt() fct.

- In test(), we substract our password to the number 322,424,845. We then enter a switch case statement. If the result is above 21, we send a random number to decrypt(), otherwise we call it with the previous result. We don't want a random number to be passed, so we already know we need a number that once substracted to 322,424,845 will be in the range [1, 21].

- In decrypt(), we XOR each character of the string "Q}|u`sfg~sf{}|a3" with our password. Once XOR'd, if the string becomes "Congratulations!" a shell is started.

We have everything we need to find the password. XOR has the very interesting properties of being symmetric and reversible. That means if A⊕B=C, then A⊕C=B and also B⊕C=A.
In our case, we XOR each char of the hashed string with our password. That means if we XOR the first character of the hashed string with the first char of the test string, we will have the correct value.

First char of the hashed string is Q, first char of the test string is C. Q ⊕ C = 18. Which is nice because it's a valid number for the swich case statement of test().

Now we know 322,424,845 - password = 18. Our password should be 322,424,827.

    level03@OverRide:~$ ./level03 322424827
    ***********************************
    *               level03         **
    ***********************************
    Password:322424827
    $ cd ../level04
    $ cat .pass
    kgv3tkEb9h2mLkRsPkXRfc2mHbjMxQzvb2FrgKkf
