/*
 * This config file is for the Samsung YH-920
 */

/* For Rolo and boot loader */
#define MODEL_NUMBER 58
#define MODEL_NAME   "Samsung YH-920"

/* define this if you have recording possibility */
#define HAVE_RECORDING

/* Define bitmask of input sources - recordable bitmask can be defined
   explicitly if different */
#define INPUT_SRC_CAPS (SRC_CAP_MIC | SRC_CAP_LINEIN | SRC_CAP_FMRADIO)

/* define the bitmask of hardware sample rates */
#define HW_SAMPR_CAPS   (SAMPR_CAP_48 | SAMPR_CAP_44 | SAMPR_CAP_32 | \
                         SAMPR_CAP_24 | SAMPR_CAP_22 | SAMPR_CAP_16 | \
                         SAMPR_CAP_11 | SAMPR_CAP_8)

/* define the bitmask of recording sample rates */
#define REC_SAMPR_CAPS  (SAMPR_CAP_48 | SAMPR_CAP_44 | SAMPR_CAP_32 | \
                         SAMPR_CAP_24 | SAMPR_CAP_22 | SAMPR_CAP_16 | \
                         SAMPR_CAP_11 | SAMPR_CAP_8)

/* Type of LCD */
#define CONFIG_LCD LCD_S1D15E06

/* LCD dimensions */
#define LCD_WIDTH  160
#define LCD_HEIGHT 128
/* sqrt(160^2 + 128^2) / 1.8 = 113.8 */
#define LCD_DPI 114
#define LCD_DEPTH  2
#define LCD_PIXELFORMAT VERTICAL_PACKING

/* Display colours, for screenshots and sim (0xRRGGBB) */
#define LCD_DARKCOLOR       0x000000
#define LCD_BRIGHTCOLOR     0x5a915a
#define LCD_BL_DARKCOLOR    0x000000
#define LCD_BL_BRIGHTCOLOR  0xadd8e6

/* todo */
/* #ifndef BOOTLOADER */
#if 0
/* Define this if your LCD can be enabled/disabled */
#define HAVE_LCD_ENABLE

/* Define this if your LCD can be put to sleep. HAVE_LCD_ENABLE
 * should be defined as well.
 * We can currently put the lcd to sleep but it won't wake up properly */
#define HAVE_LCD_SLEEP
#define HAVE_LCD_SLEEP_SETTING
#endif

/* Define this for LCD backlight available */
#define HAVE_BACKLIGHT

/* Define this if your LCD can set contrast */
#define HAVE_LCD_CONTRAST

#define MIN_CONTRAST_SETTING        50
#define MAX_CONTRAST_SETTING        70
#define DEFAULT_CONTRAST_SETTING    61 /* Match boot contrast */

/* define this if you can flip your LCD */
#define HAVE_LCD_FLIP

/* define this if you can invert the colours on your LCD */
#define HAVE_LCD_INVERT

/* put the lcd frame buffer in IRAM */
/* #define IRAM_LCDFRAMEBUFFER IDATA_ATTR */




/* define this if you want album art for this target */
#define HAVE_ALBUMART

/* define this to enable bitmap scaling */
#define HAVE_BMP_SCALING

/* define this to enable JPEG decoding */
#define HAVE_JPEG

/* define this if you have access to the quickscreen */
#define HAVE_QUICKSCREEN

/* define this if you would like tagcache to build on this target */
#define HAVE_TAGCACHE

#define CONFIG_KEYPAD SAMSUNG_YH92X_PAD

/* Define this to enable morse code input */
#define HAVE_MORSE_INPUT




/* define this if you have a real-time clock */
#ifndef BOOTLOADER
#define CONFIG_RTC RTC_E8564
/* TODO ??? */
//#define HAVE_RTC_ALARM
#endif

/* define this if you have a disk storage, i.e. something
   that needs spinups and can cause skips when shaked */
#define HAVE_DISK_STORAGE

/* define this if you use an ATA controller */
#define CONFIG_STORAGE STORAGE_ATA

/*define this if the ATA controller and method of USB access support LBA48 */
#define HAVE_LBA48

/* We're able to shut off power to the HDD */
#define HAVE_ATA_POWER_OFF

/* Software controlled LED */
#define CONFIG_LED LED_REAL

/* Define this if you have a software controlled poweroff */
#define HAVE_SW_POWEROFF

/* The number of bytes reserved for loadable codecs */
#define CODEC_SIZE 0x100000

/* The number of bytes reserved for loadable plugins */
#define PLUGIN_BUFFER_SIZE 0x80000

/* Define this if you have the AK4537 audio codec */
#define HAVE_AK4537

/* AK4537 has no tone controls, so we use the software ones */
#define HAVE_SW_TONE_CONTROLS

/* FM Tuner */
#define CONFIG_TUNER      TEA5767
#define CONFIG_TUNER_XTAL 32768
/* Define this if the tuner uses 3-wire bus instead of classic i2c */
#define CONFIG_TUNER_3WIRE

#define AB_REPEAT_ENABLE

#define BATTERY_CAPACITY_DEFAULT 900 /* default battery capacity */
#define BATTERY_CAPACITY_MIN 900  /* min. capacity selectable */
#define BATTERY_CAPACITY_MAX 1600 /* max. capacity selectable */
#define BATTERY_CAPACITY_INC 50   /* capacity increment */


#define CONFIG_BATTERY_MEASURE VOLTAGE_MEASURE

/* Hardware controlled charging */
#define CONFIG_CHARGING CHARGING_SIMPLE

/* define this if the unit can be powered or charged via USB */
#define HAVE_USB_POWER

/* Define this if you have a PortalPlayer PP5020 */
#define CONFIG_CPU PP5020

/* Define this if you want to use the PP5020 i2c interface */
#define CONFIG_I2C I2C_PP5020

/* define this if the hardware can be powered off while charging */
#define HAVE_POWEROFF_WHILE_CHARGING

/* The start address index for ROM builds */
#define ROM_START 0x00000000

/* The size of the flash ROM */
#define FLASH_SIZE 0x100000

/* Define this to the CPU frequency */
#define CPU_FREQ      75000000

/* USB On-the-go */
#define CONFIG_USBOTG USBOTG_ARC

/* enable these for the experimental usb stack */
#define HAVE_USBSTACK
#define USB_VENDOR_ID   0x04e8
#define USB_PRODUCT_ID  0x5022
#define HAVE_USB_HID_MOUSE

/* Define this if you have adjustable CPU frequency */
#define HAVE_ADJUSTABLE_CPU_FREQ

#define MI4_FORMAT
#define BOOTFILE_EXT    "mi4"
#define BOOTFILE        "rockbox." BOOTFILE_EXT
#define BOOTDIR "/.rockbox"

#define ICODE_ATTR_TREMOR_NOT_MDCT


/* DMA is used only for reading on PP502x because although reads are ~8x faster
 * writes appear to be ~25% slower.
 */
#ifndef BOOTLOADER
#define HAVE_ATA_DMA
#endif

/* Define this if a programmable hotkey is mapped */
#define HAVE_HOTKEY
