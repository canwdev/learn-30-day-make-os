#include "bootpack.h"
#include <stdio.h>

void HariMain(void)
{
  struct BOOTINFO *binfo = (struct BOOTINFO *)ADR_BOOTINFO;
  char s[40], mcursor[252];
  int mx, my;

  init_gdtidt();
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
    io_hlt();
  }
}
