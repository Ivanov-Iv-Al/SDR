import numpy as np
import matplotlib.pyplot as plt
import pandas as pd
import os


def plot_iq_data(filename, modulation_type):
    """Простая визуализация I/Q данных"""

    # Проверяем существование файла
    if not os.path.exists(filename):
        print(f"Ошибка: Файл {filename} не найден!")
        print("Убедитесь, что файл находится в правильной папке")
        return

    # Чтение данных
    try:
        data = pd.read_csv(filename)
        print(f"Файл {filename} успешно загружен")
    except Exception as e:
        print(f"Ошибка при чтении файла: {e}")
        return

    # Проверяем наличие нужных колонок
    required_columns = ['real', 'imag']
    for col in required_columns:
        if col not in data.columns:
            print(f"Ошибка: В файле нет колонки '{col}'")
            print(f"Доступные колонки: {list(data.columns)}")
            return

    # Извлечение I и Q компонентов
    i_data = data['real'].values
    q_data = data['imag'].values

    print(f"Загружено {len(i_data)} сэмплов")
    print(f"Модуляция: {modulation_type}")

    # Создание двух графиков
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(12, 5))

    # 1. Диаграмма созвездия
    ax1.scatter(i_data, q_data, alpha=0.6, s=10)
    ax1.set_xlabel('I компонент')
    ax1.set_ylabel('Q компонент')
    ax1.set_title(f'Созвездие {modulation_type}')
    ax1.grid(True)
    ax1.axhline(y=0, color='k', linestyle='-', alpha=0.3)
    ax1.axvline(x=0, color='k', linestyle='-', alpha=0.3)

    # 2. I и Q во времени (первые 100 точек)
    samples_to_show = min(100, len(i_data))
    time_axis = np.arange(samples_to_show)
    ax2.plot(time_axis, i_data[:samples_to_show], 'b-', label='I', alpha=0.7)
    ax2.plot(time_axis, q_data[:samples_to_show], 'r-', label='Q', alpha=0.7)
    ax2.set_xlabel('Время (сэмплы)')
    ax2.set_ylabel('Амплитуда')
    ax2.set_title('I и Q компоненты')
    ax2.legend()
    ax2.grid(True)

    plt.tight_layout()
    plt.show()


# Создаем тестовые данные для проверки
def create_test_file():
    """Создает тестовый файл если нужно"""
    # BPSK данные
    bits = np.random.randint(0, 2, 200)
    i_data = np.where(bits == 0, 1, -1)
    q_data = np.zeros_like(i_data)

    df = pd.DataFrame({
        'bits': bits,
        'real': i_data,
        'imag': q_data
    })

    df.to_csv('test_iq_data.csv', index=False)
    print("Создан тестовый файл: test_iq_data.csv")


# Основной код
print("Программа визуализации I/Q данных")

# Создаем тестовый файл для демонстрации
create_test_file()

# Запускаем визуализацию
plot_iq_data('iq_data_qpsk.csv', 'BPSK')

plot_iq_data('iq_data_qpsk_spread.csv', 'BPSK')

# Если у вас есть свой файл, раскомментируйте эту строку:
# plot_iq_data('ваш_файл.csv', 'BPSK')