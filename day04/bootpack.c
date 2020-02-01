void io_hlt();
void write_mem8(int addr, int data);

void HariMain(void)
{
  int i; // 变量声明，i是一个32位整数

  for (i = 0xa0000; i <= 0xaffff; i++)
  {
    write_mem8(i, i & 0x0f); /* 相当于：MOV BYTE [i],15 */
  }

  for (;;)
  {
    io_hlt();
  }
}