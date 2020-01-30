; haribote-os

ORG 0xc200            ; 该程序的加载位置

MOV AL,0x13           ; VGA显卡，320x200x8位彩色
MOV AH,0x00
INT 0x10              ; 调用显卡BIOS
fin:
    HLT
    JMP fin 