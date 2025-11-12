#include <SoapySDR/Device.h>   // Инициализация устройства
#include <SoapySDR/Formats.h>  // Типы данных, используемых для записи сэмплов
#include <stdio.h>             //printf
#include <stdlib.h>            //free
#include <stdint.h>
#include <complex.h>
#include <string.h>

#define ADC_CAPACITY 12
#define BITS_SHIFT 4
#define TAU 10
#define TAU_ON_BITS TAU*2
#define MESSAGE "Hello My Beuatiful World"

uint8_t* stob(char* str, int* out_bits_count) {
    // get string len
    int len = strlen(str);

    // 1 char = 8 bit 
    *out_bits_count = len * sizeof(char);
    
    // massive for bits
    uint8_t* bits = (uint8_t*)malloc(*out_bits_count * sizeof(uint8_t));

    // check pointer
    if (bits == NULL){
        return NULL;
    }
        
    char c;

    // iterate on string
    for (int i = 0; i < len; i++) {
        // get char 
        c = str[i];
        // iterate on bits massive
        for (int j = 0; j < 8; j++) {
            // convert char to bits 
            bits[i * 8 + j] = (c >> (7 - j)) & 1;
        }
    }
    
    return bits;
}

int16_t* bits_to_signal(uint8_t* bits, int bits_count, int tx_mtu){

    // allocate memory
    int16_t* tx_buff = (int16_t*)malloc(sizeof(int16_t) * tx_mtu * 2);

    // iterate on bitts
    for (int i = 0; i < bits_count; i += 1)
    {   
        // fill tx_buff with samples
        for(int j = i*TAU_ON_BITS; j < i*TAU_ON_BITS + 20 && j < tx_mtu*2; j+=2){
            if(bits[i]){
                tx_buff[j] = 2047 << 4;    // I
                tx_buff[j+1] = -2047 << 4; // Q
            } else{
                tx_buff[j] = 0;      //I
                tx_buff[j+1] = 0;    //Q
            }
        }
    }

    return tx_buff;
}


int main(){
    FILE *rx_samples = fopen("rx_samples.txt", "w");
    FILE *tx_samples = fopen("tx_samples.txt", "w");

    if(rx_samples == nullptr){
        printf("Erorr in open file\n");
        return -1;
    }

    if(tx_samples == nullptr){
        printf("Erorr in open file\n");
        return -1;
    }

    SoapySDRKwargs args = {};

    SoapySDRKwargs_set(&args, "driver", "plutosdr");        // Говорим какой Тип устройства 
    if (1) {
        SoapySDRKwargs_set(&args, "uri", "usb:3.10.5");            // Способ обмена сэмплами (USB)
    } else {
        SoapySDRKwargs_set(&args, "uri", "ip:192.168.2.1"); // Или по IP-адресу
    }
    SoapySDRKwargs_set(&args, "direct", "1");               // 
    SoapySDRKwargs_set(&args, "timestamp_every", "1920");   // Размер буфера + временные метки
    SoapySDRKwargs_set(&args, "loopback", "0");             // Используем антенны или нет
    SoapySDRDevice *sdr = SoapySDRDevice_make(&args);       // Инициализация
    SoapySDRKwargs_clear(&args);

    int sample_rate = 1e6;
    int carrier_freq = 800e6;
    // Параметры RX части
    SoapySDRDevice_setSampleRate(sdr, SOAPY_SDR_RX, 0, sample_rate);
    SoapySDRDevice_setFrequency(sdr, SOAPY_SDR_RX, 0, carrier_freq , NULL);

    // Параметры TX части
    SoapySDRDevice_setSampleRate(sdr, SOAPY_SDR_TX, 0, sample_rate);
    SoapySDRDevice_setFrequency(sdr, SOAPY_SDR_TX, 0, carrier_freq , NULL);

    // Инициализация количества каналов RX\TX (в AdalmPluto он один, нулевой)
    size_t channels[] = {0};
    size_t channel_count = sizeof(channels) / sizeof(channels[0]);

    // Настройки усилителей на RXTX
    SoapySDRDevice_setGain(sdr, SOAPY_SDR_RX, channels[0], 10.0); // Чувствительность приемника
    SoapySDRDevice_setGain(sdr, SOAPY_SDR_TX, channels[0], -90.0);// Усиление передатчика

    // Формирование потоков для передачи и приема сэмплов
    SoapySDRStream *rxStream = SoapySDRDevice_setupStream(sdr, SOAPY_SDR_RX, SOAPY_SDR_CS16, channels, channel_count, NULL);
    SoapySDRStream *txStream = SoapySDRDevice_setupStream(sdr, SOAPY_SDR_TX, SOAPY_SDR_CS16, channels, channel_count, NULL);

    SoapySDRDevice_activateStream(sdr, rxStream, 0, 0, 0); //start streaming
    SoapySDRDevice_activateStream(sdr, txStream, 0, 0, 0); //start streaming

    // Получение MTU (Maximum Transmission Unit), в нашем случае - размер буферов. 
    size_t rx_mtu = SoapySDRDevice_getStreamMTU(sdr, rxStream);
    //size_t tx_mtu = SoapySDRDevice_getStreamMTU(sdr, txStream);

    int16_t rx_buffer[2 * rx_mtu];
    // размер буффера (т.к в rx/tx mtu находится кол-во семплов, а в одном семпле 2 числа типа int16_t)
    //int tx_buffer_size =  tx_mtu * 2;
    // Выделяем память под буферы RX и TX
    int bits_count;
    uint8_t* bits = stob(MESSAGE, &bits_count);
    int tx_mtu = bits_count * TAU;
    int16_t* tx_buff = bits_to_signal(bits, bits_count, tx_mtu);
 
    //заполнение tx_buff значениями сэмплов первые 16 бит - I, вторые 16 бит - Q.

    for(int i = 0; i < tx_mtu * 2; i+=2){
        fprintf(tx_samples, "(%d,%d),", tx_buff[i], tx_buff[i+1]);
    }

    

    //prepare fixed bytes in transmit buffer
    //we transmit a pattern of FFFF FFFF [TS_0]00 [TS_1]00 [TS_2]00 [TS_3]00 [TS_4]00 [TS_5]00 [TS_6]00 [TS_7]00 FFFF FFFF
    //that is a flag (FFFF FFFF) followed by the 64 bit timestamp, split into 8 bytes and packed into the lsb of each of the DAC words.
    //DAC samples are left aligned 12-bits, so each byte is left shifted into place
    for(size_t i = 0; i < 2; i++)
    {
        tx_buff[0 + i] = 0xffff;
        // 8 x timestamp words
        tx_buff[10 + i] = 0xffff;
    }

    // Количество итерация чтения из буфера
    size_t iteration_count = 6;
    const long  timeoutUs = 400000;
    long long last_time = 0;

    // Начинается работа с получением и отправкой сэмплов
    for (size_t buffers_read = 0; buffers_read < iteration_count; buffers_read++)
    {
        void *rx_buffs[] = {rx_buffer};
        int flags;        // flags set by receive operation
        long long timeNs; //timestamp for receive buffer
        
        // считали буффер RX, записали его в rx_buffer
        int sr = SoapySDRDevice_readStream(sdr, rxStream, rx_buffs, rx_mtu, &flags, &timeNs, timeoutUs);

        // Смотрим на количество считаных сэмплов, времени прихода и разницы во времени с чтением прошлого буфера
        printf("Buffer: %lu - Samples: %i, Flags: %i, Time: %lli, TimeDiff: %lli\n", buffers_read, sr, flags, timeNs, timeNs - last_time);
        last_time = timeNs;
        // пишем в файл

        for(int i = 0; i < rx_mtu*2; i++){
            fprintf(rx_samples, "(%d,%d), ", rx_buffer[i], rx_buffer[i+1]);
        }

        // Переменная для времени отправки сэмплов относительно текущего приема
        long long tx_time = timeNs + (4 * 1000 * 1000); // на 4 [мс] в будущее

        // Добавляем время, когда нужно передать блок tx_buff, через tx_time -наносекунд
        for(size_t i = 0; i < 8; i++)
        {
            uint8_t tx_time_byte = (tx_time >> (i * 8)) & 0xff;
            tx_buff[2 + i] = tx_time_byte << 4;
        }

        // Здесь отправляем наш tx_buff массив
        void *tx_buffs[] = {tx_buff};
        if( (buffers_read == 2) ){
            printf("buffers_read: %d\n", buffers_read);
            flags = SOAPY_SDR_HAS_TIME;
            int st = SoapySDRDevice_writeStream(sdr, txStream, (const void * const*)tx_buffs, tx_mtu, &flags, tx_time, timeoutUs);
            if ((size_t)st != tx_mtu)
            {
                printf("TX Failed: %i\n", st);
            }
        }
        
    }

    //stop streaming
    SoapySDRDevice_deactivateStream(sdr, rxStream, 0, 0);
    SoapySDRDevice_deactivateStream(sdr, txStream, 0, 0);

    //shutdown the stream
    SoapySDRDevice_closeStream(sdr, rxStream);
    SoapySDRDevice_closeStream(sdr, txStream);

    //cleanup device handle
    SoapySDRDevice_unmake(sdr);
    fclose(rx_samples);

    return 0;
}