import numpy as np
import librosa
from pydub import AudioSegment
import sys

def main():
    if len(sys.argv) > 2:
        print(f"Programm expected one argumet, but received {len(sys.argv)}")
        sys.exit(1)

    mp3_file = "test.mp3"
    pcm_file = sys.argv[1]

    # mp3 to pcm
    y, sr = librosa.load(mp3_file, sr=44100, mono=True)

    pcm_data = (y * 32767).astype(np.int16)

    pcm_data.tofile(pcm_file)


if __name__ == '__main__':
    main()