#include <stdlib.h>
#include <stdint.h>

// Фильтр формирования импульсов (pulse shaping filter)
int16_t* apply_pulse_shaping(int* input_samples, int num_samples, int filter_length, int* filter_coeffs, int* output_size) {
    
    // Устанавливаем размер выходного массива равным входному
    *output_size = num_samples;
    
    // Выделяем память для отфильтрованных отсчетов
    int16_t* filtered_samples = (int16_t*)malloc(num_samples * sizeof(int16_t));
    
    // Обрабатываем каждый отсчет входного сигнала
    for (int sample_idx = 0; sample_idx < num_samples; ++sample_idx) {
        
        int convolution_sum = 0;
        
        // Выполняем свертку с коэффициентами фильтра
        for (int coeff_idx = 0; coeff_idx < filter_length; ++coeff_idx) {
            
            // Проверяем границы массива
            if (sample_idx - coeff_idx >= 0) {
                convolution_sum += input_samples[sample_idx - coeff_idx] * filter_coeffs[coeff_idx];
            }
        }
        
        // Масштабируем результат и сохраняем
        filtered_samples[sample_idx] = convolution_sum * (2047 << 4);
    }
    
    return filtered_samples;
}