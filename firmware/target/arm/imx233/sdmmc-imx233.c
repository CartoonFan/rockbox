/***************************************************************************
 *             __________               __   ___.
 *   Open      \______   \ ____   ____ |  | _\_ |__   _______  ___
 *   Source     |       _//  _ \_/ ___\|  |/ /| __ \ /  _ \  \/  /
 *   Jukebox    |    |   (  <_> )  \___|    < | \_\ (  <_> > <  <
 *   Firmware   |____|_  /\____/ \___  >__|_ \|___  /\____/__/\_ \
 *                     \/            \/     \/    \/            \/
 * $Id$
 *
 * Copyright (C) 2011 by Amaury Pouly
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 ****************************************************************************/
#include "config.h"
#include "system.h"
#include "sd.h"
#include "mmc.h"
#include "sdmmc.h"
#include "ssp-imx233.h"
#include "pinctrl-imx233.h"
#include "partitions-imx233.h"
#include "button-target.h"
#include "disk.h"
#include "usb.h"
#include "debug.h"
#include "string.h"
#include "ata_idle_notify.h"
#include "led.h"

#include "regs/ssp.h"

/** NOTE For convenience, this drivers relies on the many similar commands
 * between SD and MMC. The following assumptions are made:
 * - SD_SEND_STATUS = MMC_SEND_STATUS
 * - SD_SELECT_CARD = MMC_SELECT_CARD
 * - SD_TRAN = MMC_TRAN
 * - MMC_WRITE_MULTIPLE_BLOCK = SD_WRITE_MULTIPLE_BLOCK
 * - MMC_READ_MULTIPLE_BLOCK = SD_READ_MULTIPLE_BLOCK
 * - SD_STOP_TRANSMISSION = MMC_STOP_TRANSMISSION
 * - SD_DESELECT_CARD = MMC_DESELECT_CARD
 */
#if SD_SEND_STATUS != MMC_SEND_STATUS || SD_SELECT_CARD != MMC_SELECT_CARD || \
    SD_TRAN != MMC_TRAN || MMC_WRITE_MULTIPLE_BLOCK != SD_WRITE_MULTIPLE_BLOCK || \
    MMC_READ_MULTIPLE_BLOCK != SD_READ_MULTIPLE_BLOCK || \
    SD_STOP_TRANSMISSION != MMC_STOP_TRANSMISSION || \
    SD_DESELECT_CARD != MMC_DESELECT_CARD
#error SD/MMC mismatch
#endif

/* static configuration */
struct sdmmc_config_t
{
    const char *name; /* name(for debug) */
    int flags; /* flags */
    int power_pin; /* power pin */
    int power_delay; /* extra power up delay */
    int wp_pin; /* write protect pin */
    int ssp; /* associated ssp block */
    int mode; /* mode (SD vs MMC) */
};

/* flags */
#define POWER_PIN       (1 << 0)
#define POWER_INVERTED  (1 << 1)
#define REMOVABLE       (1 << 2)
#define DETECT_INVERTED (1 << 3)
#define POWER_DELAY     (1 << 4)
#define WINDOW          (1 << 5)
#define WP_PIN          (1 << 6)
#define WP_INVERTED     (1 << 7)
#define PROBE           (1 << 8)

/* modes */
#define SD_MODE         0
#define MMC_MODE        1

#define PIN(bank,pin) ((bank) << 5 | (pin))
#define PIN2BANK(v) ((v) >> 5)
#define PIN2PIN(v) ((v) & 0x1f)

const struct sdmmc_config_t sdmmc_config[] =
{
#ifdef SANSA_FUZEPLUS
    /* The Fuze+ uses pin #B0P8 for power */
    {
        .name = "microSD",
        .flags = POWER_PIN | POWER_INVERTED | REMOVABLE,
        .power_pin = PIN(0, 8),
        .ssp = 1,
        .mode = SD_MODE,
    },
    /* The Fuze+ uses pin #B1P29 for power */
    {
        .name = "eMMC",
        .flags = POWER_PIN | POWER_INVERTED | WINDOW | POWER_DELAY,
        .power_pin = PIN(1, 29),
        .power_delay = HZ / 5, /* extra delay, to ramp up voltage? */
        .ssp = 2,
        .mode = MMC_MODE,
    },
#elif defined(CREATIVE_ZENXFI2)
    {
        .name = "internal/SD",
        .flags = WINDOW | PROBE,
        .ssp = 2,
        .mode = SD_MODE,
    },
    /* The Zen X-Fi2 uses pin B1P29 for power */
    {
        .name = "microSD",
        .flags = POWER_PIN | REMOVABLE | DETECT_INVERTED,
        .power_pin = PIN(1, 29),
        .ssp = 1,
        .mode = SD_MODE,
    },
#elif defined(CREATIVE_ZENXFI3)
    {
        .name = "internal/SD",
        .flags = WINDOW,
        .ssp = 2,
        .mode = SD_MODE,
    },
    /* The Zen X-Fi3 uses pin #B0P07 for power */
    {
        .name = "microSD",
        .flags = POWER_PIN | POWER_INVERTED | REMOVABLE | POWER_DELAY,
        .power_pin = PIN(0, 7),
        .power_delay = HZ / 10, /* extra delay, to ramp up voltage? */
        .ssp = 1,
        .mode = SD_MODE,
    },
#elif defined(CREATIVE_ZENXFI) || defined(CREATIVE_ZEN)
    {
        .name = "internal/SD",
        .flags = WINDOW,
        .ssp = 2,
        .mode = SD_MODE,
    },
    /* The Zen X-Fi uses pin #B0P10 for power*/
    {
        .name = "SD",
        .flags = POWER_PIN | REMOVABLE | DETECT_INVERTED | POWER_DELAY | WP_PIN,
        .power_pin = PIN(0, 10),
        .wp_pin = PIN(0, 11),
        .power_delay = HZ / 10, /* extra delay, to ramp up voltage? */
        .ssp = 1,
        .mode = SD_MODE,
    },
#elif defined(CREATIVE_ZENMOZAIC)
    {
        .name = "internal/SD",
        .flags = WINDOW,
        .ssp = 2,
        .mode = SD_MODE,
    }
#elif defined(CREATIVE_ZENXFISTYLE)
    {
        .name = "internal/SD",
        .flags = WINDOW,
        .ssp = 2,
        .mode = SD_MODE
    },
#elif defined(SONY_NWZE370) || defined(SONY_NWZE360)
    /* The Sony NWZ-E370 uses #B1P29 for power */
    {
        .name = "internal/SD",
        .flags = POWER_PIN | POWER_INVERTED | WINDOW,
        .power_pin = PIN(1, 29),
        .ssp = 2,
        .mode = MMC_MODE
    },
#else
#error You need to write the sd/mmc config!
#endif
};

/* drive status */
struct sdmmc_status_t
{
    int bus_width; /* bus width (1, 4 or 8) */
    bool hs_capable; /* HS capable device */
    bool hs_enabled; /* HS enabled */
    bool has_sbc; /* support SET_BLOCK_COUNT */
};

#define SDMMC_NUM_DRIVES    (sizeof(sdmmc_config) / sizeof(sdmmc_config[0]))

#define SDMMC_CONF(drive) sdmmc_config[drive]
#define SDMMC_FLAGS(drive) SDMMC_CONF(drive).flags
#define SDMMC_SSP(drive) SDMMC_CONF(drive).ssp
#define SDMMC_MODE(drive) SDMMC_CONF(drive).mode

/** WARNING
 * to be consistent with all our SD drivers, the .rca field of sdmmc_card_info
 * in reality holds (rca << 16) because all command arguments actually require
 * the RCA is the 16-bit msb. Be careful that this is not the actuall RCA ! */

/* common */
static sector_t window_start[SDMMC_NUM_DRIVES];
static sector_t window_end[SDMMC_NUM_DRIVES];
static uint8_t aligned_buffer[SDMMC_NUM_DRIVES][512] CACHEALIGN_ATTR;
static tCardInfo sdmmc_card_info[SDMMC_NUM_DRIVES];
static struct mutex mutex[SDMMC_NUM_DRIVES];
static int disk_last_activity[SDMMC_NUM_DRIVES];
static struct sdmmc_status_t sdmmc_status[SDMMC_NUM_DRIVES];
#define MIN_YIELD_PERIOD    5

#define SDMMC_INFO(drive) sdmmc_card_info[drive]
#define SDMMC_RCA(drive) SDMMC_INFO(drive).rca
#define SDMMC_STATUS(drive) sdmmc_status[drive]

/* sd only */
#if CONFIG_STORAGE & STORAGE_SD
static int sd_first_drive;
static unsigned _sd_num_drives;
static int sd_map[SDMMC_NUM_DRIVES]; /* sd->sdmmc map */
#endif
/* mmc only */
#if CONFIG_STORAGE & STORAGE_MMC
static int mmc_first_drive;
static unsigned _mmc_num_drives;
static int mmc_map[SDMMC_NUM_DRIVES]; /* mmc->sdmmc map */
#endif

static int init_drive(int drive);

/* WARNING NOTE BUG FIXME
 * There are three numbering schemes involved in the driver:
 * - the sdmmc indexes into sdmmc_config[]
 * - the sd drive indexes
 * - the mmc drive indexes
 * By convention, [drive] refers to a sdmmc index whereas sd_drive/mmc_drive
 * refer to sd/mmc drive indexes. We keep two maps sd->sdmmc and mmc->sdmmc
 * to find the sdmmc index from the sd or mmc one */

static inline int sdmmc_removable(int drive)
{
    return SDMMC_FLAGS(drive) & REMOVABLE;
}

static int sdmmc_present(int drive)
{
    if(sdmmc_removable(drive))
        return imx233_ssp_sdmmc_detect(SDMMC_SSP(drive));
    else
        return true;
}

static void sdmmc_detect_callback(int ssp)
{
    /* This is called only if the state was stable for 300ms - check state
     * and post appropriate event. */
    long evid = imx233_ssp_sdmmc_detect(ssp) ?
                    SYS_HOTSWAP_INSERTED : SYS_HOTSWAP_EXTRACTED;

    /* Have to reverse lookup the ssp */
    for (unsigned drive = 0; drive < SDMMC_NUM_DRIVES; drive++)
    {
        if (SDMMC_SSP(drive) != ssp)
            continue;

        int first_drive, *map;
        switch (SDMMC_MODE(drive))
        {
#if (CONFIG_STORAGE & STORAGE_MMC)
        case MMC_MODE:
            first_drive = mmc_first_drive;
            map = mmc_map;
            break;
#endif
#if (CONFIG_STORAGE & STORAGE_SD)
        case SD_MODE:
            first_drive = sd_first_drive;
            map = sd_map;
            break;
#endif
        default:
            continue;
        }

        /* message requires logical drive number as data */
        queue_broadcast(evid, first_drive + map[drive]);
    }

    imx233_ssp_sdmmc_setup_detect(ssp, true, sdmmc_detect_callback, false,
        imx233_ssp_sdmmc_is_detect_inverted(ssp));
}

static void sdmmc_enable_pullups(int drive, bool pullup)
{
    /* setup pins, never use alternatives pin on SSP1 because no device use it
     * but this could be made a flag */
    int bus_width = SDMMC_MODE(drive) == MMC_MODE ? 8 : 4;
    if(SDMMC_SSP(drive) == 1)
        imx233_ssp_setup_ssp1_sd_mmc_pins(pullup, bus_width, false);
    else
        imx233_ssp_setup_ssp2_sd_mmc_pins(pullup, bus_width);
}

static void sdmmc_power(int drive, bool on)
{
    /* power chip if needed */
    if(SDMMC_FLAGS(drive) & POWER_PIN)
    {
        int bank = PIN2BANK(SDMMC_CONF(drive).power_pin);
        int pin = PIN2PIN(SDMMC_CONF(drive).power_pin);
        imx233_pinctrl_acquire(bank, pin, "sdmmc_power");
        imx233_pinctrl_set_function(bank, pin, PINCTRL_FUNCTION_GPIO);
        imx233_pinctrl_enable_gpio(bank, pin, true);
        if(SDMMC_FLAGS(drive) & POWER_INVERTED)
            imx233_pinctrl_set_gpio(bank, pin, !on);
        else
            imx233_pinctrl_set_gpio(bank, pin, on);
    }
    if(SDMMC_FLAGS(drive) & POWER_DELAY)
        sleep(SDMMC_CONF(drive).power_delay);
    /* enable pullups for identification */
    sdmmc_enable_pullups(drive, true);
}

#define MCI_NO_RESP     0
#define MCI_RESP        (1<<0)
#define MCI_LONG_RESP   (1<<1)
#define MCI_ACMD        (1<<2) /* sd only */
#define MCI_NOCRC       (1<<3)
#define MCI_BUSY        (1<<4)

static bool send_cmd(int drive, uint8_t cmd, uint32_t arg, uint32_t flags, uint32_t *resp)
{
    if((flags & MCI_ACMD) && !send_cmd(drive, SD_APP_CMD, SDMMC_RCA(drive), MCI_RESP, resp))
        return false;

    enum imx233_ssp_resp_t resp_type = (flags & MCI_LONG_RESP) ? SSP_LONG_RESP :
        (flags & MCI_RESP) ? SSP_SHORT_RESP : SSP_NO_RESP;
    enum imx233_ssp_error_t ret = imx233_ssp_sd_mmc_transfer(SDMMC_SSP(drive), cmd,
        arg, resp_type, NULL, 0, !!(flags & MCI_BUSY), false, resp);
    if(resp_type == SSP_LONG_RESP)
    {
        /* Our SD codes assume most significant word first, so reverse resp */
        uint32_t tmp = resp[0];
        resp[0] = resp[3];
        resp[3] = tmp;
        tmp = resp[1];
        resp[1] = resp[2];
        resp[2] = tmp;
    }
    return ret == SSP_SUCCESS;
}

static int wait_for_state(int drive, unsigned state)
{
    unsigned long response;
    unsigned int timeout = current_tick + 5*HZ;
    int cmd_retry = 10;
    int next_yield = current_tick + MIN_YIELD_PERIOD;

    while (1)
    {
        /* NOTE: rely on SD_SEND_STATUS=MMC_SEND_STATUS */
        while(!send_cmd(drive, SD_SEND_STATUS, SDMMC_RCA(drive), MCI_RESP, &response) && cmd_retry > 0)
            cmd_retry--;

        if(cmd_retry <= 0)
            return -1;

        if(((response >> 9) & 0xf) == state)
            return 0;

        if(TIME_AFTER(current_tick, timeout))
            return -10 * ((response >> 9) & 0xf);

        if(TIME_AFTER(current_tick, next_yield))
        {
            yield();
            next_yield = current_tick + MIN_YIELD_PERIOD;
        }
    }

    return 0;
}

#if CONFIG_STORAGE & STORAGE_SD
static int init_sd_card(int drive)
{
    int ssp = SDMMC_SSP(drive);
    sdmmc_power(drive, false);
    sdmmc_power(drive, true);
    imx233_ssp_start(ssp);
    imx233_ssp_softreset(ssp);
    imx233_ssp_set_mode(ssp, BV_SSP_CTRL1_SSP_MODE__SD_MMC);
    /* SSPCLK @ 96MHz
     * gives bitrate of 96000 / 240 / 1 = 400kHz */
    imx233_ssp_set_timings(ssp, 240, 0, 0xffff);
    imx233_ssp_sd_mmc_power_up_sequence(ssp);
    imx233_ssp_set_bus_width(ssp, 1);
    imx233_ssp_set_block_size(ssp, 9);

    SDMMC_RCA(drive) = 0;
    bool sd_v2 = false, sd_hs = false;
    uint32_t resp;
    long init_timeout;
    /* go to idle state */
    if(!send_cmd(drive, SD_GO_IDLE_STATE, 0, MCI_NO_RESP, NULL))
        return -1;
    /* CMD8 Check for v2 sd card.  Must be sent before using ACMD41
       Non v2 cards will not respond to this command */
    if(send_cmd(drive, SD_SEND_IF_COND, 0x1AA, MCI_RESP, &resp))
        if((resp & 0xFFF) == 0x1AA)
            sd_v2 = true;
    /* timeout for initialization is 1sec, from SD Specification 2.00 */
    init_timeout = current_tick + HZ;
    do
    {
        /* this timeout is the only valid error for this loop*/
        if(TIME_AFTER(current_tick, init_timeout))
            return -2;

        /* ACMD41 For v2 cards set HCS bit[30] & send host voltage range to all */
        if(!send_cmd(drive, SD_APP_OP_COND, (0x00FF8000 | (sd_v2 ? 1<<30 : 0)),
                MCI_ACMD|MCI_NOCRC|MCI_RESP, &SDMMC_INFO(drive).ocr))
            return -100;
    } while(!(SDMMC_INFO(drive).ocr & (1<<31)));

    /* CMD2 send CID */
    if(!send_cmd(drive, SD_ALL_SEND_CID, 0, MCI_RESP|MCI_LONG_RESP, SDMMC_INFO(drive).cid))
        return -3;

    /* CMD3 send RCA */
    if(!send_cmd(drive, SD_SEND_RELATIVE_ADDR, 0, MCI_RESP, &SDMMC_INFO(drive).rca))
        return -4;

    /* CMD9 send CSD */
    if(!send_cmd(drive, SD_SEND_CSD, SDMMC_RCA(drive), MCI_RESP|MCI_LONG_RESP,
            SDMMC_INFO(drive).csd))
        return -9;

    sd_parse_csd(&SDMMC_INFO(drive));
    window_start[drive] = 0;
    window_end[drive] = SDMMC_INFO(drive).numblocks;

    /* CMD7 w/rca: Select card to put it in TRAN state */
    if(!send_cmd(drive, SD_SELECT_CARD, SDMMC_RCA(drive), MCI_RESP, &resp))
        return -12;
    if(wait_for_state(drive, SD_TRAN))
        return -13;

    /* ACMD6: set bus width to 4-bit */
    if(!send_cmd(drive, SD_SET_BUS_WIDTH, 2, MCI_RESP|MCI_ACMD, &resp))
        return -15;
    /* ACMD42: disconnect the pull-up resistor on CD/DAT3 */
    if(!send_cmd(drive, SD_SET_CLR_CARD_DETECT, 0, MCI_RESP|MCI_ACMD, &resp))
        return -17;

    /* Switch to 4-bit */
    imx233_ssp_set_bus_width(ssp, 4);
    SDMMC_STATUS(drive).bus_width = 4;

    /* Try to switch V2 cards to HS timings, non HS seem to ignore this */
    if(sd_v2)
    {
        /* only transfer 64 bytes */
        imx233_ssp_set_block_size(ssp, /*log2(64)*/6);
        /* CMD6 switch to HS */
        if(imx233_ssp_sd_mmc_transfer(ssp, SD_SWITCH_FUNC, 0x80fffff1,
                SSP_SHORT_RESP, aligned_buffer[drive], 1, true, true, NULL))
            return -12;
        imx233_ssp_set_block_size(ssp, /*log2(512)*/9);
        if((aligned_buffer[drive][16] & 0xf) == 1)
            sd_hs = true;
    }

    /* probe for CMD23 support */
    SDMMC_STATUS(drive).has_sbc = false;
    /* ACMD51, only transfer 8 bytes */
    imx233_ssp_set_block_size(ssp, /*log2(8)*/3);
    if(send_cmd(drive, SD_APP_CMD, SDMMC_RCA(drive), MCI_RESP, &resp))
    {
        if(imx233_ssp_sd_mmc_transfer(ssp, SD_SEND_SCR, 0, SSP_SHORT_RESP,
            aligned_buffer[drive], 1, true, true, NULL) == SSP_SUCCESS)
        {
            if(aligned_buffer[drive][3] & 2)
                SDMMC_STATUS(drive).has_sbc = true;
        }
    }
    imx233_ssp_set_block_size(ssp, /*log2(512)*/9);

    /* SSPCLK @ 96MHz
     * gives bitrate of 96 / 4 / 1 = 24MHz
     * gives bitrate of 96 / 2 / 1 = 48MHz */
    SDMMC_STATUS(drive).hs_capable = sd_hs;
    SDMMC_STATUS(drive).hs_enabled = false;
    if(/*sd_hs*/false)
        imx233_ssp_set_timings(ssp, 2, 0, 0xffff);
    else
        imx233_ssp_set_timings(ssp, 4, 0, 0xffff);
    /* deselect card */
    if(!send_cmd(drive, SD_DESELECT_CARD, 0, MCI_NO_RESP, NULL))
        return -13;
    /* successfully initialised */
    SDMMC_INFO(drive).initialized = 1;

    return 0;
}

int sd_event(long id, intptr_t data)
{
    int rc = 0;

    switch (id)
    {
#ifdef HAVE_HOTSWAP
    case SYS_HOTSWAP_INSERTED:
    case SYS_HOTSWAP_EXTRACTED:;
        const int drive = sd_map[data];

        /* Skip non-removable drivers */
        if(!sdmmc_removable(drive))
        {
            rc = -1;
            break;
        }

        mutex_lock(&mutex[drive]); /* lock-out card activity */

        /* Force card init for new card, re-init for re-inserted one or
         * clear if the last attempt to init failed with an error. */
        SDMMC_INFO(drive).initialized = 0;

        if(id == SYS_HOTSWAP_INSERTED)
            rc = init_drive(drive);

        /* unlock card */
        mutex_unlock(&mutex[drive]);
        /* Access is now safe */
        break;
#endif /* HAVE_HOTSWAP */
    default:
        rc = storage_event_default_handler(id, data, sd_last_disk_activity(),
                                           STORAGE_SD);
        break;
    }

    return rc;
}
#endif /* CONFIG_STORAGE & STORAGE_SD */

#if CONFIG_STORAGE & STORAGE_MMC
static int init_mmc_drive(int drive)
{
    int ssp = SDMMC_SSP(drive);
    /* we can choose the RCA of mmc cards: pick drive. Following our convention,
     * .rca is actually RCA << 16 */
    SDMMC_RCA(drive) = drive << 16;

    sdmmc_power(drive, false);
    sdmmc_power(drive, true);
    imx233_ssp_start(ssp);
    imx233_ssp_softreset(ssp);
    imx233_ssp_set_mode(ssp, BV_SSP_CTRL1_SSP_MODE__SD_MMC);
    /* SSPCLK @ 96MHz
     * gives bitrate of 96000 / 240 / 1 = 400kHz */
    imx233_ssp_set_timings(ssp, 240, 0, 0xffff);
    imx233_ssp_sd_mmc_power_up_sequence(ssp);
    imx233_ssp_set_bus_width(ssp, 1);
    imx233_ssp_set_block_size(ssp, 9);
    /* go to idle state */
    if(!send_cmd(drive, MMC_GO_IDLE_STATE, 0, MCI_NO_RESP, NULL))
        return -1;
    /* send op cond until the card respond with busy bit set; it must complete within 1sec */
    unsigned timeout = current_tick + HZ;
    bool ret = false;
    do
    {
        uint32_t ocr;
        ret = send_cmd(drive, MMC_SEND_OP_COND, 0x40ff8000, MCI_RESP, &ocr);
        if(ret && ocr & (1 << 31))
            break;
    }while(!TIME_AFTER(current_tick, timeout));

    if(!ret)
        return -2;
    /* get CID */
    uint32_t cid[4];
    if(!send_cmd(drive, MMC_ALL_SEND_CID, 0, MCI_LONG_RESP, cid))
        return -3;
    /* Set RCA */
    uint32_t status;
    if(!send_cmd(drive, MMC_SET_RELATIVE_ADDR, SDMMC_RCA(drive), MCI_RESP, &status))
        return -4;
    /* Select card */
    if(!send_cmd(drive, MMC_SELECT_CARD, SDMMC_RCA(drive), MCI_RESP, &status))
        return -5;
    /* Check TRAN state */
    if(wait_for_state(drive, MMC_TRAN))
        return -6;
    /* Switch to 8-bit bus */
    if(!send_cmd(drive, MMC_SWITCH, 0x3b70200, MCI_RESP|MCI_BUSY, &status))
        return -8;
    /* switch error ? */
    if(status & 0x80)
        return -9;
    imx233_ssp_set_bus_width(ssp, 8);
    /* Switch to high speed mode */
    if(!send_cmd(drive, MMC_SWITCH, 0x3b90100,  MCI_RESP|MCI_BUSY, &status))
        return -10;
    /* switch error ?*/
    if(status & 0x80)
        return -11;
    /* SSPCLK @ 96MHz
     * gives bitrate of 96 / 2 / 1 = 48MHz */
    imx233_ssp_set_timings(ssp, 2, 0, 0xffff);
    SDMMC_STATUS(drive).bus_width = 8;
    SDMMC_STATUS(drive).hs_capable = true;
    SDMMC_STATUS(drive).hs_enabled = true;

    /* read extended CSD */
    {
        uint8_t *ext_csd = aligned_buffer[drive];
        if(imx233_ssp_sd_mmc_transfer(ssp, 8, 0, SSP_SHORT_RESP, aligned_buffer[drive], 1, true, true, &status))
            return -12;
        uint32_t *sec_count = (void *)&ext_csd[212];
        window_start[drive] = 0;
        window_end[drive] = *sec_count;
    }
    /* deselect card */
    if(!send_cmd(drive, MMC_DESELECT_CARD, 0, MCI_NO_RESP, NULL))
        return -13;

    /* MMC always support CMD23 */
    SDMMC_STATUS(drive).has_sbc = true;
    SDMMC_INFO(drive).initialized = 1;

    return 0;
}

int mmc_event(long id, intptr_t data)
{
    return storage_event_default_handler(id, data, mmc_last_disk_activity(),
                                         STORAGE_MMC);
}
#endif /* CONFIG_STORAGE & STORAGE_MMC */

/* low-level function, don't call directly! */
static int __xfer_sectors(int drive, sector_t start, int count, void *buf, bool read)
{
    uint32_t resp;
    int ret = 0;
    while(count != 0)
    {
        int this_count = MIN(count, IMX233_MAX_SINGLE_DMA_XFER_SIZE / 512);
        bool need_stop = true;
        if(SDMMC_STATUS(drive).has_sbc && send_cmd(drive, 23, this_count, MCI_RESP, &resp))
            need_stop = false;
        /* Set bank_start to the correct unit (blocks or bytes).
         * MMC drives use block addressing, SD cards bytes or blocks */
        sector_t bank_start = start;
        if(SDMMC_MODE(drive) == SD_MODE && !(SDMMC_INFO(drive).ocr & (1<<30)))   /* not SDHC */
            bank_start *= SD_BLOCK_SIZE;
        /* issue read/write
         * NOTE: rely on SD_{READ,WRITE}_MULTIPLE_BLOCK=MMC_{READ,WRITE}_MULTIPLE_BLOCK */
        ret = imx233_ssp_sd_mmc_transfer(SDMMC_SSP(drive),
            read ? SD_READ_MULTIPLE_BLOCK : SD_WRITE_MULTIPLE_BLOCK,
            bank_start, SSP_SHORT_RESP, buf, this_count, false, read, &resp);
        if(ret != SSP_SUCCESS)
            need_stop = true;
        /* stop transmission
         * NOTE: rely on SD_STOP_TRANSMISSION=MMC_STOP_TRANSMISSION */
        if(need_stop && !send_cmd(drive, SD_STOP_TRANSMISSION, 0, MCI_RESP|MCI_BUSY, &resp))
        {
            ret = -15;
            break;
        }
        if(ret != 0)
            return ret;
        count -= this_count;
        start += this_count;
        buf += this_count * 512;
    }
    return ret;
}

static int transfer_sectors(int drive, sector_t start, int count, void *buf, bool read)
{
    int ret = 0;
    /* the function doesn't work when count is 0 */
    if(count == 0)
        return ret;

    /* update disk activity */
    disk_last_activity[drive] = current_tick;

    /* lock per-drive mutex */
    mutex_lock(&mutex[drive]);

    /* update led status */
    led(true);

    /* for SD cards, init if necessary */
#if CONFIG_STORAGE & STORAGE_SD
    if(SDMMC_MODE(drive) == SD_MODE && SDMMC_INFO(drive).initialized <= 0)
    {
        ret = init_drive(drive);
        if(SDMMC_INFO(drive).initialized <= 0)
            goto Lend;
    }
#endif

    /* check window */
    start += window_start[drive];
    if((start + count) > window_end[drive])
    {
        ret = -201;
        goto Lend;
    }
    /* select card.
     * NOTE: rely on SD_SELECT_CARD=MMC_SELECT_CARD */
    if(!send_cmd(drive, SD_SELECT_CARD, SDMMC_RCA(drive), MCI_NO_RESP, NULL))
    {
        ret = -20;
        goto Lend;
    }
    /* wait for TRAN state */
    /* NOTE: rely on SD_TRAN=MMC_TRAN */
    ret = wait_for_state(drive, SD_TRAN);
    if(ret < 0)
        goto Ldeselect;

    /**
     * NOTE: we need to make sure dma transfers are aligned. This is handled
     * differently for read and write transfers. We do not repeat it each
     * time but it should be noted that all transfers are limited by
     * IMX233_MAX_SINGLE_DMA_XFER_SIZE and thus need to be split if needed.
     *
     * Read transfers:
     *   If the buffer is already aligned, transfer everything at once.
     *   Otherwise, transfer all sectors but one to the sub-buffer starting
     *   on the next cache line and then move the data. Then transfer the
     *   last sector to the aligned_buffer and then copy to the buffer.
     *
     * Write transfers:
     *   If the buffer is already aligned, transfer everything at once.
     *   Otherwise, copy the first sector to the aligned_buffer and transfer.
     *   Then move all other sectors within the buffer to make it cache
     *   aligned and transfer it. Then move data to pretend the buffer was
     *   never modified.
     */
    if(read)
    {
        void *ptr = CACHEALIGN_UP(buf);
        if(buf != ptr)
        {
            /* copy count-1 sector and then move within the buffer */
            ret = __xfer_sectors(drive, start, count - 1, ptr, read);
            memmove(buf, ptr, 512 * (count - 1));
            if(ret >= 0)
            {
                /* transfer the last sector the aligned_buffer and copy */
                ret = __xfer_sectors(drive, start + count - 1, 1,
                    aligned_buffer[drive], read);
                memcpy(buf + 512 * (count - 1), aligned_buffer[drive], 512);
            }
        }
        else
            ret = __xfer_sectors(drive, start, count, buf, read);
    }
    else
    {
        void *ptr = CACHEALIGN_UP(buf);
        if(buf != ptr)
        {
            /* transfer the first sector to aligned_buffer and copy */
            memcpy(aligned_buffer[drive], buf, 512);
            ret = __xfer_sectors(drive, start, 1, aligned_buffer[drive], read);
            if(ret >= 0)
            {
                /* move within the buffer and transfer */
                memmove(ptr, buf + 512, 512 * (count - 1));
                ret = __xfer_sectors(drive, start + 1, count - 1, ptr, read);
                /* move back */
                memmove(buf + 512, ptr, 512 * (count - 1));
                memcpy(buf, aligned_buffer[drive], 512);
            }
        }
        else
            ret = __xfer_sectors(drive, start, count, buf, read);
    }
    /* deselect card */
    Ldeselect:
    /*  CMD7 w/rca =0 : deselects card & puts it in STBY state
     * NOTE: rely on SD_DESELECT_CARD=MMC_DESELECT_CARD */
    if(!send_cmd(drive, SD_DESELECT_CARD, 0, MCI_NO_RESP, NULL))
        ret = -23;
    Lend:
    /* update led status */
    led(false);
    /* release per-drive mutex */
    mutex_unlock(&mutex[drive]);
    return ret;
}

/* user specifies the sdmmc drive */
static int part_read_fn(intptr_t user, sector_t start, int count, void* buf)
{
    return transfer_sectors(user, start, count, buf, true);
}

static int init_drive(int drive)
{
    int ret;
    switch(SDMMC_MODE(drive))
    {
#if CONFIG_STORAGE & STORAGE_SD
        case SD_MODE: ret = init_sd_card(drive); break;
#endif
#if CONFIG_STORAGE & STORAGE_MMC
        case MMC_MODE: ret = init_mmc_drive(drive); break;
#endif
        default: ret = 0;
    }
    if(ret < 0)
        return ret;

    /* compute window */
    if((SDMMC_FLAGS(drive) & WINDOW) && imx233_partitions_is_window_enabled())
    {
        /* NOTE: at this point the window shows the whole disk so raw disk
         * accesses can be made to lookup partitions */
        ret = imx233_partitions_compute_window(IF_MD_DRV(drive), part_read_fn,
            IMX233_PART_USER, &window_start[drive], &window_end[drive]);
        if(ret)
            panicf("cannot compute partitions window: %d", ret);
        SDMMC_INFO(drive).numblocks = window_end[drive] - window_start[drive];
    }

    return 0;
}

static int sdmmc_init(void)
{
    static int is_initialized = false;
    if(is_initialized)
        return 0;
    is_initialized = true;
    for(unsigned drive = 0; drive < SDMMC_NUM_DRIVES; drive++)
        mutex_init(&mutex[drive]);

    for(unsigned drive = 0; drive < SDMMC_NUM_DRIVES; drive++)
    {
        if(sdmmc_removable(drive))
            imx233_ssp_sdmmc_setup_detect(SDMMC_SSP(drive), true, sdmmc_detect_callback,
                false, SDMMC_FLAGS(drive) & DETECT_INVERTED);
    }

    return 0;
}

#if CONFIG_STORAGE & STORAGE_SD
int sd_init(void)
{
    int ret = sdmmc_init();
    if(ret < 0) return ret;

    _sd_num_drives = 0;
    for(unsigned drive = 0; drive < SDMMC_NUM_DRIVES; drive++)
        if(SDMMC_MODE(drive) == SD_MODE)
        {
            /* if asked to probe, try to init it and ignore it if it fails */
            if(SDMMC_FLAGS(drive) & PROBE)
            {
                int ret = init_drive(drive);
                if(ret < 0)
                    continue;
            }
            sd_map[_sd_num_drives++] = drive;
        }
    return 0;
}

tCardInfo *card_get_info_target(int sd_card_no)
{
    return &SDMMC_INFO(sd_map[sd_card_no]);
}

int sd_num_drives(int first_drive)
{
    sd_first_drive = first_drive;
    return _sd_num_drives;
}

bool sd_present(IF_MD_NONVOID(int sd_drive))
{
    return sdmmc_present(sd_map[IF_MD_DRV(sd_drive)]);
}

bool sd_removable(IF_MD_NONVOID(int sd_drive))
{
    return sdmmc_removable(sd_map[IF_MD_DRV(sd_drive)]);
}

long sd_last_disk_activity(void)
{
    long last = 0;
    for(unsigned i = 0; i < _sd_num_drives; i++)
        last = MAX(last, disk_last_activity[sd_map[i]]);
    return last;
}

void sd_enable(bool on)
{
    (void) on;
}

int sd_read_sectors(IF_MD(int sd_drive,) sector_t start, int count, void *buf)
{
#ifndef HAVE_MULTIDRIVE
    int sd_drive = 0;
#endif
    return transfer_sectors(sd_map[sd_drive], start, count, buf, true);
}

int sd_write_sectors(IF_MD(int sd_drive,) sector_t start, int count, const void* buf)
{
#ifndef HAVE_MULTIDRIVE
    int sd_drive = 0;
#endif
    return transfer_sectors(sd_map[sd_drive], start, count, (void *)buf, false);
}
#endif

#if CONFIG_STORAGE & STORAGE_MMC
int mmc_init(void)
{
    int ret = sdmmc_init();
    if(ret < 0) return ret;

    _mmc_num_drives = 0;
    for(unsigned drive = 0; drive < SDMMC_NUM_DRIVES; drive++)
        if(SDMMC_MODE(drive) == MMC_MODE)
        {
            /* try to init drive, panic on failure or skip if probing */
            int ret = init_drive(drive);
            if(ret < 0)
            {
                if(SDMMC_FLAGS(drive) & PROBE)
                    continue;
                else
                    panicf("init_drive(%d) failed: %d (mmc)", drive, ret);
            }
            mmc_map[_mmc_num_drives++] = drive;
        }
    return 0;
}

void mmc_get_info(IF_MD(int mmc_drive,) struct storage_info *info)
{
#ifndef HAVE_MULTIDRIVE
    int mmc_drive = 0;
#endif
    int drive = mmc_map[mmc_drive];
    info->sector_size = 512;
    info->num_sectors = window_end[drive] - window_start[drive];
    info->vendor = "Rockbox";
    info->product = "Internal Storage";
    info->revision = "0.00";
}

int mmc_num_drives(int first_drive)
{
    mmc_first_drive = first_drive;
    return _mmc_num_drives;
}

bool mmc_present(IF_MD_NONVOID(int mmc_drive))
{
    return sdmmc_present(mmc_map[IF_MD_DRV(mmc_drive)]);
}

bool mmc_removable(IF_MD_NONVOID(int mmc_drive))
{
    return sdmmc_removable(mmc_map[IF_MD_DRV(mmc_drive)]);
}

long mmc_last_disk_activity(void)
{
    long last = 0;
    for(unsigned i = 0; i < _mmc_num_drives; i++)
        last = MAX(last, disk_last_activity[mmc_map[i]]);
    return last;
}

void mmc_enable(bool on)
{
    (void) on;
}

void mmc_sleepnow(void)
{
}

bool mmc_disk_is_active(void)
{
    return false;
}

bool mmc_usb_active(int delayticks)
{
    (void) delayticks;
    return mmc_disk_is_active();
}

int mmc_soft_reset(void)
{
    return 0;
}

int mmc_flush(void)
{
    return 0;
}

void mmc_spin(void)
{
}

void mmc_spindown(int seconds)
{
    (void) seconds;
}

int mmc_spinup_time(void)
{
    return 0;
}

int mmc_read_sectors(IF_MD(int mmc_drive,) sector_t start, int count, void *buf)
{
#ifndef HAVE_MULTIDRIVE
    int mmc_drive = 0;
#endif
    return transfer_sectors(mmc_map[mmc_drive], start, count, buf, true);
}

int mmc_write_sectors(IF_MD(int mmc_drive,) sector_t start, int count, const void* buf)
{
#ifndef HAVE_MULTIDRIVE
    int mmc_drive = 0;
#endif
    return transfer_sectors(mmc_map[mmc_drive], start, count, (void *)buf, false);
}

tCardInfo *mmc_card_info(int card_no)
{
    return &SDMMC_INFO(mmc_map[card_no]);
}

#endif

/** Information about SD/MMC slot */
struct sdmmc_info_t
{
    int drive; /* drive number (for queries like storage_removable(drive) */
    const char *slot_name; /* name of the slot: 'internal' or 'microsd' */
    bool window; /* is window enabled for this slot? */
    int bus_width; /* current bus width */
    bool hs_capable; /* is device high-speed capable? */
    bool hs_enabled; /* is high-speed enabled? */
    bool has_sbc; /* device support SET_BLOCK_COUNT */
};

struct sdmmc_info_t imx233_sdmmc_get_info(int drive, int storage_drive)
{
    struct sdmmc_info_t info;
    memset(&info, 0, sizeof(info));
    info.drive = storage_drive;
    info.slot_name = SDMMC_CONF(drive).name;
    info.window = !!(SDMMC_CONF(drive).flags & WINDOW);
    info.bus_width = SDMMC_STATUS(drive).bus_width;
    info.hs_capable = SDMMC_STATUS(drive).hs_capable;
    info.hs_enabled = SDMMC_STATUS(drive).hs_enabled;
    info.has_sbc = SDMMC_STATUS(drive).has_sbc;
    return info;
}

#if CONFIG_STORAGE & STORAGE_SD
/* return information about a particular sd device (use regular drive number) */
struct sdmmc_info_t imx233_sd_get_info(int card_no)
{
    return imx233_sdmmc_get_info(sd_map[card_no], sd_first_drive + card_no);
}
#endif

#if CONFIG_STORAGE & STORAGE_MMC
/* return information about a particular mmc device (use regular drive number) */
struct sdmmc_info_t imx233_mmc_get_info(int card_no)
{
    return imx233_sdmmc_get_info(mmc_map[card_no], mmc_first_drive + card_no);
}
#endif
