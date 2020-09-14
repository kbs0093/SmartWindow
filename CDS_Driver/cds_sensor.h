#ifndef cds_censor_H
#define cds_censor_H

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

#define ARRAY_SIZE(array) sizeof(array) / sizeof(array[0])
#define MCP3008_RX_DATA_00      0x3FF  
#define MCP3008_RX_WORD_01      0x00
#define MCP3008_RX_NULLBYTE_02  0x04
#define MCP3008_RX_DATA_02      0x03
#define MCP3008_RX_DATA_03      0xFF

static const char *DEVICE = "/dev/spidev0.0";
static uint8_t MODE = SPI_MODE_0;
static uint8_t BITS = 8;
static uint32_t CLOCK = 1000000;
static uint16_t DELAY = 5;


int readadc(uint8_t channel);

#endif