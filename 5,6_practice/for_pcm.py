import numpy as np
import matplotlib.pyplot as plt
from pathlib import Path

def visualize_pcm_signal(file_path):
    """
    Визуализирует I/Q компоненты из PCM файла
    """
    # Чтение бинарных данных
    raw_data = np.fromfile(file_path, dtype=np.int16)
    
    # Разделение на I и Q компоненты
    in_phase = raw_data[0::2]      # Четные элементы - I компонента
    quadrature = raw_data[1::2]     # Нечетные элементы - Q компонента
    
    sample_indices = np.arange(len(in_phase))
    
    # Создание комплексного сигнала
    complex_signal = in_phase + 1j * quadrature
    signal_envelope = np.abs(complex_signal)
    
    # Построение графиков
    fig, ((ax1, ax2), (ax3, ax4)) = plt.subplots(2, 2, figsize=(15, 10))
    
    # I компонента
    ax1.plot(sample_indices[:1000], in_phase[:1000], 'b-', linewidth=1)
    ax1.set_title('In-Phase Component (I) - Первые 1000 отсчетов')
    ax1.set_xlabel('Отсчеты')
    ax1.set_ylabel('Амплитуда')
    ax1.grid(True, alpha=0.3)
    
    # Q компонента
    ax2.plot(sample_indices[:1000], quadrature[:1000], 'r-', linewidth=1)
    ax2.set_title('Quadrature Component (Q) - Первые 1000 отсчетов')
    ax2.set_xlabel('Отсчеты')
    ax2.set_ylabel('Амплитуда')
    ax2.grid(True, alpha=0.3)
    
    # Огибающая сигнала
    ax3.plot(sample_indices[:2000], signal_envelope[:2000], 'g-', linewidth=1)
    ax3.set_title('Огибающая сигнала')
    ax3.set_xlabel('Отсчеты')
    ax3.set_ylabel('Амплитуда')
    ax3.grid(True, alpha=0.3)
    
    # Диаграмма созвездия
    ax4.scatter(in_phase[:5000], quadrature[:5000], alpha=0.6, s=1)
    ax4.set_title('Диаграмма созвездия (5000 отсчетов)')
    ax4.set_xlabel('I компонента')
    ax4.set_ylabel('Q компонента')
    ax4.grid(True, alpha=0.3)
    ax4.axis('equal')
    
    plt.tight_layout()
    plt.show()
    
    # Статистика сигнала
    print(f"Анализ файла: {file_path}")
    print(f"Всего отсчетов: {len(raw_data)}")
    print(f"Комплексных samples: {len(complex_signal)}")
    print(f"Диапазон I компоненты: [{in_phase.min()}, {in_phase.max()}]")
    print(f"Диапазон Q компоненты: [{quadrature.min()}, {quadrature.max()}]")
    print(f"Средняя амплитуда: {signal_envelope.mean():.2f}")

def main():
    """
    Основная функция визуализатора PCM данных
    """
    file_path = "../build/received_data.pcm"
    
    try:
        if not Path(file_path).exists():
            print(f"Файл {file_path} не найден")
            return
        
        visualize_pcm_signal(file_path)
        
    except Exception as e:
        print(f"Ошибка при обработке файла: {e}")

if __name__ == '__main__':
    main()