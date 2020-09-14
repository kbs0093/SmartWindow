#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include "dust_sensor.h"

#define ARRAY_SIZE(array) sizeof(array) / sizeof(array[0])

#define IOCTL_MAGIC_NUMBER 'd'
#define IOCTL_DUST_ON         _IO(IOCTL_MAGIC_NUMBER, 0)
#define IOCTL_DUST_OFF        _IO(IOCTL_MAGIC_NUMBER, 1)

static const char *DEVICE = "/dev/spidev0.0";
static uint8_t MODE = SPI_MODE_0;
static uint8_t BITS = 8;
static uint32_t CLOCK = 1000000;
static uint16_t DELAY = 5;

/*
 * Ensure all settings are correct for the ADC
 */
static int prepare2(int fd) {

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
uint8_t control_bits_differential2(uint8_t channel) {
  return (channel & 7) << 4;
}

/*
 * (SGL/DIF = 1, D2=D1=D0=0)
 */ 
uint8_t control_bits2(uint8_t channel) {
  return 0x8 | control_bits_differential2(channel);
}

/*
 * Fetch the raw ADC value for the given channel.
 */
int readadc(int fd, uint8_t channel) {
  uint8_t tx[] = {1, control_bits2(channel), 0};
  uint8_t rx[3];

  struct spi_ioc_transfer tr = {
    .tx_buf = (unsigned long)tx,
    .rx_buf = (unsigned long)rx,
    .len = ARRAY_SIZE(tx),
    .delay_usecs = DELAY,
    .speed_hz = CLOCK,
    .bits_per_word = BITS,
  };

  if (ioctl(fd, SPI_IOC_MESSAGE(1), &tr) == 1) {
    perror("IO Error");
    abort();
  }

  // the value what we want is in the last 3 bit of rx[1] and all of rx[2]
  return ((rx[1] << 8) & 0x300) | (rx[2] & 0xFF);
}


int dust_sensor(){

  int res_ana = -1;
  float res_dig = -1;
  float res_dust = -1;

  int spi_fd = open(DEVICE, O_RDWR);
  int dust_fd = open("/dev/dust_sensor", O_RDWR);
  
  // validate connection
  if (spi_fd < 0 || dust_fd < 0) {
    printf("Devices are not found\n");
    return -1;
  }
  if (prepare2(spi_fd) == -1) {
    return -1;
  }

  uint8_t i;
  
  // control infrared light in dust sensor using ioctl function and get analog value
  ioctl(dust_fd, IOCTL_DUST_ON);
  usleep(280);
    
  res_ana = readadc(spi_fd, 1);
    
  usleep(40);
  ioctl(dust_fd, IOCTL_DUST_OFF);
  usleep(9680);
    
     
  // transform analog value to digital value
  res_dig = res_ana / 1024.0f * 5.0f;
  // transform digital value to dust value(㎍/㎥) 
  res_dust = ((0.17*res_dig) - 0.1) * 1000;
    
  //printf("ana : %d,\tdig : %f,\tdust : %f\n", res_ana, res_dig, res_dust);
    
  
  

  close(dust_fd);
  close(spi_fd);

  return res_ana;
}