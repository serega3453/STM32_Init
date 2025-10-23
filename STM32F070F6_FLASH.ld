/* STM32F070F6: 32K FLASH, 6K RAM */
MEMORY
{
  FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 32K
  RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 6K
}

/* Верх стека: 0x20001800 — обязательно */
_estack = ORIGIN(RAM) + LENGTH(RAM);
ENTRY(Reset_Handler)

SECTIONS
{
  .isr_vector : { KEEP(*(.isr_vector)) } > FLASH

  .text : {
    *(.text*) *(.rodata*) *(.glue_7) *(.glue_7t)
    KEEP(*(.init)) KEEP(*(.fini))
  } > FLASH

  .ARM.extab : { *(.ARM.extab* .gnu.linkonce.armextab.*) } > FLASH
  .ARM.exidx : { *(.ARM.exidx*) } > FLASH

  .data : AT (ADDR(.text) + SIZEOF(.text)) {
    _sdata = .;
    *(.data*)
    _edata = .;
  } > RAM

  .bss : {
    _sbss = .;
    *(.bss*) *(COMMON)
    _ebss = .;
  } > RAM

  _sidata = LOADADDR(.data);
}
