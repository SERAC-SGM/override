//----- (08048748) --------------------------------------------------------
bool auth(char *login, int inputSerial)
{
  int i; // [esp+14h] [ebp-14h]
  int serial; // [esp+18h] [ebp-10h]
  int loginLen; // [esp+1Ch] [ebp-Ch]

  login[strcspn(login, "\n")] = 0;
  loginLen = strnlen(login, 32);
  if ( loginLen <= 5 )
    return 1;
  if ( ptrace(PTRACE_TRACEME, 0, 1, 0) == -1 )
  {
    puts("\x1B[32m.---------------------------.");
    puts("\x1B[31m| !! TAMPERING DETECTED !!  |");
    puts("\x1B[32m'---------------------------'");
    return 1;
  }
  else
  {
    serial = (login[3] ^ 4919) + 6221293;
    for ( i = 0; i < loginLen; ++i )
    {
      if ( login[i] <= 31 )
        return 1;
      serial += (serial ^ (unsigned int)login[i]) % 1337;
    }
    return inputSerial != serial;
  }
}

//----- (08048879) --------------------------------------------------------
int main(int argc, const char **argv, const char **envp)
{
  int serial; // [esp+2Ch] [ebp-24h] BYREF
  char login[28]; // [esp+30h] [ebp-20h] BYREF
  unsigned int v6; // [esp+4Ch] [ebp-4h]

  v6 = __readgsdword(0x14u);
  puts("***********************************");
  puts("*\t\tlevel06\t\t  *");
  puts("***********************************");
  printf("-> Enter Login: ");
  fgets(login, 32, stdin);
  puts("***********************************");
  puts("***** NEW ACCOUNT DETECTED ********");
  puts("***********************************");
  printf("-> Enter Serial: ");
  scanf("%d", &serial);
  if ( auth(login, serial) )
    return 1;
  puts("Authenticated!");
  system("/bin/sh");
  return 0;
}
