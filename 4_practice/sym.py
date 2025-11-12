import sys
import matplotlib.pyplot as plt
import numpy as np
import re

def main():
    #check command arguments
    if len(sys.argv) > 3:
        print(f"Programm expected one argumet, but received {len(sys.argv)}")
        sys.exit(1)
    #file name
    input_file = sys.argv[1]
    
    #modulation type
    modulation_type = sys.argv[2]
    
    if modulation_type == "bpsk":
        Rs = 1
        
    elif modulation_type == "qpsk":
        Rs = 2

    else:
        sys.exit(1)

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

    
    I_symbols = []
    Q_symbols = []
    Fs = 1000
    
    for sample in complex_samples:
        I_symbols.extend([sample.real] * Fs)
        Q_symbols.extend([sample.imag] * Fs)

    t = np.arange(0, len(I_symbols)/Fs, dt/Fs)
    
    print(t)

    plt.subplot(2, 1, 1)
    plt.step(t, I_symbols)
    plt.title(f"{modulation_type.upper()} I Signal")
    plt.xlabel("t")
    plt.ylabel("A")
    plt.grid(True)
    
    plt.subplot(2, 1, 2)
    plt.step(t, Q_symbols)
    plt.title(f"{modulation_type.upper()} Q Signal")
    plt.xlabel("t")
    plt.ylabel("A")
    plt.grid(True)
    
    plt.show()
    
    f = 1
    
    I_comp = I_symbols*np.cos(2*np.pi*f*t)
    Q_comp = Q_symbols*(-np.sin(2*np.pi*f*t))
    
    
    plt.subplot(3, 1, 1)
    plt.step(t, I_comp)
    plt.title(f"In-phase component")
    plt.xlabel("t")
    plt.ylabel("A")
    plt.grid(True)
    
    plt.subplot(3, 1, 2)
    plt.step(t, Q_comp)
    plt.title(f"Quadrature component")
    plt.xlabel("t")
    plt.ylabel("A")
    plt.grid(True)
    
    
    carrier_signal = I_comp + Q_comp 
    
    plt.subplot(3, 1, 3)
    plt.plot(t, carrier_signal)
    plt.title(f"Carrier signal, f = {f}")
    plt.xlabel("t")
    plt.ylabel("A")
    plt.grid(True)
    plt.show()

if __name__ == '__main__':
    main()