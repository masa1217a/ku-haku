#include "ketugou.h"

static int distance_adc01_ch0 = 0;                  //測距：脱水投入口
static int distance_adc01_ch1 = 0;                  //測距：脱水投入口
static int distance_adc01_ch2 = 0;                  //測距：減容貯蓄部
static int distance_adc01_ch3 = 0;                  //測距：減容貯蓄部
static int distance_adc01_ch4 = 0;                  //測距：屑箱
static int distance_adc01_ch5 = 0;                  //測距：屑箱

static double temp_adc02_ch0 = 0;                   //温度：脱水モーター
static double temp_adc02_ch1 = 0;                   //温度：脱水モーター
static double temp_adc02_ch2 = 0;                   //温度：減容モーター
static double temp_adc02_ch3 = 0;                   //温度：減容モーター

/*****************************************
 *                              関数                                                  *
 * ****************************************/
int volt_distance(float volt)
{
  int distance = 0;
  if(2.4 < volt){
    distance = 10;
  }else if((1.37 < volt) && (volt <= 2.4)){
    distance = 33.3 - 9.71 * volt;
  }else if((0.9 < volt) && (volt <= 1.37)){
    distance = 49.15 - 21.27 * volt;
  }else if((0.79 < volt) && (volt <= 0.9)){
    distance = 111.82 - 90.91 * volt;
  }else if((0.69 < volt) && (volt <= 0.79)){
    distance = 119 - 100 * volt;
  }else if((0.5 < volt) && (volt <= 0.69)){
    distance = 86.32 - 52.63 * volt;
  }else if((0.44 < volt) && (volt <= 0.5)){
    distance = 180 - 250 * volt;
  }else{
    distance = 80;
  }

  return distance;
}

double map(double v)
{
    return(v - 4095) * (1 - 100) / (5 - 4095) + 100;
}

int adc01(void)
{

  int loop_count = 0;
  int anarog_ch0 = 0;
  int anarog_ch1 = 0;
  int anarog_ch2 = 0;
  int anarog_ch3 = 0;
  int anarog_ch4 = 0;
  int anarog_ch5 = 0;
  float volt_ch0 = 0;
  float volt_ch1 = 0;
  float volt_ch2 = 0;
  float volt_ch3 = 0;
  float volt_ch4 = 0;
  float volt_ch5 = 0;


  for(loop_count=1; loop_count<=10; loop_count++){
  char out_ch0[] = { 0b00000110, 0b00000000, 0b00000000 };
  char ch0_data[] = { 0x00, 0x00, 0x00 };
  char out_ch1[] = { 0b00000110, 0b01000000, 0b00000000 };
  char ch1_data[] = { 0x00, 0x00, 0x00 };
  char out_ch2[] = { 0b00000110, 0b10000000, 0b00000000 };
  char ch2_data[] = { 0x00, 0x00, 0x00 };
  char out_ch3[] = { 0b00000110, 0b11000000, 0b00000000 };
  char ch3_data[] = { 0x00, 0x00, 0x00 };
  char out_ch4[] = { 0b00000111, 0b00000000, 0b00000000 };
  char ch4_data[] = { 0x00, 0x00, 0x00 };
  char out_ch5[] = { 0b00000111, 0b01000000, 0b00000000 };
  char ch5_data[] = { 0x00, 0x00, 0x00 };

  if(!bcm2835_init()) return 1;

  bcm2835_spi_begin();
  bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);
  bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);
  bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_128);
  bcm2835_spi_chipSelect(BCM2835_SPI_CS0);
  bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0, LOW);

  bcm2835_spi_transfernb(out_ch0, ch0_data, 3);
  bcm2835_spi_transfernb(out_ch1, ch1_data, 3);
  bcm2835_spi_transfernb(out_ch2, ch2_data, 3);
  bcm2835_spi_transfernb(out_ch3, ch3_data, 3);
  bcm2835_spi_transfernb(out_ch4, ch4_data, 3);
  bcm2835_spi_transfernb(out_ch5, ch5_data, 3);
  //printf("CH0:    %02X %02X %02X\n", ch0_data[0], ch0_data[1], ch0_data[2]);
  //printf("CH6:    %02X %02X %02X\n", ch6_data[0], ch6_data[1], ch6_data[2]);
  anarog_ch0 = 16*16*ch0_data[1] + ch0_data[2];
  anarog_ch1 = 16*16*ch1_data[1] + ch1_data[2];
  anarog_ch2 = 16*16*ch2_data[1] + ch2_data[2];
  anarog_ch3 = 16*16*ch3_data[1] + ch3_data[2];
  anarog_ch4 = 16*16*ch4_data[1] + ch4_data[2];
  anarog_ch5 = 16*16*ch5_data[1] + ch5_data[2];
  volt_ch0 = 3.3 / 4095 * anarog_ch0;
  volt_ch1 = 3.3 / 4095 * anarog_ch1;
  volt_ch2 = 3.3 / 4095 * anarog_ch2;
  volt_ch3 = 3.3 / 4095 * anarog_ch3;
  volt_ch4 = 3.3 / 4095 * anarog_ch4;
  volt_ch5 = 3.3 / 4095 * anarog_ch5;
  //printf("CH0 Volt = %4.2f\n", volt_ch0);
  distance_adc01_ch0 += volt_distance(volt_ch0);
  distance_adc01_ch1 += volt_distance(volt_ch1);
  distance_adc01_ch2 += volt_distance(volt_ch2);
  distance_adc01_ch3 += volt_distance(volt_ch3);
  distance_adc01_ch4 += volt_distance(volt_ch4);
  distance_adc01_ch5 += volt_distance(volt_ch5);
  //printf("CH0 Distance %d cm\n", distance_adc01_ch0);

  bcm2835_spi_end();
  bcm2835_close();

  //sleep(1);
  }
  distance_adc01_ch0 = distance_adc01_ch0 / loop_count;
  distance_adc01_ch1 = distance_adc01_ch1 / loop_count;
  distance_adc01_ch2 = distance_adc01_ch2 / loop_count;
  distance_adc01_ch3 = distance_adc01_ch3 / loop_count;
  distance_adc01_ch4 = distance_adc01_ch4 / loop_count;
  distance_adc01_ch5 = distance_adc01_ch5 / loop_count;

  dist.ch0 = distance_adc01_ch0;
  dist.ch1 = distance_adc01_ch1;
  dist.ch2 = distance_adc01_ch2;
  dist.ch3 = distance_adc01_ch3;
  dist.ch4 = distance_adc01_ch4;
  dist.ch5 = distance_adc01_ch5;

  write_value("Distance");

  //printf("ADC01 CH0 Distance %d cm\n", distance_adc01_ch0);
  //printf("ADC01 CH1 Distance %d cm\n", distance_adc01_ch1);
  //printf("ADC01 CH2 Distance %d cm\n", distance_adc01_ch2);
  //printf("ADC01 CH3 Distance %d cm\n", distance_adc01_ch3);
  //printf("ADC01 CH4 Distance %d cm\n", distance_adc01_ch4);
  //printf("ADC01 CH5 Distance %d cm\n", distance_adc01_ch5);

  return 0;
}

int adc02(void)
{
  int loop_count = 0;
  int anarog_ch0 = 0;
  int anarog_ch1 = 0;
  int anarog_ch2 = 0;
  int anarog_ch3 = 0;
  double volt_ch0 = 0;
  double volt_ch1 = 0;
  double volt_ch2 = 0;
  double volt_ch3 = 0;
  double log_temp_ch0 = 0;
  double log_temp_ch1 = 0;
  double log_temp_ch2 = 0;
  double log_temp_ch3 = 0;

  for(loop_count=1; loop_count<=10; loop_count++){
  char out_ch0[] = { 0b00000110, 0b00000000, 0b00000000 };
  char ch0_data[] = { 0x00, 0x00, 0x00 };
  char out_ch1[] = { 0b00000110, 0b01000000, 0b00000000 };
  char ch1_data[] = { 0x00, 0x00, 0x00 };
  char out_ch2[] = { 0b00000110, 0b10000000, 0b00000000 };
  char ch2_data[] = { 0x00, 0x00, 0x00 };
  char out_ch3[] = { 0b00000110, 0b11000000, 0b00000000 };
  char ch3_data[] = { 0x00, 0x00, 0x00 };

  if(!bcm2835_init()) return 1;

  bcm2835_spi_begin();
  bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);
  bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);
  bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_128);
  bcm2835_spi_chipSelect(BCM2835_SPI_CS1);
  bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS1, LOW);

  bcm2835_spi_transfernb(out_ch0, ch0_data, 3);
  bcm2835_spi_transfernb(out_ch1, ch1_data, 3);
  bcm2835_spi_transfernb(out_ch2, ch2_data, 3);
  bcm2835_spi_transfernb(out_ch3, ch3_data, 3);

  anarog_ch0 = 16*16*ch0_data[1] + ch0_data[2];
  anarog_ch1 = 16*16*ch1_data[1] + ch1_data[2];
  anarog_ch2 = 16*16*ch2_data[1] + ch2_data[2];
  anarog_ch3 = 16*16*ch3_data[1] + ch3_data[2];

  //printf("CH0 Anarog = %d\n", anarog_ch0);

  volt_ch0 = 3.3 / 4095 * anarog_ch0;
  volt_ch1 = 3.3 / 4095 * anarog_ch1;
  volt_ch2 = 3.3 / 4095 * anarog_ch2;
  volt_ch3 = 3.3 / 4095 * anarog_ch3;

  //printf("CH0 Volt = %g\n", volt_ch0);

  log_temp_ch0 = (volt_ch0 * 9100) / (3.3 - volt_ch0);
  log_temp_ch1 = (volt_ch1 * 9100) / (3.3 - volt_ch1);
  log_temp_ch2 = (volt_ch2 * 9100) / (3.3 - volt_ch2);
  log_temp_ch3 = (volt_ch3 * 9100) / (3.3 - volt_ch3);
  temp_adc02_ch0 += 1 / ( log(log_temp_ch0 / 10000.0) / 3435 + (1 / (25.0 + 273.15))) - 273.15;
  temp_adc02_ch1 += 1 / ( log(log_temp_ch1 / 10000.0) / 3435 + (1 / (25.0 + 273.15))) - 273.15;
  temp_adc02_ch2 += 1 / ( log(log_temp_ch2 / 10000.0) / 3435 + (1 / (25.0 + 273.15))) - 273.15;
  temp_adc02_ch3 += 1 / ( log(log_temp_ch3 / 10000.0) / 3435 + (1 / (25.0 + 273.15))) - 273.15;

  //printf("CH0 %g\n", temp_adc02_ch0);

  bcm2835_spi_end();
  bcm2835_close();

  //sleep(1);
  }
  temp_adc02_ch0 = temp_adc02_ch0 / loop_count;
  temp_adc02_ch1 = temp_adc02_ch1 / loop_count;
  temp_adc02_ch2 = temp_adc02_ch2 / loop_count;
  temp_adc02_ch3 = temp_adc02_ch3 / loop_count;

  temp.dryA   = temp_adc02_ch0;
  temp.dryB   = temp_adc02_ch1;
  temp.crashA = temp_adc02_ch2;
  temp.crashB = temp_adc02_ch3;

  write_value("Temprature");

   //printf("CH0 Temprature %g ℃\n", temp_adc02_ch0);
   //printf("CH1 Temprature %g ℃\n", temp_adc02_ch1);
   //printf("CH2 Temprature %g ℃\n", temp_adc02_ch2);
   //printf("CH3 Temprature %g ℃\n", temp_adc02_ch3);
  return 0;
}
