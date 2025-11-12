import sys
import matplotlib.pyplot as plt
import numpy as np
import re

def main():
    if len(sys.argv) != 2:
        print(f"Нужно указать один файл, а получено: {len(sys.argv)-1}")
        sys.exit(1)
    
    filename = sys.argv[1]
    amplitudes = []
    
    with open(filename, "r") as file:
        for line in file:
            numbers = re.findall(r'(\d+)', line)
            amplitudes.extend(numbers)
    
    time_points = np.arange(len(amplitudes))
    
    plt.figure(figsize=(10, 5))
    plt.plot(time_points, amplitudes, 'g-', linewidth=1)
    plt.title("График сигнала S(t)")
    plt.xlabel("Время (отсчёты)")
    plt.ylabel("Амплитуда")
    plt.grid(True, linestyle=':', alpha=0.7)
    plt.show()

if __name__ == '__main__':
    main()