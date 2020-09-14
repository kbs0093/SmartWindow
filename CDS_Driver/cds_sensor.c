#include "cds_sensor.h"

static int prepare(int fd) {
    if (ioctl(fd, SPI_IOC_WR_MODE, &MODE) == -1) {
      perror("Can't set MODE");
      close(fd);
      return -1;
    }
    if (ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &BITS) == -1) {
      perror("Can't set number of BITS");
      close(fd);
      return -1;
    }
    if (ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &CLOCK) == -1) {
      perror("Can't set write CLOCK");
      close(fd);
      return -1;
    }
    if (ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &CLOCK) == -1) {
      perror("Can't set read CLOCK");
      close(fd);
      return -1;
    }
    return 0;
}

uint8_t control_bits_differential(uint8_t channel){
    return (channel & 7) << 4;
}

uint8_t control_bits(uint8_t channel) {
    return 0x8 | control_bits_differential(channel);
    //return 0x0;
}

int readadc(uint8_t channel) {
    uint8_t tx[3] = {1,120,0 }; //  SGL/DIFF D2/D1/D0 = 1111 0000
    uint8_t rx[3];
    
    uint8_t rx_w0;
    uint8_t rx_w1;
    uint8_t rx_w2;
    uint8_t rx_w3;
    
    int fd = open(DEVICE, O_RDWR);
    int answer;
        
    //printf("%u \n", control_bits(channel));    
      
    //tx[0] = 0xF0;      

    struct spi_ioc_transfer tr = {
      .tx_buf = (unsigned long)tx,
      .rx_buf = (unsigned long)rx, //we check only output so doesnt matter
      .len = ARRAY_SIZE(tx),
      .delay_usecs = DELAY,
      .speed_hz = CLOCK,       //10kHz
      .bits_per_word = BITS,   //mcp3008 8 bit per frame
    };
    
loop: 
    
    if (fd <= 0) {
      printf("Device %s not found\n", DEVICE);
      close(fd);
      return -1;
    }    
    
    if (prepare(fd) == -1) {
      close(fd);
      return -1;
    }

    if (ioctl(fd, SPI_IOC_MESSAGE(1), &tr) == 1) {
      perror("IO Error");
      close(fd);
      abort();
    }
    
    rx_w1 = rx[0] & MCP3008_RX_WORD_01;
    if(rx_w1 != 0)
        goto loop;
        
    rx_w2 = rx[1] & MCP3008_RX_NULLBYTE_02;
    if(rx_w2 != 0)
        goto loop;
        
    rx_w2 = rx[1] & MCP3008_RX_DATA_02;   // cds sensor value is too low change
    rx_w3 = rx[2] & MCP3008_RX_DATA_03;   // so rx value calculate 
    
    rx_w0 = ((rx_w2 << 8) | (rx_w3)) & MCP3008_RX_DATA_00; 
    answer = rx_w0;
        
    close(fd);

    return answer;
}
