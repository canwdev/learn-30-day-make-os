#include "bootpack.h"
#include <stdio.h>

extern struct KEYBUF keybuf;

void HariMain(void)
{
  struct BOOTINFO *binfo = (struct BOOTINFO *)ADR_BOOTINFO;
  char s[40], mcursor[252];
  int mx, my, i, j;

  init_gdtidt();
  init_pic();
  io_sti(); /* IDT/PIC的初始化已经完成，于是开放CPU的中断 */

	io_out8(PIC0_IMR, 0xf9); /* 开放PIC1和键盘中断(11111001) */
	io_out8(PIC1_IMR, 0xef); /* 开放鼠标中断(11101111) */

  init_palette(); // 初始化调色板
  init_screen8(binfo->vram, binfo->scrnx, binfo->scrny); // 绘制背景

  // 显示鼠标指针
  mx = (binfo->scrnx - 12) / 2; // 计算鼠标在画面中心的坐标
  my = (binfo->scrny - 21) / 2;

  init_mouse_cursor8(mcursor, COL8_BG);
  putblock8_8(binfo->vram, binfo->scrnx, 12, 21, mx, my, mcursor, 12);

  // 输出debug信息
  sprintf(s, "(%d, %d)", mx, my);
  putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, s);
  
  for (;;)
  {
    io_cli(); // 屏蔽中断

    if (keybuf.len == 0)
    {
      io_stihlt();
    }
    else
    {
      i = keybuf.data[keybuf.next_r];
      keybuf.len--;
      keybuf.next_r++;
      if (keybuf.next_r == 32) {
        keybuf.next_r = 0;
      }
      io_sti(); // 开放中断
      sprintf(s, "%02X", i);
      boxfill8(binfo->vram, binfo->scrnx, COL8_000000, 0, 16, 15, 31);
      putfonts8_asc(binfo->vram, binfo->scrnx, 0, 16, COL8_FFFFFF, s);
    }
  }
}
