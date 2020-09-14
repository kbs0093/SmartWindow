#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include "water_sensor.h"

#define ARRAY_SIZE(array) sizeof(array) / sizeof(array[0])

static const char *DEVICE = "/dev/spidev0.0"; //We use spidev for ADC
static uint8_t MODE = SPI_MODE_0;
static uint8_t BITS = 8;
static uint32_t CLOCK = 1000000;
static uint16_t DELAY = 5;

/*
 * Ensure all settings are correct for the ADC
 */
static int prepare(int fd) {

  if (ioctl(fd, SPI_IOC_WR_MODE, &MODE) == -1) {
    perror("Can't set MODE");
    return -1;
  }

  if (ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &BITS) == -1) {
    perror("Can't set number of BITS");
    return -1;
  }

  if (ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &CLOCK) == -1) {
    perror("Can't set write CLOCK");
    return -1;
  }

  if (ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &CLOCK) == -1) {
    perror("Can't set read CLOCK");
    return -1;
  }

  return 0;
}

/*
 * (SGL/DIF = 0, D2=D1=D0=0)
 */ 
uint8_t control_bits_differential(uint8_t channel) {
  return (channel & 7) << 4;
}

/*
 * (SGL/DIF = 1, D2=D1=D0=0)
 */ 
uint8_t control_bits(uint8_t channel) {
  return 0x8 | control_bits_differential(channel);
}

 
int readadc_water() { // Function that we can use in Main app effectively.
  uint8_t tx[] = {1, 8, 0};
  uint8_t rx[3];
     
  int fd = open(DEVICE, O_RDWR); // Open device
  
  if (fd <= 0) {
    printf("Water Device %s not found\n", DEVICE);
    return -1;
  }
  
  struct spi_ioc_transfer tr = {
    .tx_buf = (unsigned long)tx,
    .rx_buf = (unsigned long)rx,
    .len = ARRAY_SIZE(tx),
    .delay_usecs = DELAY,
    .speed_hz = CLOCK,
    .bits_per_word = BITS, 
  };

  if (ioctl(fd, SPI_IOC_MESSAGE(1), &tr) == 1) { //SPI_IOC_MESSAGE is macro that kind of '_IOW' in spidev.
    perror("IO Error");
    close(fd);
    abort();
  }
  
  close(fd);

  return ((rx[1] << 8) & 0x300) | (rx[2] & 0xFF); // return last 3 bit of rx[1] and all of rx[2]
}