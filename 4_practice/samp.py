import sys
import matplotlib.pyplot as plt
import numpy as np
import re

def main():
    #check command arguments
    if len(sys.argv) > 2:
        print(f"Programm expected one argumet, but received {len(sys.argv)}")
        sys.exit(1)
    #file name
    input_file = sys.argv[1]

    #template for paring
    complex_template = r'(-?\d+)\s*,\s*(-?\d+)'

    #lists for I/Q samples
    complex_samples = []

    #time in ms bw receives IQ samples
    dt = 1
    #open file
    with open(input_file, "r") as file:
        for line in file:
            #find numbers
            pairs = re.findall(rf"{complex_template}", line)

            for i,q in pairs:
                #get real and image parts
                complex_samples.append(complex(int(i), int(q)))

    start = 0
    stop = len(complex_samples) * dt
    amps = [abs(x) for x in complex_samples]
    real = [x.real for x in complex_samples]
    imag = [x.imag for x in complex_samples]

    timeline = np.arange(start, stop, dt).tolist()

    print(complex_samples)

    plt.plot(timeline, amps)
    plt.title("S(t)")
    plt.xlabel("t")
    plt.ylabel("A")
    plt.grid(True)
    plt.show()

    plt.plot(timeline, imag)
    plt.title("S(t)")
    plt.xlabel("t")
    plt.ylabel("A")
    plt.grid(True)
    plt.show()


if __name__ == '__main__':
    main()