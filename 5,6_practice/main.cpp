#include <SoapySDR/Device.h>
#include <SoapySDR/Formats.h>
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <vector>
#include <string>

constexpr int ADC_RESOLUTION = 12;
constexpr int SAMPLE_SHIFT = 4;
constexpr int SYMBOL_DURATION = 20;
constexpr int CARRIER_FREQUENCY = 800000000;
constexpr int SAMPLING_RATE = 1000000;
constexpr char TRANSMISSION_MESSAGE[] = "Digital Signal Processing Test";
constexpr char AUDIO_FILE_PATH[] = "../audio_converter/audio_data.pcm";

std::vector<uint8_t> convert_string_to_bits(const std::string& text) {
    std::vector<uint8_t> bit_sequence;
    bit_sequence.reserve(text.length() * 8);
    
    for (char character : text) {
        for (int bit_pos = 7; bit_pos >= 0; --bit_pos) {
            bit_sequence.push_back((character >> bit_pos) & 1);
        }
    }
    
    return bit_sequence;
}

std::vector<int16_t> generate_bpsk_signal(const std::vector<uint8_t>& bits, size_t buffer_size) {
    std::vector<int16_t> signal_buffer(buffer_size * 2, 0);
    const int16_t signal_amplitude = 2047 << SAMPLE_SHIFT;
    
    for (size_t bit_index = 0; bit_index < bits.size(); ++bit_index) {
        size_t start_sample = bit_index * SYMBOL_DURATION;
        
        for (size_t sample_offset = 0; sample_offset < SYMBOL_DURATION && 
             start_sample + sample_offset < buffer_size; sample_offset += 2) {
            
            if (bits[bit_index] == 1) {
                signal_buffer[start_sample + sample_offset] = signal_amplitude;
                signal_buffer[start_sample + sample_offset + 1] = -signal_amplitude;
            }
        }
    }
    
    return signal_buffer;
}

std::vector<int16_t> load_audio_samples(const char* filename, size_t* total_samples) {
    FILE* audio_file = fopen(filename, "rb");
    if (!audio_file) {
        printf("Не удалось открыть файл: %s\n", filename);
        return {};
    }
    
    fseek(audio_file, 0, SEEK_END);
    long file_size = ftell(audio_file);
    fseek(audio_file, 0, SEEK_SET);
    
    *total_samples = file_size / sizeof(int16_t);
    std::vector<int16_t> audio_data(*total_samples);
    
    size_t read_samples = fread(audio_data.data(), sizeof(int16_t), *total_samples, audio_file);
    fclose(audio_file);
    
    if (read_samples != *total_samples) {
        printf("Предупреждение: прочитано %zu samples из %zu\n", read_samples, *total_samples);
    }
    
    return audio_data;
}

void configure_sdr_device(SoapySDRDevice* device) {
    // Конфигурация приемника
    SoapySDRDevice_setSampleRate(device, SOAPY_SDR_RX, 0, SAMPLING_RATE);
    SoapySDRDevice_setFrequency(device, SOAPY_SDR_RX, 0, CARRIER_FREQUENCY, nullptr);
    
    // Конфигурация передатчика
    SoapySDRDevice_setSampleRate(device, SOAPY_SDR_TX, 0, SAMPLING_RATE);
    SoapySDRDevice_setFrequency(device, SOAPY_SDR_TX, 0, CARRIER_FREQUENCY, nullptr);
    
    // Настройка усиления
    SoapySDRDevice_setGain(device, SOAPY_SDR_RX, 0, 65.0);
    SoapySDRDevice_setGain(device, SOAPY_SDR_TX, 0, -30.0);
}

int main() {
    // Инициализация SDR устройства
    SoapySDRKwargs device_args = {};
    SoapySDRKwargs_set(&device_args, "driver", "plutosdr");
    SoapySDRKwargs_set(&device_args, "uri", "usb:");
    SoapySDRKwargs_set(&device_args, "timestamp_every", "1920");
    
    SoapySDRDevice* sdr_device = SoapySDRDevice_make(&device_args);
    SoapySDRKwargs_clear(&device_args);
    
    if (!sdr_device) {
        printf("Ошибка инициализации SDR устройства\n");
        return -1;
    }
    
    // Конфигурация устройства
    configure_sdr_device(sdr_device);
    
    // Настройка потоков
    const size_t channels[] = {0};
    SoapySDRStream* rx_stream = SoapySDRDevice_setupStream(
        sdr_device, SOAPY_SDR_RX, SOAPY_SDR_CS16, channels, 1, nullptr);
    SoapySDRStream* tx_stream = SoapySDRDevice_setupStream(
        sdr_device, SOAPY_SDR_TX, SOAPY_SDR_CS16, channels, 1, nullptr);
    
    SoapySDRDevice_activateStream(sdr_device, rx_stream, 0, 0, 0);
    SoapySDRDevice_activateStream(sdr_device, tx_stream, 0, 0, 0);
    
    // Загрузка аудиоданных
    size_t audio_sample_count = 0;
    std::vector<int16_t> audio_samples = load_audio_samples(AUDIO_FILE_PATH, &audio_sample_count);
    
    if (audio_samples.empty()) {
        printf("Не удалось загрузить аудиоданные\n");
        SoapySDRDevice_unmake(sdr_device);
        return -1;
    }
    
    // Подготовка буферов
    size_t rx_buffer_size = SoapySDRDevice_getStreamMTU(sdr_device, rx_stream);
    size_t tx_buffer_size = SoapySDRDevice_getStreamMTU(sdr_device, tx_stream);
    
    std::vector<int16_t> rx_buffer(rx_buffer_size * 2);
    std::vector<int16_t> tx_buffer(tx_buffer_size * 2);
    
    // Файлы для записи данных
    FILE* tx_record_file = fopen("transmitted_data.pcm", "wb");
    FILE* rx_record_file = fopen("received_data.pcm", "wb");
    
    const long long timeout_microseconds = 400000;
    long long previous_timestamp = 0;
    int buffer_counter = 1;
    
    // Основной цикл обработки данных
    for (size_t data_offset = 0; data_offset < audio_sample_count; data_offset += 1920 * 2) {
        // Прием данных
        void* rx_buffers[] = {rx_buffer.data()};
        int rx_flags;
        long long rx_timestamp;
        
        int received_samples = SoapySDRDevice_readStream(
            sdr_device, rx_stream, rx_buffers, rx_buffer_size, 
            &rx_flags, &rx_timestamp, timeout_microseconds);
        
        printf("Буфер #%d: %d samples, Время: %lld нс, Интервал: %lld нс\n",
               buffer_counter++, received_samples, rx_timestamp, 
               rx_timestamp - previous_timestamp);
        
        previous_timestamp = rx_timestamp;
        
        // Запись принятых данных
        if (rx_record_file) {
            fwrite(rx_buffer.data(), sizeof(int16_t), rx_buffer_size * 2, rx_record_file);
        }
        
        // Передача данных
        if (data_offset + 1920 * 2 >= audio_sample_count) break;
        
        long long tx_timestamp = rx_timestamp + 4000000; // +4 мс
        void* tx_buffers[] = {audio_samples.data() + data_offset};
        
        int tx_flags = SOAPY_SDR_HAS_TIME;
        int transmitted_samples = SoapySDRDevice_writeStream(
            sdr_device, tx_stream, tx_buffers, tx_buffer_size, 
            &tx_flags, tx_timestamp, timeout_microseconds);
        
        if (tx_record_file) {
            fwrite(audio_samples.data() + data_offset, sizeof(int16_t), 
                   tx_buffer_size * 2, tx_record_file);
        }
        
        if ((size_t)transmitted_samples != tx_buffer_size) {
            printf("Ошибка передачи: %d samples\n", transmitted_samples);
        }
    }
    
    // Завершение работы
    if (tx_record_file) fclose(tx_record_file);
    if (rx_record_file) fclose(rx_record_file);
    
    SoapySDRDevice_deactivateStream(sdr_device, rx_stream, 0, 0);
    SoapySDRDevice_deactivateStream(sdr_device, tx_stream, 0, 0);
    SoapySDRDevice_cleanupStream(sdr_device, rx_stream);
    SoapySDRDevice_cleanupStream(sdr_device, tx_stream);
    SoapySDRDevice_unmake(sdr_device);
    
    printf("Программа завершена успешно\n");
    return 0;
}