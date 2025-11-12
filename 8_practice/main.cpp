#include <iostream>
#include <vector>
#include <cmath>
#include <complex>
#include <random>
#include <fstream>
#include <iio.h>
#include <thread>
#include <chrono>

using namespace std;

// Генерация случайных битов
vector<int> generate_bits(int num_bits) {
    vector<int> bits(num_bits);
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(0, 1);
    
    for (int i = 0; i < num_bits; i++) {
        bits[i] = dis(gen);
    }
    return bits;
}

// Функция свертки по формуле: y[n] = sum_k x[n] * h[n-k]
vector<complex<double>> custom_convolution(const vector<complex<double>>& x, const vector<double>& h) {
    int N = x.size();
    int M = h.size();
    vector<complex<double>> y(N, 0.0);
    
    for (int n = 0; n < N; n++) {
        for (int k = 0; k < M; k++) {
            if (n - k >= 0) {
                y[n] += x[n] * h[k];
            }
        }
    }
    
    return y;
}

// Специальная функция для преобразования [1,1] + 9 нулей -> 10 раз [1,1]
vector<complex<double>> apply_symbol_spreading(const vector<complex<double>>& input, int spread_factor = 10) {
    vector<complex<double>> output;
    
    for (int i = 0; i < input.size(); i++) {
        if (input[i] != complex<double>(0, 0)) {
            // Найден символ - распространяем его на spread_factor позиций
            for (int j = 0; j < spread_factor; j++) {
                output.push_back(input[i]);
            }
        } else {
            // Нулевой символ - добавляем как есть
            output.push_back(input[i]);
        }
    }
    
    return output;
}

vector<complex<double>> bpsk_modulation(const vector<int>& bits, int upsample_factor = 10) {
    vector<complex<double>> iq_samples(bits.size() * upsample_factor);
    
    for (size_t i = 0; i < bits.size(); i++) {
        // BPSK: 0 -> +1, 1 -> -1
        double symbol = bits[i] == 0 ? 1.0 : -1.0;
        
        // Первый сэмпл - символ, остальные upsample_factor-1 сэмплов - нули
        iq_samples[i * upsample_factor] = complex<double>(symbol, 0.0);
        
        // Заполняем нулями между символами
        for (int j = 1; j < upsample_factor; j++) {
            iq_samples[i * upsample_factor + j] = complex<double>(0.0, 0.0);
        }
    }
    
    return iq_samples;
}

// QPSK модуляция с upsampling
vector<complex<double>> qpsk_modulation(const vector<int>& bits, int upsample_factor = 10) {
    if (bits.size() % 2 != 0) {
        throw invalid_argument("Для QPSK количество битов должно быть четным");
    }
    
    vector<complex<double>> iq_samples((bits.size() / 2) * upsample_factor);
    
    for (size_t i = 0; i < bits.size(); i += 2) {
        int bit_i = bits[i];
        int bit_q = bits[i + 1];
        
        double i_component = bit_i == 0 ? 1.0 : -1.0;
        double q_component = bit_q == 0 ? 1.0 : -1.0;
        
        // Первый сэмпл - символ, остальные upsample_factor-1 сэмплов - нули
        size_t symbol_index = (i / 2) * upsample_factor;
        iq_samples[symbol_index] = complex<double>(i_component, q_component);
        
        // Заполняем нулями между символами
        for (int j = 1; j < upsample_factor; j++) {
            iq_samples[symbol_index + j] = complex<double>(0.0, 0.0);
        }
    }
    
    return iq_samples;
}

// Upsampling с фильтром возвышения косинуса
vector<complex<double>> upsample(const vector<complex<double>>& symbols, 
                                int samples_per_symbol, 
                                double beta = 0.35) {
    int num_symbols = symbols.size();
    int total_samples = num_symbols * samples_per_symbol;
    vector<complex<double>> upsampled(total_samples, 0.0);
    
    // Простой upsampling
    for (int i = 0; i < num_symbols; i++) {
        upsampled[i * samples_per_symbol] = symbols[i];
    }
    
    return upsampled;
}

// Конвертация complex<double> в int16_t для Pluto SDR
vector<int16_t> convert_to_pluto_format(const vector<complex<double>>& iq_data, double scale_factor = 2000.0) {
    vector<int16_t> pluto_data(iq_data.size() * 2); // I и Q чередуются
    
    for (size_t i = 0; i < iq_data.size(); i++) {
        pluto_data[2 * i] = static_cast<int16_t>(iq_data[i].real() * scale_factor);     // I
        pluto_data[2 * i + 1] = static_cast<int16_t>(iq_data[i].imag() * scale_factor); // Q
    }
    
    return pluto_data;
}

// Инициализация Pluto SDR
struct iio_context* init_pluto_sdr(const char* uri = "ip:pluto.local") {
    struct iio_context* ctx = iio_create_context_from_uri(uri);
    if (!ctx) {
        cerr << "Не удалось подключиться к Pluto SDR по адресу: " << uri << endl;
        return nullptr;
    }
    
    cout << "Успешное подключение к Pluto SDR" << endl;
    return ctx;
}

// Настройка Pluto SDR для передачи
bool setup_pluto_tx(struct iio_context* ctx, long long sample_rate = 1000000, long long frequency = 1000000000) {
    // Находим устройство TX
    struct iio_device* tx_dev = iio_context_find_device(ctx, "cf-ad9361-dds-core-lpc");
    if (!tx_dev) {
        cerr << "Не удалось найти TX устройство" << endl;
        return false;
    }
    
    // Настраиваем частоту дискретизации
    struct iio_device* phy = iio_context_find_device(ctx, "ad9361-phy");
    if (phy) {
        iio_channel_attr_write_longlong(
            iio_device_find_channel(phy, "voltage0", true),
            "sampling_frequency", sample_rate);
    }
    
    // Настраиваем несущую частоту
    if (phy) {
        iio_channel_attr_write_longlong(
            iio_device_find_channel(phy, "altvoltage0", true),
            "frequency", frequency);
    }
    
    cout << "Pluto SDR настроен: Sample Rate = " << sample_rate 
         << " Hz, Frequency = " << frequency << " Hz" << endl;
    
    return true;
}

// Передача данных через Pluto SDR
bool transmit_with_pluto(struct iio_context* ctx, const vector<int16_t>& tx_data) {
    struct iio_device* tx_dev = iio_context_find_device(ctx, "cf-ad9361-dds-core-lpc");
    if (!tx_dev) {
        cerr << "TX устройство не найдено" << endl;
        return false;
    }
    
    struct iio_buffer* tx_buf = iio_device_create_buffer(tx_dev, tx_data.size() / 2, false);
    if (!tx_buf) {
        cerr << "Не удалось создать TX buffer" << endl;
        return false;
    }
    
    // Копируем данные в буфер
    void* buf_start = iio_buffer_start(tx_buf);
    memcpy(buf_start, tx_data.data(), tx_data.size() * sizeof(int16_t));
    
    // Передаем данные
    ssize_t result = iio_buffer_push(tx_buf);
    if (result < 0) {
        cerr << "Ошибка передачи данных: " << result << endl;
        iio_buffer_destroy(tx_buf);
        return false;
    }
    
    cout << "Передано " << result << " сэмплов" << endl;
    
    iio_buffer_destroy(tx_buf);
    return true;
}

// Сохранение данных в файл для анализа
void save_to_file(const vector<complex<double>>& iq_data, 
                  const vector<int>& bits, 
                  const string& filename) {
    ofstream file(filename);
    
    // Записываем заголовок
    file << "bits,real,imag" << endl;
    
    int symbols_per_bit = (iq_data.size() > bits.size()) ? iq_data.size() / bits.size() : 1;
    
    for (size_t i = 0; i < iq_data.size(); i++) {
        int bit_index = i / symbols_per_bit;
        if (bit_index < bits.size()) {
            file << bits[bit_index] << "," 
                 << iq_data[i].real() << "," 
                 << iq_data[i].imag() << endl;
        } else {
            file << "0," << iq_data[i].real() << "," << iq_data[i].imag() << endl;
        }
    }
    
    file.close();
}

// Демонстрация работы преобразования
void demonstrate_spreading() {
    cout << "\n=== ДЕМОНСТРАЦИЯ ПРЕОБРАЗОВАНИЯ ===" << endl;
    
    // Тест 1: [1,1] и 9 нулей
    vector<complex<double>> test1 = {
        {1, 1}, {1, 1},  // первые 2 сэмпла (1,1)
        {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}  // 9 нулей
    };
    
    cout << "ТЕСТ 1: [1,1] и 9 нулей" << endl;
    cout << "Входные сэмплы: ";
    for (int i = 0; i < 11; i++) {
        cout << "(" << test1[i].real() << "," << test1[i].imag() << ") ";
    }
    cout << endl;
    
    vector<complex<double>> result1 = apply_symbol_spreading(test1, 10);
    
    cout << "Сэмплы после преобразования: ";
    for (int i = 0; i < 10; i++) {
        cout << "(" << result1[i].real() << "," << result1[i].imag() << ") ";
    }
    cout << endl;
}

int main() {
    // Демонстрация работы
    demonstrate_spreading();
    
    // Параметры
    int num_bits = 1000; // Количество битов
    int samples_per_symbol = 4;   // Samples per symbol для upsampling
    string modulation = "qpsk";   // "bpsk" или "qpsk"
    long long sample_rate = 1000000;  // 1 MHz
    long long frequency = 1000000000; // 1 GHz
    
    cout << "\n=== ОСНОВНАЯ ПРОГРАММА ===" << endl;
    cout << "Генерация " << num_bits << " случайных битов..." << endl;
    vector<int> bits = generate_bits(num_bits);
    
    vector<complex<double>> modulated_symbols;
    
    // Модуляция
    if (modulation == "bpsk") {
        cout << "BPSK модуляция..." << endl;
        modulated_symbols = bpsk_modulation(bits);
    } else if (modulation == "qpsk") {
        cout << "QPSK модуляция..." << endl;
        modulated_symbols = qpsk_modulation(bits);
    } else {
        cerr << "Неизвестный тип модуляции: " << modulation << endl;
        return 1;
    }
    
    cout << "Upsampling (" << samples_per_symbol << " samples per symbol)..." << endl;
    vector<complex<double>> upsampled_iq = upsample(modulated_symbols, samples_per_symbol);
    
    // ПРИМЕНЯЕМ ПРЕОБРАЗОВАНИЕ ДЛЯ РАСПРОСТРАНЕНИЯ СИМВОЛОВ
    cout << "\nПрименение преобразования для распространения символов..." << endl;
    vector<complex<double>> spread_iq = apply_symbol_spreading(upsampled_iq, 10);
    
    // Выводим сэмплы до и после преобразования
    cout << "\nПервые 30 сэмплов ДО преобразования: " << endl;
    for (int i = 0; i < min(30, (int)upsampled_iq.size()); i++) {
        cout << "Сэмпл " << i << ": (" << upsampled_iq[i].real() << ", " << upsampled_iq[i].imag() << ")" << endl;
    }
    
    cout << "\nПервые 30 сэмплов ПОСЛЕ преобразования: " << endl;
    for (int i = 0; i < min(30, (int)spread_iq.size()); i++) {
        cout << "Сэмпл " << i << ": (" << spread_iq[i].real() << ", " << spread_iq[i].imag() << ")" << endl;
    }
    
    // Инициализация Pluto SDR
    cout << "\n=== НАСТРОЙКА PLUTO SDR ===" << endl;
    struct iio_context* ctx = init_pluto_sdr();
    if (!ctx) {
        cerr << "Продолжаем без Pluto SDR - данные будут сохранены в файл" << endl;
    } else {
        // Настройка параметров Pluto SDR
        if (setup_pluto_tx(ctx, sample_rate, frequency)) {
            // Конвертация данных в формат Pluto SDR
            cout << "Конвертация данных в формат Pluto SDR..." << endl;
            vector<int16_t> pluto_data = convert_to_pluto_format(spread_iq);
            
            // Передача данных
            cout << "Передача данных через Pluto SDR..." << endl;
            if (transmit_with_pluto(ctx, pluto_data)) {
                cout << "Передача успешно завершена!" << endl;
            } else {
                cerr << "Ошибка передачи данных" << endl;
            }
        }
        
        // Задержка для завершения передачи
        this_thread::sleep_for(chrono::milliseconds(100));
        
        // Освобождение ресурсов
        iio_context_destroy(ctx);
        cout << "Pluto SDR отключен" << endl;
    }
    
    // Сохранение результатов в файл
    string filename = "iq_data_" + modulation + "_spread.csv";
    save_to_file(spread_iq, bits, filename);
    
    cout << "\nДанные сохранены в " << filename << endl;
    cout << "Количество сэмплов после преобразования: " << spread_iq.size() << endl;
    
    return 0;
}