import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

df = pd.read_csv('data-gestures/analog.csv')
fft_c = np.fft.rfft(df['Sample'])
psd_c = (np.abs(fft_c)**2)/(len(df['Sample'])*1000) #fs = 100Hz

print(df.head())


fig, axs = plt.subplots(3, 1, layout='constrained')

axs[0].plot(df['Time'], df['Sample'])
axs[0].set_title('Time vs Sample')
axs[0].set_xlabel('Time (s)')
axs[0].set_ylabel('Sample Value')

# Plotting the first 20 values of FFT and PSD on the same graph for comparison
sampleToPlot = 35
axs[1].plot(df['FFT'][:sampleToPlot], label='Embedded FFT')
axs[1].plot(fft_c[:sampleToPlot], label='Calculated FFT', linestyle='dashed')
axs[1].set_title('Frequency vs FFT')
axs[1].set_xlabel('Coefficient Index')
axs[1].set_ylabel('FFT Value')  
axs[1].legend()

axs[2].plot(df['PSD'][:sampleToPlot], label='Embedded PSD')
axs[2].plot(psd_c[:sampleToPlot], label='Calculated PSD', linestyle='dashed')
axs[2].set_title('Frequency vs PSD')
axs[2].set_xlabel('Coefficient Index')
axs[2].set_ylabel('PSD Value')
axs[2].legend()


plt.show()

# df['FFT_c'] = np.fft.rfft(df['Sample'])
# df['PSD_c'] = (np.abs(df['FFT_c'])**2)/(len(df['Sample'])*1000) #fs = 100Hz