The project is the bootlader for QT2410 which is created by ADS IDE tool.
The meaning fo the project to me is to dig into how basic level of embedded system works(MMU, IRQ, ethernet...etc).

Below is a short instruction about how to use this bootloader.
This bootloader support TFTP upgrade and USB upgrade. Very easy to use, even if you dont know what a embedded system is.

Quick install steps:

    Download Image pack from download section

    Install GIVE IO driver

    connect wriggler , parallel port between your QT2410 board and computer

    Use biosjy.bat to burn bootloader.bin, and then enjoy

    If you want to use usb firmware upgrade, please use the USB host driver(WDM) at computer side. DNW is the windows UI software which can do bulk transfer with our usb firmware upgrade program
