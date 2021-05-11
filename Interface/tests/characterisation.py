import glob
import sys
import re
import numpy as mp
import matplotlib as mpl
import matplotlib.pyplot as plt

results = {}

for directory in sys.argv[1:]:
    filenames = glob.glob(directory + "/*.device.log")
    with open(filenames[0]) as f:
        content = f.readlines()
        content = [x.strip() for x in content]
        for line in content:
            m = re.search(r'SEL at ([^\s]+) ([^\s]+) V - ([^\s]+) mA', line)
            if m:
                offset = m[1]
                threshold = m[2]
                current = m[3]

                if not offset in results:
                    results[offset] = ([], [])
                results[offset][0].append(float(threshold))
                results[offset][1].append(float(current))

print(results)

for key, value in results.items():
    plt.plot(value[0], value[1], label="offset = " + key + " V")

plt.xlabel("Threshold voltage (V)")
plt.ylabel("Current (mA)")
plt.grid()
plt.legend()

plt.show()