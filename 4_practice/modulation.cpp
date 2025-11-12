#include <stdlib.h>
#include <stdio.h>
#include <math.h>

// BPSK модуляция
double* BPSK_modulation(int* bits, int bits_count) {
    double* IQ_samples = (double*)malloc(sizeof(double) * bits_count * 2);

    for(int i = 0, j = 0; i < bits_count * 2; i += 2, j++) {
        if(bits[j]) {
            IQ_samples[i] = 1;
            IQ_samples[i + 1] = 0;
        } else {
            IQ_samples[i] = -1;
            IQ_samples[i + 1] = 0;
        }
    }
    return IQ_samples;
}

// QPSK модуляция
double* QPSK_modulation(int* bits, int bits_count) {
    double* IQ_samples = (double*)malloc(sizeof(double) * bits_count);

    for(int i = 0; i < bits_count; i += 2) {
        if(bits[i]) {
            IQ_samples[i] = -1;
            if(bits[i + 1]) {
                IQ_samples[i + 1] = -1;
            } else {
                IQ_samples[i + 1] = 1;
            }
        } else {
            IQ_samples[i] = 1;
            if(bits[i + 1]) {
                IQ_samples[i + 1] = 1;
            } else {
                IQ_samples[i + 1] = -1;
            }
        }
    }
    return IQ_samples;
}

// Функция для сохранения данных для построения графиков
void save_plot_data(const char* filename, double* I_samples, double* Q_samples, int samples_count, int Fs) {
    FILE* file = fopen(filename, "w");
    if(file == nullptr) return;
    
    // Сохраняем I компоненту
    fprintf(file, "I_samples = [");
    for(int i = 0; i < samples_count; i++) {
        for(int j = 0; j < Fs; j++) {
            fprintf(file, "%.2f", I_samples[i]);
            if(i < samples_count - 1 || j < Fs - 1) fprintf(file, ", ");
        }
    }
    fprintf(file, "]\n\n");
    
    // Сохраняем Q компоненту
    fprintf(file, "Q_samples = [");
    for(int i = 0; i < samples_count; i++) {
        for(int j = 0; j < Fs; j++) {
            fprintf(file, "%.2f", Q_samples[i]);
            if(i < samples_count - 1 || j < Fs - 1) fprintf(file, ", ");
        }
    }
    fprintf(file, "]\n\n");
    
    // Сохраняем временную ось
    fprintf(file, "t = [");
    double total_time = (double)(samples_count * Fs) / Fs;
    for(int i = 0; i < samples_count * Fs; i++) {
        fprintf(file, "%.3f", (double)i / Fs);
        if(i < samples_count * Fs - 1) fprintf(file, ", ");
    }
    fprintf(file, "]\n");
    
    fclose(file);
}

// Функция для создания Python скрипта для построения графиков
void create_python_plot_script(const char* modulation_type) {
    FILE* script = fopen("plot_signal.py", "w");
    if(script == nullptr) return;
    
    fprintf(script, "import matplotlib.pyplot as plt\n");
    fprintf(script, "import numpy as np\n\n");
    
    fprintf(script, "def plot_modulation():\n");
    fprintf(script, "    # Загружаем данные\n");
    fprintf(script, "    with open('plot_data.txt', 'r') as f:\n");
    fprintf(script, "        exec(f.read())\n\n");
    
    fprintf(script, "    # График I и Q компонент\n");
    fprintf(script, "    plt.figure(figsize=(12, 8))\n\n");
    
    fprintf(script, "    plt.subplot(2, 1, 1)\n");
    fprintf(script, "    plt.step(t, I_samples, where='post')\n");
    fprintf(script, "    plt.title('%s - In-phase Component (I)')\n", modulation_type);
    fprintf(script, "    plt.xlabel('Time (s)')\n");
    fprintf(script, "    plt.ylabel('Amplitude')\n");
    fprintf(script, "    plt.grid(True)\n\n");
    
    fprintf(script, "    plt.subplot(2, 1, 2)\n");
    fprintf(script, "    plt.step(t, Q_samples, where='post')\n");
    fprintf(script, "    plt.title('%s - Quadrature Component (Q)')\n", modulation_type);
    fprintf(script, "    plt.xlabel('Time (s)')\n");
    fprintf(script, "    plt.ylabel('Amplitude')\n");
    fprintf(script, "    plt.grid(True)\n\n");
    
    fprintf(script, "    plt.tight_layout()\n");
    fprintf(script, "    plt.show()\n\n");
    
    fprintf(script, "    # Создание созвездия\n");
    fprintf(script, "    I_symbols = I_samples[::1000]  # Берем по одному sample на символ\n");
    fprintf(script, "    Q_symbols = Q_samples[::1000]\n\n");
    
    fprintf(script, "    plt.figure(figsize=(8, 8))\n");
    fprintf(script, "    plt.scatter(I_symbols, Q_symbols, s=100, c='red', marker='o')\n");
    fprintf(script, "    plt.axhline(y=0, color='k', linestyle='-', alpha=0.3)\n");
    fprintf(script, "    plt.axvline(x=0, color='k', linestyle='-', alpha=0.3)\n");
    fprintf(script, "    plt.grid(True)\n");
    fprintf(script, "    plt.title('%s Constellation Diagram')\n", modulation_type);
    fprintf(script, "    plt.xlabel('In-phase (I)')\n");
    fprintf(script, "    plt.ylabel('Quadrature (Q)')\n");
    fprintf(script, "    plt.axis('equal')\n");
    fprintf(script, "    plt.xlim(-1.5, 1.5)\n");
    fprintf(script, "    plt.ylim(-1.5, 1.5)\n");
    fprintf(script, "    plt.show()\n\n");
    
    fprintf(script, "if __name__ == '__main__':\n");
    fprintf(script, "    plot_modulation()\n");
    
    fclose(script);
}

int main() {
    // Исходная битовая последовательность
    int bits_seq[] = {1, 1, 0, 0, 0, 1, 1, 0, 1, 1, 1, 0};
    int bits_seq_len = sizeof(bits_seq) / sizeof(int);

    printf("Битовая последовательность: ");
    for(int i = 0; i < bits_seq_len; i++) {
        printf("%d ", bits_seq[i]);
    }
    printf("\n\n");

    // BPSK модуляция
    printf("=== BPSK МОДУЛЯЦИЯ ===\n");
    double* bpsk_result = BPSK_modulation(bits_seq, bits_seq_len);
    
    FILE* bpsk_samples = fopen("bpsk_samples.txt", "w");
    if(bpsk_samples != nullptr) {
        for(int i = 0; i < bits_seq_len * 2; i += 2) {
            fprintf(bpsk_samples, "(%.0f,%.0f), ", bpsk_result[i], bpsk_result[i + 1]);
        }
        fclose(bpsk_samples);
        printf("BPSK samples saved to bpsk_samples.txt\n");
    }

    // Подготовка данных для графиков BPSK
    double* bpsk_I = (double*)malloc(sizeof(double) * bits_seq_len);
    double* bpsk_Q = (double*)malloc(sizeof(double) * bits_seq_len);
    for(int i = 0; i < bits_seq_len; i++) {
        bpsk_I[i] = bpsk_result[i * 2];
        bpsk_Q[i] = bpsk_result[i * 2 + 1];
    }
    save_plot_data("bpsk_plot_data.txt", bpsk_I, bpsk_Q, bits_seq_len, 1000);
    create_python_plot_script("BPSK");
    printf("BPSK plot data saved to bpsk_plot_data.txt\n");
    printf("Run 'python plot_signal.py' to view BPSK plots\n\n");

    free(bpsk_I);
    free(bpsk_Q);

    // QPSK модуляция
    printf("=== QPSK МОДУЛЯЦИЯ ===\n");
    double* qpsk_result = QPSK_modulation(bits_seq, bits_seq_len);
    
    FILE* qpsk_samples = fopen("qpsk_samples.txt", "w");
    if(qpsk_samples != nullptr) {
        for(int i = 0; i < bits_seq_len; i += 2) {
            fprintf(qpsk_samples, "(%.0f,%.0f), ", qpsk_result[i], qpsk_result[i + 1]);
        }
        fclose(qpsk_samples);
        printf("QPSK samples saved to qpsk_samples.txt\n");
    }

    // Подготовка данных для графиков QPSK
    int qpsk_symbols_count = bits_seq_len / 2;
    double* qpsk_I = (double*)malloc(sizeof(double) * qpsk_symbols_count);
    double* qpsk_Q = (double*)malloc(sizeof(double) * qpsk_symbols_count);
    for(int i = 0; i < qpsk_symbols_count; i++) {
        qpsk_I[i] = qpsk_result[i * 2];
        qpsk_Q[i] = qpsk_result[i * 2 + 1];
    }
    save_plot_data("qpsk_plot_data.txt", qpsk_I, qpsk_Q, qpsk_symbols_count, 1000);
    create_python_plot_script("QPSK");
    printf("QPSK plot data saved to qpsk_plot_data.txt\n");
    printf("Run 'python plot_signal.py' to view QPSK plots\n\n");

    free(qpsk_I);
    free(qpsk_Q);

    // Освобождение памяти
    free(bpsk_result);
    free(qpsk_result);

    printf("Для построения графиков выполните:\n");
    printf("1. Для BPSK: скопируйте содержимое bpsk_plot_data.txt в plot_data.txt и запустите 'python plot_signal.py'\n");
    printf("2. Для QPSK: скопируйте содержимое qpsk_plot_data.txt в plot_data.txt и запустите 'python plot_signal.py'\n");

    return 0;
}