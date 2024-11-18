#!/usr/bin/env python
import csv
from pathlib import Path
results = []
results_path = Path(__file__).absolute().parent / 'figures/results.csv'
output_path = str(Path(__file__).absolute().parent / "figures" / "rate.png")
print("results=", results_path)
print("output=", output_path)
with open(results_path) as csvfile:
    reader = csv.DictReader(csvfile)
    for row in reader:
        results.append(row)

import matplotlib.pyplot as plt
for compressor_id in set(i['compressor_id'] for i in results):
    y = [float(i['compression_ratio']) for i in results if i['compressor_id'] == compressor_id]
    x = [float(i['psnr']) for i in results if i['compressor_id'] == compressor_id]
    print(compressor_id, x, y)
    plt.plot(x,y, label=compressor_id)
plt.xlim((30,80))
plt.ylim((0,1500))
plt.xlabel("psnr")
plt.ylabel("compression_ratio")
plt.legend()
plt.tight_layout()
plt.savefig(output_path)

