#include "bootpack.h"
#include <stdio.h>


void HariMain(void)
{
  struct BOOTINFO *binfo = (struct BOOTINFO *)ADR_BOOTINFO;
  char s[40], mcursor[252], keybuf[32], mousebuf[128];
  int mx, my, i;
  struct MOUSE_DEC mdec;
  
  init_gdtidt();
  init_pic();
  io_sti(); /* IDT/PIC的初始化已经完成，于是开放CPU的中断 */

  // 键鼠FIFO缓冲区初始化
  fifo8_init(&keyfifo, 32, keybuf);
  fifo8_init(&mousefifo, 128, mousebuf);

	io_out8(PIC0_IMR, 0xf9); /* 开放PIC1和键盘中断(11111001) */
	io_out8(PIC1_IMR, 0xef); /* 开放鼠标中断(11101111) */

  init_keyboard(); // 初始化键鼠设备

  init_palette(); // 初始化调色板
  init_screen8(binfo->vram, binfo->scrnx, binfo->scrny); // 绘制背景

  // 显示鼠标指针
  mx = (binfo->scrnx - CURSOR_WIDTH) / 2; // 计算鼠标在画面中心的坐标
  my = (binfo->scrny - CURSOR_HEIGHT) / 2;

  init_mouse_cursor8(mcursor, COL8_BG);
  putblock8_8(binfo->vram, binfo->scrnx, CURSOR_WIDTH, CURSOR_HEIGHT, mx, my, mcursor, CURSOR_WIDTH);

  // 输出debug信息
  sprintf(s, "(%3d, %3d)", mx, my);
  putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, s);
  
  enable_mouse(&mdec);

  for (;;)
  {
    io_cli(); // 屏蔽中断

    if (fifo8_status(&keyfifo) + fifo8_status(&mousefifo) == 0)
    {
      io_stihlt();
    }
    else
    {
      if (fifo8_status(&keyfifo) != 0)
      {
        i = fifo8_get(&keyfifo);
        io_sti();
        sprintf(s, "%02X", i);
        boxfill8(binfo->vram, binfo->scrnx, COL8_BG, 0, 16, 15, 31);
        putfonts8_asc(binfo->vram, binfo->scrnx, 0, 16, COL8_FFFFFF, s);
      }
      else if (fifo8_status(&mousefifo) != 0)
      {
        i = fifo8_get(&mousefifo);
        io_sti();

        if (mouse_decode(&mdec, i) != 0) {
          // 鼠标的3个字节都齐了，显示出来

          sprintf(s, "[lcr %4d %4d]", mdec.x, mdec.y);

          if ((mdec.btn & 0x01) != 0)
          {
            s[1] = 'L';
          }
          if ((mdec.btn & 0x02) != 0)
          {
            s[3] = 'R';
          }
          if ((mdec.btn & 0x04) != 0)
          {
            s[2] = 'C';
          }

          boxfill8(binfo->vram, binfo->scrnx, COL8_BG, 32, 16, 32 + 15 * 8 - 1, 31);
          putfonts8_asc(binfo->vram, binfo->scrnx, 32, 16, COL8_FFFFFF, s);
        } 

        boxfill8(binfo->vram, binfo->scrnx, COL8_BG, mx, my, mx+CURSOR_WIDTH, my+CURSOR_HEIGHT); // 清除原来的鼠标
        mx += mdec.x; // 获得绝对坐标
        my += mdec.y;
        if (mx < 0) {
          mx = 0;
        }
        if (my < 0) {
          my = 0;
        }
        if (mx > binfo->scrnx - CURSOR_WIDTH)
        {
          mx = binfo->scrnx - CURSOR_WIDTH;
        }
        if (my > binfo->scrny -  CURSOR_HEIGHT)
        {
          my = binfo->scrny - CURSOR_HEIGHT;
        }
        
        // 输出绝对坐标
        sprintf(s, "(%3d, %3d)", mx, my);
        boxfill8(binfo->vram, binfo->scrnx, COL8_BG, 0, 0, 79, 15);
        putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, s);

        // 描画鼠标
        putblock8_8(binfo->vram, binfo->scrnx, CURSOR_WIDTH, CURSOR_HEIGHT, mx, my, mcursor, CURSOR_WIDTH);
      }
    }
  }
}

