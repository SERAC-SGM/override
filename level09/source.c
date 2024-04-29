//----- (000000000000088C) ----------------------------------------------------
int secret_backdoor()
{
  char buffer[128]; // [rsp+0h] [rbp-80h] BYREF

  fgets(buffer, 128, stdin);
  return system(buffer);
}

//----- (00000000000008C0) ----------------------------------------------------
int handle_msg()
{
  char buf[140]; // [rsp+0h] [rbp-C0h] BYREF

  set_username(char *buf);
  set_msg(char *buf);
  return puts(">: Msg sent!");
}

//----- (0000000000000932) ----------------------------------------------------
char *set_msg(char *buf)
{
  char s[1024]; // [rsp+10h] [rbp-400h] BYREF

  memset(s, 0, sizeof(s));
  puts(">: Msg @Unix-Dude");
  printf(">>: ");
  fgets(s, 1024, stdin);
  return strncpy(buf, s, 140);
}

//----- (00000000000009CD) ----------------------------------------------------
int set_username(char *buf)
{
  char s[140]; // [rsp+10h] [rbp-90h] BYREF
  int i; // [rsp+9Ch] [rbp-4h]

  memset(s, 0, 128);
  puts(">: Enter your username");
  printf(">>: ");
  fgets(s, 128, stdin);
  for ( i = 0; i <= 40 && s[i]; ++i )
    buf[i + 140] = s[i];
  return printf(">: Welcome, %s", buf[140]);
}
// 9CD: using guessed type char s[140];

//----- (0000000000000AA8) ----------------------------------------------------
int main(int argc, const char **argv, const char **envp)
{
  puts(
    "--------------------------------------------\n"
    "|   ~Welcome to l33t-m$n ~    buf337        |\n"
    "--------------------------------------------");
  handle_msg();
  return 0;
}
