/* 关于绘图部分的处理 */

#include "bootpack.h"

void init_palette(void)
{
  static unsigned char table_rgb[17 * 3] = {
      0x00, 0x00, 0x00, /*  0:黑 */
      0xff, 0x00, 0x00, /*  1:亮红 */
      0x00, 0xff, 0x00, /*  2:亮绿 */
      0xff, 0xff, 0x00, /*  3:亮黄 */
      0x00, 0x00, 0xff, /*  4:亮蓝 */
      0xff, 0x00, 0xff, /*  5:亮紫 */
      0x00, 0xff, 0xff, /*  6:浅亮蓝 */
      0xff, 0xff, 0xff, /*  7:白 */
      0xc6, 0xc6, 0xc6, /*  8:亮灰 */
      0x84, 0x00, 0x00, /*  9:暗红 */
      0x00, 0x84, 0x00, /* 10:暗绿 */
      0x84, 0x84, 0x00, /* 11:暗黄 */
      0x00, 0x00, 0x84, /* 12:暗蓝 */
      0x84, 0x00, 0x84, /* 13:暗紫 */
      0x00, 0x84, 0x84, /* 14:浅暗蓝 */
      0x84, 0x84, 0x84,  /* 15:暗灰 */
      0x3f, 0x5e, 0xb5, /* 16:Material */
  };
  set_palette(0, 16, table_rgb);
  return;

  // C语言中的static char语句只能用于数据，相当于汇编中的DB指令
}

void set_palette(int start, int end, unsigned char *rgb)
{
  int i, eflags;
  eflags = io_load_eflags(); // 记录中断许可标志的值
  io_cli();                  // 将中断许可标志置为0，禁止中断
  io_out8(0x03c8, start);    // 开始写入调色板数据

  for (i = start; i <= end; i++)
  {
    io_out8(0x03c9, rgb[0] / 4);
    io_out8(0x03c9, rgb[1] / 4);
    io_out8(0x03c9, rgb[2] / 4);
    rgb += 3;
  }
  io_store_eflags(eflags); // 恢复许可标志的值
  return;
}

/**
 * 绘制矩形
 * varm 显存地址
 * x 屏幕宽度（用于计算y值）
 * c 颜色代码
 * x0 x轴绘制起点
 * y0 y轴绘制起点
 * x1 x轴绘制终点
 * y1 y轴绘制终点
 */
void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1)
{
  int x, y;
  for (y = y0; y <= y1; y++)
  {
    for (x = x0; x <= x1; x++)
    {
      *(vram + (y * xsize + x)) = c;
    }
  }
  return;
}

void init_screen8(char *vram, int x, int y)
{
  // 绘制桌面背景
  int barHeight = 30;

  boxfill8(vram, x, COL8_BG, 0, 0, x - 1, y - barHeight);

  // 绘制任务栏
  boxfill8(vram, x, COL8_C6C6C6, 0, y - barHeight + 1, x - 1, y - barHeight + 1);
  boxfill8(vram, x, COL8_FFFFFF, 0, y - barHeight + 2, x - 1, y - barHeight + 2);
  boxfill8(vram, x, COL8_C6C6C6, 0, y - barHeight + 3, x - 1, y - 1);

  // 开始按钮
  int buttonWidth = 50;
  boxfill8(vram, x, COL8_FFFFFF, 3, y - barHeight + 5, buttonWidth + 3, y - barHeight + 5);
  boxfill8(vram, x, COL8_FFFFFF, 2, y - barHeight + 5, 2, y - 4);
  boxfill8(vram, x, COL8_848484, 3, y - 4, buttonWidth + 3, y - 4);
  boxfill8(vram, x, COL8_848484, buttonWidth + 3, y - barHeight + 5, buttonWidth + 3, y - 4);

  // 通知区域
  int statWidth = 50;
  boxfill8(vram, x, COL8_848484, x - statWidth - 3, y - barHeight + 5, x - 3, y - barHeight + 5);
  boxfill8(vram, x, COL8_848484, x - statWidth - 4, y - barHeight + 5, x - statWidth - 4, y - 4);
  boxfill8(vram, x, COL8_FFFFFF, x - statWidth - 3, y - 4, x - 3, y - 4);
  boxfill8(vram, x, COL8_FFFFFF, x - 3, y - barHeight + 6, x - 3, y - 4);

  return;
}

void putfont8(char *vram, int xsize, int x, int y, char c, char *font)
{
  int i;
  char *p, d /* data */;
  for (i = 0; i < 16; i++)
  {
    p = vram + (y + i) * xsize + x;
    d = font[i];
    if ((d & 0x80) != 0) { p[0] = c; }
    if ((d & 0x40) != 0) { p[1] = c; }
    if ((d & 0x20) != 0) { p[2] = c; }
    if ((d & 0x10) != 0) { p[3] = c; }
    if ((d & 0x08) != 0) { p[4] = c; }
    if ((d & 0x04) != 0) { p[5] = c; }
    if ((d & 0x02) != 0) { p[6] = c; }
    if ((d & 0x01) != 0) { p[7] = c; }
  }
  return;
}

void putfonts8_asc(char *vram, int xsize, int x, int y, char c, unsigned char *s)
{
  extern char hankaku[4096];
  for (; *s != 0x00; s++) // 遍历字符串（字符串以 0x00 结尾）
  {
    putfont8(vram, xsize, x, y, c, hankaku + *s * 16);
    x += 8;
  }
  return;
}

/**
 * 初始化鼠标指针(CURSOR_HEIGHT*CURSOR_WIDTH)
 * */
void init_mouse_cursor8(char *mouse, char bc)
{
  static char cursor[CURSOR_HEIGHT][CURSOR_WIDTH] = {
    "*...........",
    "**..........",
    "*0*.........",
    "*00*........",
    "*000*.......",
    "*0000*......",
    "*00000*.....",
    "*000000*....",
    "*0000000*...",
    "*00000000*..",
    "*000000000*.",
    "*000000*****",
    "*000*00*....",
    "*00**00*....",
    "*0*..*00*...",
    "**...*00*...",
    "*.....*00*..",
    "......*00*..",
    ".......*00*.",
    ".......*00*.",
    "........**..",
  };

  int x, y;

  for (y = 0; y < CURSOR_HEIGHT; y++)
  {
    for (x = 0; x < CURSOR_WIDTH; x++)
    {
      if (cursor[y][x] == '*')
      {
        mouse[y * CURSOR_WIDTH + x] = COL8_FFFFFF;
      }
      if (cursor[y][x] == '0')
      {
        mouse[y * CURSOR_WIDTH + x] = COL8_000000;
      }
      if (cursor[y][x] == '.')
      {
        mouse[y * CURSOR_WIDTH + x] = bc;
      }
    }
  }
  return;
}

void putblock8_8(char *vram, int vxsize, int pxsize, int pysize, int px0, int py0, char *buf, int bxsize)
{
  int x, y;
  for (y = 0; y < pysize; y++)
  {
    for (x = 0; x < pxsize; x++)
    {
      vram[(py0 + y) * vxsize + (px0 + x)] = buf[y * bxsize + x];
    }
  }
  return;
}
