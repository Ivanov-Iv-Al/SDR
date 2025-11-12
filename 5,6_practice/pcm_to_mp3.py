import numpy as np
import librosa
from pydub import AudioSegment
import sys

def main():
    if len(sys.argv) > 2:
        print(f"Programm expected one argumet, but received {len(sys.argv)}")
        sys.exit(1)

    pcm_file = sys.argv[1]
    mp3_file = "pcm_to_mp3.mp3"

    pcm_data = np.fromfile(pcm_file, dtype=np.int16)

    audio = AudioSegment(
        data=pcm_data.tobytes(),
        sample_width=2,      # 2 байта = 16 бит
        frame_rate=44100,    # частота дискретизации
        channels=1           # моно
    )

    audio.export(mp3_file, format="mp3", bitrate="192k")


if __name__ == '__main__':
    main()