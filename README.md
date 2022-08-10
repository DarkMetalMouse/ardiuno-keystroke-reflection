# ardiuno-keystroke-reflection
This is an open source implementation of the Keystroke Reflection side channel attack demonstrated by Hak5 in [on their channel](https://www.youtube.com/watch?v=Qg1M3jUCPgw). The data exfiltrated is stored in a file on an SD card connected to the Arduino.

This attack works because the state of the status LEDs (CAPS LOCK, NUM LOCK, SCROLL LOCK etc.) is controlled by the host computer. When you press for example CAPS LOCK, the keyboard sends the keystroke to the computer and the computer then sends a command to the keyboard to turn on the CAPS LOCK LED. Because of this, if we can simulate keystrokes on the computer, we can exfiltrate any data by encoding it as a sequence of CAPS LOCK and NUM LOCK presses.

Keystroke Reflection is slower than other kinds of exfiltration methods such copying the file to a USB storage device or sending the data over the internet. It is intended for air-gapped systems that block external storage devices and have no internet connection. 

The sketch was run on an Arduino Pro Micro connected to an SD card adapter, but it can be used with any microcontroller supported by [HID-Project](https://github.com/NicoHood/HID).

## Wiring

| SD Card Adapter | Arduino | 
| --------------- | ------- |
| GND             | GND     | 
| VCC             | VCC     | 
| MOSI            | D16     |
| MISO            | D14     |
| CLK             | D15     |
| CS              | D10     |

## Custom payload
The payload itself starts on [Line 137 of StealWifiCreds.ino](StealWifiCreds/StealWifiCreds.ino#L137). After you run the command to reflect the keystrokes, call [exfiltrate(file)](StealWifiCreds/StealWifiCreds.ino#L73) to read the data and store it on the SD card.
