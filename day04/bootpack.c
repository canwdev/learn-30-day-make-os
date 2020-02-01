void io_hlt();
void write_mem8(int addr, int data);

void HariMain(void)
{
  int i;   // 变量声明，i是一个32位整数
  char *p; // 变量p，用于BYTE型地址

  for (i = 0xa0000; i <= 0xaffff; i++)
  {
    p = (char *) i; // 代入地址
    *p = i & 0x0f;
  }

  for (;;)
  {
    io_hlt();
  }
}
