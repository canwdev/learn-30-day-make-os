#include "bootpack.h"
#include <stdio.h>

struct MOUSE_DEC {
  unsigned char buf[3], phase;
};

extern struct FIFO8 keyfifo, mousefifo;
void enable_mouse(struct MOUSE_DEC *mdec);
void init_keyboard(void);
int mouse_decode(struct MOUSE_DEC *mdec, unsigned char dat);

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
  mx = (binfo->scrnx - 12) / 2; // 计算鼠标在画面中心的坐标
  my = (binfo->scrny - 21) / 2;

  init_mouse_cursor8(mcursor, COL8_BG);
  putblock8_8(binfo->vram, binfo->scrnx, 12, 21, mx, my, mcursor, 12);

  // 输出debug信息
  sprintf(s, "(%d, %d)", mx, my);
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
          sprintf(s, "%02X %02X %02X", mdec.buf[0], mdec.buf[1], mdec.buf[2]);
          boxfill8(binfo->vram, binfo->scrnx, COL8_BG, 32, 16, 32+8*8-1, 31);
          putfonts8_asc(binfo->vram, binfo->scrnx, 32, 16, COL8_FFFFFF, s);
        } 

        
      }
    }
  }
}

#define PORT_KEYDAT				0x0060
#define PORT_KEYSTA				0x0064
#define PORT_KEYCMD				0x0064
#define KEYSTA_SEND_NOTREADY	0x02
#define KEYCMD_WRITE_MODE		0x60
#define KBC_MODE				0x47

/**
 * 等待键盘控制电路准备完毕
 * */
void wait_KBC_sendready(void)
{
  for (;;) {
    if ((io_in8(PORT_KEYSTA) & KEYSTA_SEND_NOTREADY) == 0) {
      break;
    }
  }
  return;
}

/**
 * 初始化键盘控制电路
 * */
void init_keyboard(void) 
{
  wait_KBC_sendready();
  io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE);
  wait_KBC_sendready();
  io_out8(PORT_KEYDAT, KBC_MODE);
  return;
}

#define KEYCMD_SENDTO_MOUSE		0xd4
#define MOUSECMD_ENABLE			0xf4

/**
 * 启用鼠标
 * */
void enable_mouse(struct MOUSE_DEC *mdec)
{
  wait_KBC_sendready();
  io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
  wait_KBC_sendready();
  io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);
   // 顺利的话，键盘控制器会返送回ACK(0xfa)
  mdec->phase = 0; // 等待0xfa的阶段
  return;
}

int mouse_decode(struct MOUSE_DEC *mdec, unsigned char dat)
{
  if (mdec->phase == 0)
  {
    // 等待鼠标的0xfa状态
    if (dat == 0xfa)
    {
      mdec->phase = 1;
    }
    return 0;
  }
  else if (mdec->phase == 1)
  {
    // 等待鼠标的第1字节
    mdec->buf[0] = dat;
    mdec->phase = 2;
    return 0;
  }
  else if (mdec->phase == 2)
  {
    // 等待鼠标的第2字节
    mdec->buf[1] = dat;
    mdec->phase = 3;
    return 0;
  }
  else if (mdec->phase == 3)
  {
    // 等待鼠标的第3字节
    mdec->buf[2] = dat;
    mdec->phase = 1;
    return 1;
  }
  return -1; // 应该不可能到这里来
}