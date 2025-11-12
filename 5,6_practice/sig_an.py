# [file name]: signal_analyzer.py
import sys
import matplotlib.pyplot as plt
import numpy as np
import re

def parse_complex_signal(filename):
    """
    Извлекает комплексные отсчеты из файла
    """
    signal_data = []
    pattern = r'\((-?\d+)\s*,\s*(-?\d+)\)'
    
    with open(filename, 'r') as file:
        content = file.read()
        matches = re.findall(pattern, content)
        
        for real_part, imag_part in matches:
            signal_data.append(complex(float(real_part), float(imag_part)))
    
    return signal_data

def plot_signal_characteristics(signal_data, filename):
    """
    Строит графики амплитуды, реальной и мнимой частей сигнала
    """
    time_axis = np.arange(len(signal_data))
    
    # Вычисляем характеристики сигнала
    amplitude = np.abs(signal_data)
    real_component = np.real(signal_data)
    imag_component = np.imag(signal_data)
    
    # Создаем графики
    fig, (ax1, ax2, ax3) = plt.subplots(3, 1, figsize=(12, 10))
    
    # График амплитуды
    ax1.plot(time_axis, amplitude, 'b-', linewidth=2, label='Амплитуда')
    ax1.set_title(f'Огибающая сигнала |{filename}|')
    ax1.set_xlabel('Временные отсчеты')
    ax1.set_ylabel('Амплитуда')
    ax1.grid(True, alpha=0.3)
    ax1.legend()
    
    # График реальной компоненты
    ax2.plot(time_axis, real_component, 'g-', linewidth=2, label='Реальная часть')
    ax2.set_title('Реальная компонента сигнала (I)')
    ax2.set_xlabel('Временные отсчеты')
    ax2.set_ylabel('Амплитуда')
    ax2.grid(True, alpha=0.3)
    ax2.legend()
    
    # График мнимой компоненты
    ax3.plot(time_axis, imag_component, 'r-', linewidth=2, label='Мнимая часть')
    ax3.set_title('Мнимая компонента сигнала (Q)')
    ax3.set_xlabel('Временные отсчеты')
    ax3.set_ylabel('Амплитуда')
    ax3.grid(True, alpha=0.3)
    ax3.legend()
    
    plt.tight_layout()
    plt.show()
    
    # Дополнительный график - диаграмма созвездия
    plt.figure(figsize=(8, 8))
    plt.scatter(real_component, imag_component, alpha=0.7, s=50)
    plt.axhline(y=0, color='k', linestyle='--', alpha=0.5)
    plt.axvline(x=0, color='k', linestyle='--', alpha=0.5)
    plt.grid(True, alpha=0.3)
    plt.title('Диаграмма созвездия сигнала')
    plt.xlabel('Реальная компонента (I)')
    plt.ylabel('Мнимая компонента (Q)')
    plt.axis('equal')
    plt.show()

def main():
    """
    Основная функция анализатора сигналов
    """
    if len(sys.argv) != 2:
        print("Использование: python signal_analyzer.py <файл_с_данными>")
        sys.exit(1)
    
    input_filename = sys.argv[1]
    
    try:
        # Чтение и анализ сигнала
        complex_signal = parse_complex_signal(input_filename)
        
        if not complex_signal:
            print("В файле не найдены комплексные отсчеты сигнала")
            sys.exit(1)
            
        print(f"Обработано {len(complex_signal)} комплексных отсчетов")
        
        # Визуализация характеристик сигнала
        plot_signal_characteristics(complex_signal, input_filename)
        
    except FileNotFoundError:
        print(f"Ошибка: Файл '{input_filename}' не найден")
        sys.exit(1)
    except Exception as e:
        print(f"Ошибка при обработке файла: {e}")
        sys.exit(1)

if __name__ == '__main__':
    main()