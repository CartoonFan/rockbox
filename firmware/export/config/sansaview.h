/*
 * This config file is for the Sandisk Sansa View
 */

/* For Rolo and boot loader */
#define MODEL_NUMBER 63
#define MODEL_NAME   "Sandisk View"

#define HW_SAMPR_CAPS       (SAMPR_CAP_44)

/* define this if you have recording possibility */
/* #define HAVE_RECORDING */

#define REC_SAMPR_CAPS      (SAMPR_CAP_22)
#define REC_FREQ_DEFAULT    REC_FREQ_22 /* Default is not 44.1kHz */
#define REC_SAMPR_DEFAULT   SAMPR_22

/* Define bitmask of input sources - recordable bitmask can be defined
   explicitly if different */
#define INPUT_SRC_CAPS (SRC_CAP_MIC | SRC_CAP_FMRADIO)




/* define this if you have a colour LCD */
#define HAVE_LCD_COLOR

/* define this if you want album art for this target */
#define HAVE_ALBUMART

/* define this to enable bitmap scaling */
#define HAVE_BMP_SCALING

/* define this to enable JPEG decoding */
#define HAVE_JPEG

/* define this if you have a light associated with the buttons */
#define HAVE_BUTTON_LIGHT

/* define this if you have access to the quickscreen */
#define HAVE_QUICKSCREEN

/* define this if you would like tagcache to build on this target */
#define HAVE_TAGCACHE

/* LCD dimensions */
#define LCD_WIDTH  240
#define LCD_HEIGHT 320
/* sqrt(240^2 + 320^2) / 2.4 = 166.7 */
#define LCD_DPI 167
#define LCD_DEPTH  16   /* 65536 colours */
#define LCD_PIXELFORMAT RGB565 /* rgb565 */

#ifndef BOOTLOADER
/* define this if you have LCD enable function */
#define HAVE_LCD_ENABLE
#endif

/* define this if you can flip your LCD */
#define HAVE_LCD_FLIP

/* define this if you can invert the colours on your LCD */
#define HAVE_LCD_INVERT

/* The only difference is that the power/hold is on the left instead of right on Fuze */
#define CONFIG_KEYPAD SANSA_FUZE_PAD

/* Define this to have CPU boosted while scrolling in the UI */
#define HAVE_GUI_BOOST

/* Define this to enable morse code input */
#define HAVE_MORSE_INPUT



/* There is no hardware tone control */
/* #define HAVE_SW_TONE_CONTROLS*/
#define HAVE_WM8731

/* define this if you have a real-time clock */
#ifndef BOOTLOADER
/* #define CONFIG_RTC RTC_ */
#endif

/* Define this if you have a software controlled poweroff */
#define HAVE_SW_POWEROFF

/* Required for MicroSD cards */
#define HAVE_FAT16SUPPORT

/* The number of bytes reserved for loadable codecs */
#define CODEC_SIZE 0x100000

/* The number of bytes reserved for loadable plugins */
#define PLUGIN_BUFFER_SIZE 0x80000

#define AB_REPEAT_ENABLE

/* Define this for LCD backlight available */
#define HAVE_BACKLIGHT
/* #define HAVE_BACKLIGHT_BRIGHTNESS */

/* define this if the unit uses a scrollwheel for navigation */
#define HAVE_SCROLLWHEEL
/* define to activate advanced wheel acceleration code */
#define HAVE_WHEEL_ACCELERATION
/* define from which rotation speed [degree/sec] on the acceleration starts */
#define WHEEL_ACCEL_START 540
/* define type of acceleration (1 = ^2, 2 = ^3, 3 = ^4) */
#define WHEEL_ACCELERATION 1

/* define this if you have a flash memory storage */
#define HAVE_FLASH_STORAGE

/* define this if the flash memory uses the SecureDigital Memory Card protocol */
#define CONFIG_STORAGE STORAGE_SD

#define BATTERY_CAPACITY_DEFAULT 750    /* default battery capacity */
#define BATTERY_CAPACITY_MIN 750        /* min. capacity selectable */
#define BATTERY_CAPACITY_MAX 750        /* max. capacity selectable */
#define BATTERY_CAPACITY_INC 0          /* capacity increment */


#define CONFIG_BATTERY_MEASURE VOLTAGE_MEASURE

/* Charging implemented in a target-specific algorithm */
#define CONFIG_CHARGING CHARGING_SIMPLE
#define HAVE_POWEROFF_WHILE_CHARGING

/* define current usage levels */
#define CURRENT_NORMAL     30  /* Toni's measurements in Nov 2008  */
#define CURRENT_BACKLIGHT  40  /* Screen is about 20, blue LEDs are another 20, so 40 if both */
#define CURRENT_RECORD     30  /* flash player, so this is just unboosted current*/

/* define this if the unit can be powered or charged via USB */
#define HAVE_USB_POWER

/* Define this if you have a PortalPlayer PP5024 */
#define CONFIG_CPU PP6100

/* Define this if you want to use the PP5024 i2c interface */
#define CONFIG_I2C I2C_PP5024

/* define this if the hardware can be powered off while charging */
/* Sansa can't be powered off while charging */
/* #define HAVE_POWEROFF_WHILE_CHARGING */

/* The start address index for ROM builds */
#define ROM_START 0x00000000

/* Define this to the CPU frequency */
#define CPU_FREQ      250000000

/* Type of LCD */
#define CONFIG_LCD LCD_VIEW

#ifndef BOOTLOADER
#define HAVE_MULTIDRIVE
#define NUM_DRIVES 2
#define HAVE_HOTSWAP
#endif

/* USB On-the-go */
#define CONFIG_USBOTG USBOTG_ARC

/* enable these for the experimental usb stack */
#define HAVE_USBSTACK
#define USB_VENDOR_ID 0x0781
#define USB_PRODUCT_ID 0x74b1

/* Define this if you have adjustable CPU frequency */
/* #define HAVE_ADJUSTABLE_CPU_FREQ */

#define MI4_FORMAT
#define BOOTFILE_EXT    "mi4"
#define BOOTFILE        "rockbox." BOOTFILE_EXT
#define BOOTDIR "/.rockbox"

#define ICODE_ATTR_TREMOR_NOT_MDCT

#define INCLUDE_TIMEOUT_API

/** Port-specific settings **/

/* Main LCD backlight brightness range and defaults */
#define MIN_BRIGHTNESS_SETTING      1
#define MAX_BRIGHTNESS_SETTING     12
#define DEFAULT_BRIGHTNESS_SETTING  6

/* Default recording levels */
#define DEFAULT_REC_MIC_GAIN    23
#define DEFAULT_REC_LEFT_GAIN   23
#define DEFAULT_REC_RIGHT_GAIN  23

/* Define this if a programmable hotkey is mapped */
#define HAVE_HOTKEY
