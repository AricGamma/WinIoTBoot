/** @file
*
*  Copyright (c) 2007-2014, Allwinner Technology Co., Ltd. All rights reserved.
*  http://www.allwinnertech.com
*
*  Martin.Zheng <martinzheng@allwinnertech.com>
*  
*  This program and the accompanying materials                          
*  are licensed and made available under the terms and conditions of the BSD License         
*  which accompanies this distribution.  The full text of the license may be found at        
*  http://opensource.org/licenses/bsd-license.php                                            
*
*  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
*  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             
*
**/
#ifndef _MMC_H_
#define _MMC_H_

#define SD_VERSION_SD 0x20000
#define SD_VERSION_2  (SD_VERSION_SD | 0x20)
#define SD_VERSION_1_0  (SD_VERSION_SD | 0x10)
#define SD_VERSION_1_10 (SD_VERSION_SD | 0x1a)
#define MMC_VERSION_MMC   0x10000
#define MMC_VERSION_UNKNOWN (MMC_VERSION_MMC)
#define MMC_VERSION_1_2   (MMC_VERSION_MMC | 0x12)
#define MMC_VERSION_1_4   (MMC_VERSION_MMC | 0x14)
#define MMC_VERSION_2_2   (MMC_VERSION_MMC | 0x22)
#define MMC_VERSION_3   (MMC_VERSION_MMC | 0x30)
#define MMC_VERSION_4   (MMC_VERSION_MMC | 0x40)
#define MMC_VERSION_4_1   (MMC_VERSION_MMC | 0x41)
#define MMC_VERSION_4_2   (MMC_VERSION_MMC | 0x42)
#define MMC_VERSION_4_3   (MMC_VERSION_MMC | 0x43)
#define MMC_VERSION_4_41  (MMC_VERSION_MMC | 0x44)
#define MMC_VERSION_4_5   (MMC_VERSION_MMC | 0x45)
#define MMC_VERSION_5_0   (MMC_VERSION_MMC | 0x50)




#define MMC_MODE_HS   0x001
#define MMC_MODE_HS_52MHz 0x010
#define MMC_MODE_4BIT   0x100
#define MMC_MODE_8BIT   0x200
#define MMC_MODE_SPI    0x400
#define MMC_MODE_HC   0x800

#define SD_DATA_4BIT  0x00040000

#define IS_SD(x) (x->version & SD_VERSION_SD)

#define MMC_DATA_READ   1
#define MMC_DATA_WRITE    2

#define NO_CARD_ERR   -16 /* No SD/MMC card inserted */
#define UNUSABLE_ERR    -17 /* Unusable Card */
#define COMM_ERR    -18 /* Communications Error */
#define TIMEOUT     -19

#define MMC_CMD_GO_IDLE_STATE   0
#define MMC_CMD_SEND_OP_COND    1
#define MMC_CMD_ALL_SEND_CID    2
#define MMC_CMD_SET_RELATIVE_ADDR 3
#define MMC_CMD_SET_DSR     4
#define MMC_CMD_SWITCH      6
#define MMC_CMD_SELECT_CARD   7
#define MMC_CMD_SEND_EXT_CSD    8
#define MMC_CMD_SEND_CSD    9
#define MMC_CMD_SEND_CID    10
#define MMC_CMD_STOP_TRANSMISSION 12
#define MMC_CMD_SEND_STATUS   13
#define MMC_CMD_SET_BLOCKLEN    16
#define MMC_CMD_READ_SINGLE_BLOCK 17
#define MMC_CMD_READ_MULTIPLE_BLOCK 18
#define MMC_CMD_WRITE_SINGLE_BLOCK  24
#define MMC_CMD_WRITE_MULTIPLE_BLOCK  25
#define MMC_CMD_ERASE_GROUP_START 35
#define MMC_CMD_ERASE_GROUP_END   36
#define MMC_CMD_ERASE     38
#define MMC_CMD_APP_CMD     55
#define MMC_CMD_SPI_READ_OCR    58
#define MMC_CMD_SPI_CRC_ON_OFF    59

#define SD_CMD_SEND_RELATIVE_ADDR 3
#define SD_CMD_SWITCH_FUNC    6
#define SD_CMD_SEND_IF_COND   8

#define SD_CMD_APP_SET_BUS_WIDTH  6
#define SD_CMD_ERASE_WR_BLK_START 32
#define SD_CMD_ERASE_WR_BLK_END   33
#define SD_CMD_APP_SEND_OP_COND   41
#define SD_CMD_APP_SEND_SCR   51

/* SCR definitions in different words */
#define SD_HIGHSPEED_BUSY 0x00020000
#define SD_HIGHSPEED_SUPPORTED  0x00020000

#define MMC_HS_TIMING   0x00000100
#define MMC_HS_52MHZ    0x2

#define OCR_BUSY    0x80000000
#define OCR_HCS     0x40000000
#define OCR_VOLTAGE_MASK  0x007FFF80
#define OCR_ACCESS_MODE   0x60000000

#define SECURE_ERASE    0x80000000

#define MMC_STATUS_MASK   (~0x0206BF7F)
#define MMC_STATUS_RDY_FOR_DATA (1 << 8)
#define MMC_STATUS_CURR_STATE (0xf << 9)
#define MMC_STATUS_ERROR  (1 << 19)

#define MMC_VDD_165_195   0x00000080  /* VDD voltage 1.65 - 1.95 */
#define MMC_VDD_20_21   0x00000100  /* VDD voltage 2.0 ~ 2.1 */
#define MMC_VDD_21_22   0x00000200  /* VDD voltage 2.1 ~ 2.2 */
#define MMC_VDD_22_23   0x00000400  /* VDD voltage 2.2 ~ 2.3 */
#define MMC_VDD_23_24   0x00000800  /* VDD voltage 2.3 ~ 2.4 */
#define MMC_VDD_24_25   0x00001000  /* VDD voltage 2.4 ~ 2.5 */
#define MMC_VDD_25_26   0x00002000  /* VDD voltage 2.5 ~ 2.6 */
#define MMC_VDD_26_27   0x00004000  /* VDD voltage 2.6 ~ 2.7 */
#define MMC_VDD_27_28   0x00008000  /* VDD voltage 2.7 ~ 2.8 */
#define MMC_VDD_28_29   0x00010000  /* VDD voltage 2.8 ~ 2.9 */
#define MMC_VDD_29_30   0x00020000  /* VDD voltage 2.9 ~ 3.0 */
#define MMC_VDD_30_31   0x00040000  /* VDD voltage 3.0 ~ 3.1 */
#define MMC_VDD_31_32   0x00080000  /* VDD voltage 3.1 ~ 3.2 */
#define MMC_VDD_32_33   0x00100000  /* VDD voltage 3.2 ~ 3.3 */
#define MMC_VDD_33_34   0x00200000  /* VDD voltage 3.3 ~ 3.4 */
#define MMC_VDD_34_35   0x00400000  /* VDD voltage 3.4 ~ 3.5 */
#define MMC_VDD_35_36   0x00800000  /* VDD voltage 3.5 ~ 3.6 */

#define MMC_SWITCH_MODE_CMD_SET   0x00 /* Change the command set */
#define MMC_SWITCH_MODE_SET_BITS  0x01 /* Set bits in EXT_CSD byte
            addressed by index which are
            1 in value field */
#define MMC_SWITCH_MODE_CLEAR_BITS  0x02 /* Clear bits in EXT_CSD byte
            addressed by index, which are
            1 in value field */
#define MMC_SWITCH_MODE_WRITE_BYTE  0x03 /* Set target byte to value */

#define SD_SWITCH_CHECK   0
#define SD_SWITCH_SWITCH  1

/*
 * EXT_CSD fields
 */

#define EXT_CSD_BOOT_BUS_COND 177 /* R/W */
#define EXT_CSD_PART_CONF 179 /* R/W */
#define EXT_CSD_BUS_WIDTH 183 /* R/W */
#define EXT_CSD_HS_TIMING 185 /* R/W */
#define EXT_CSD_CARD_TYPE 196 /* RO */
#define EXT_CSD_REV   192 /* RO */
#define EXT_CSD_SEC_CNT   212 /* RO, 4 bytes */

/*
 * EXT_CSD field definitions
 */

#define EXT_CSD_CMD_SET_NORMAL    (1 << 0)
#define EXT_CSD_CMD_SET_SECURE    (1 << 1)
#define EXT_CSD_CMD_SET_CPSECURE  (1 << 2)

#define EXT_CSD_CARD_TYPE_26  (1 << 0)  /* Card can run at 26MHz */
#define EXT_CSD_CARD_TYPE_52  (1 << 1)  /* Card can run at 52MHz */

#define EXT_CSD_BUS_WIDTH_1 0 /* Card is in 1 bit mode */
#define EXT_CSD_BUS_WIDTH_4 1 /* Card is in 4 bit mode */
#define EXT_CSD_BUS_WIDTH_8 2 /* Card is in 8 bit mode */




/* MMC_SWITCH boot modes */
#define MMC_SWITCH_MMCPART_NOAVAILABLE  (0xff)
#define MMC_SWITCH_PART_ACCESS_MASK   (0x7)
#define MMC_SWITCH_PART_SUPPORT     (0x1)
#define MMC_SWITCH_PART_BOOT_PART_MASK  (0x7 << 3)
#define MMC_SWITCH_PART_BOOT_PART_NONE  (0x0)
#define MMC_SWITCH_PART_BOOT_PART_1   (0x1)
#define MMC_SWITCH_PART_BOOT_PART_2   (0x2)
#define MMC_SWITCH_PART_BOOT_USER   (0x7)
#define MMC_SWITCH_PART_BOOT_ACK_MASK (0x1 << 6)
#define MMC_SWITCH_PART_BOOT_ACK_ENB  (0x1)

/* MMC_SWITCH boot condition */
#define MMC_SWITCH_MMCBOOT_BUS_NOAVAILABLE  (0xff)
#define MMC_SWITCH_BOOT_MODE_MASK     (0x3 << 3)
#define MMC_SWITCH_BOOT_SDR_NORMAL      (0x0)
#define MMC_SWITCH_BOOT_SDR_HS        (0x1)
#define MMC_SWITCH_BOOT_DDR         (0x2)
#define MMC_SWITCH_BOOT_RST_BUS_COND_MASK (0x1 << 2)
#define MMC_SWITCH_BOOT_RST_BUS_COND    (0x0)
#define MMC_SWITCH_BOOT_RETAIN_BUS_COND   (0x1)
#define MMC_SWITCH_BOOT_BUS_WIDTH_MASK    (0x3 << 0)
#define MMC_SWITCH_BOOT_BUS_SDRx1_DDRx4   (0x0)
#define MMC_SWITCH_BOOT_BUS_SDRx4_DDRx4   (0x1)
#define MMC_SWITCH_BOOT_BUS_SDRx8_DDRx8   (0x2)






#define R1_ILLEGAL_COMMAND    (1 << 22)
#define R1_APP_CMD      (1 << 5)

#define MMC_RSP_PRESENT (1 << 0)
#define MMC_RSP_136 (1 << 1)    /* 136 bit response */
#define MMC_RSP_CRC (1 << 2)    /* expect valid crc */
#define MMC_RSP_BUSY  (1 << 3)    /* card may send busy */
#define MMC_RSP_OPCODE  (1 << 4)    /* response contains opcode */

#define MMC_RSP_NONE  (0)
#define MMC_RSP_R1  (MMC_RSP_PRESENT|MMC_RSP_CRC|MMC_RSP_OPCODE)
#define MMC_RSP_R1b (MMC_RSP_PRESENT|MMC_RSP_CRC|MMC_RSP_OPCODE| \
      MMC_RSP_BUSY)
#define MMC_RSP_R2  (MMC_RSP_PRESENT|MMC_RSP_136|MMC_RSP_CRC)
#define MMC_RSP_R3  (MMC_RSP_PRESENT)
#define MMC_RSP_R4  (MMC_RSP_PRESENT)
#define MMC_RSP_R5  (MMC_RSP_PRESENT|MMC_RSP_CRC|MMC_RSP_OPCODE)
#define MMC_RSP_R6  (MMC_RSP_PRESENT|MMC_RSP_CRC|MMC_RSP_OPCODE)
#define MMC_RSP_R7  (MMC_RSP_PRESENT|MMC_RSP_CRC|MMC_RSP_OPCODE)

#define IF_TYPE_MMC   6

#define MMCPART_NOAVAILABLE (0xff)
#define PART_ACCESS_MASK  (0x7)
#define PART_SUPPORT    (0x1)


#define USER_PART   0
#define BOOT1_PART  1
#define BOOT2_PART  2


struct mmc_cid {
  unsigned long psn;
  unsigned short oid;
  unsigned char mid;
  unsigned char prv;
  unsigned char mdt;
  char pnm[7];
};

/*
 * WARNING!
 *
 * This structure is used by atmel_mci.c only.
 * It works for the AVR32 architecture but NOT
 * for ARM/AT91 architectures.
 * Its use is highly depreciated.
 * After the atmel_mci.c driver for AVR32 has
 * been replaced this structure will be removed.
 */
struct mmc_csd
{
  u8  csd_structure:2,
    spec_vers:4,
    rsvd1:2;
  u8  taac;
  u8  nsac;
  u8  tran_speed;
  u16 ccc:12,
    read_bl_len:4;
  u64 read_bl_partial:1,
    write_blk_misalign:1,
    read_blk_misalign:1,
    dsr_imp:1,
    rsvd2:2,
    c_size:12,
    vdd_r_curr_min:3,
    vdd_r_curr_max:3,
    vdd_w_curr_min:3,
    vdd_w_curr_max:3,
    c_size_mult:3,
    sector_size:5,
    erase_grp_size:5,
    wp_grp_size:5,
    wp_grp_enable:1,
    default_ecc:2,
    r2w_factor:3,
    write_bl_len:4,
    write_bl_partial:1,
    rsvd3:5;
  u8  file_format_grp:1,
    copy:1,
    perm_write_protect:1,
    tmp_write_protect:1,
    file_format:2,
    ecc:2;
  u8  crc:7;
  u8  one:1;
};

struct mmc_cmd {
  unsigned cmdidx;
  unsigned resp_type;
  unsigned cmdarg;
  unsigned response[4];
  unsigned flags;
};

struct mmc_data {
  union {
    char *dest;
    const char *src; /* src buffers don't get written to */
  };
  unsigned flags;
  unsigned blocks;
  unsigned blocksize;
};

typedef struct block_dev_desc {
  int   if_type;  /* type of the interface */
  int   dev;    /* device number */
  unsigned char part_type;  /* partition type */
  unsigned char target;   /* target SCSI ID */
  unsigned char lun;    /* target LUN */
  unsigned char type;   /* device type */
  unsigned char removable;  /* removable device */
#ifdef CONFIG_LBA48
  unsigned char lba48;    /* device can use 48bit addr (ATA/ATAPI v7) */
#endif
  unsigned long lba;    /* number of blocks */
  unsigned long blksz;    /* block size */
  char    vendor [40+1];  /* IDE model, SCSI Vendor */
  char    product[20+1];  /* IDE Serial no, SCSI product */
  char    revision[8+1];  /* firmware revision */
  unsigned long (*block_read)(int dev,
              unsigned long start,
              unsigned long blkcnt,
              void *buffer);
  unsigned long (*block_int_read)(int dev,
              unsigned long start,
              unsigned long blkcnt,
              void *buffer);
  unsigned long (*block_write)(int dev,
               unsigned long start,
               unsigned long blkcnt,
               const void *buffer);
  unsigned long   (*block_erase)(int dev,
               unsigned long start,
               unsigned long blkcnt);
  unsigned long  (*block_read_mass_pro)(int dev,
              unsigned long start,
              unsigned long blkcnt,
              void *buffer);  
  unsigned long (*block_write_mass_pro)(int dev,
               unsigned long start,
               unsigned long blkcnt,
               const void *buffer); 
  void    *priv;    /* driver private struct pointer */
}block_dev_desc_t;

struct mmc {
  char name[32];
  void *priv;
  unsigned voltages;
  unsigned version;
  volatile unsigned has_init;
  unsigned control_num;
  unsigned f_min;
  unsigned f_max;
  int high_capacity;
  unsigned bus_width;
  unsigned clock;
  unsigned card_caps;
  unsigned host_caps;
  unsigned ocr;
  unsigned scr[2];
  unsigned csd[4];
  unsigned cid[4];
  unsigned short rca;
  char part_config;
  char part_num;
  char part_support;
  char boot_support;
  unsigned tran_speed;
  unsigned read_bl_len;
  unsigned write_bl_len;
  unsigned erase_grp_size;
  unsigned long long capacity;
  block_dev_desc_t block_dev;
  int (*send_cmd)(struct mmc *mmc,
      struct mmc_cmd *cmd, struct mmc_data *data);
  void (*set_ios)(struct mmc *mmc);
  int (*init)(struct mmc *mmc);
  unsigned b_max;
    unsigned lba;        /* number of blocks */
    unsigned user_lba;
    unsigned boot1_lba;
    unsigned boot2_lba;
    unsigned blksz;      /* block size */
  unsigned char boot_bus_cond;
};


#define mmc_host_is_spi(mmc)  ((mmc)->host_caps & MMC_MODE_SPI)

int mmc_register(int dev_num,struct mmc *mmc);
int mmc_unregister(int dev_num);
struct mmc *find_mmc_device(int dev_num);
unsigned long mmc_berase(int dev_num, unsigned long start, unsigned long blkcnt);
unsigned long mmc_bwrite(int dev_num, unsigned long start, unsigned long blkcnt, const void*src);
unsigned long mmc_bread(int dev_num, unsigned long start, unsigned long blkcnt, void *dst);

int mmc_switch_boot_bus_cond(int dev_num, unsigned int boot_mode, unsigned int rst_bus_cond, unsigned int bus_width);
int mmc_switch_boot_part(int dev_num, unsigned int boot_ack, unsigned int boot_part);
int mmc_switch_part(int dev_num, unsigned int part_num);

#endif /* _MMC_H_ */
