
.\mkspiffs.exe -c data -s 0xFB000 -p 256 -b 8192 spiffs.bin
esptool.py --port COM5 write_flash 0x300000 .\spiffs.bin