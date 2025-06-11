#!/usr/bin/env python

from pathlib import Path
import json
import libpressio
import numpy as np

# load dataset, create output path
input_path = Path(__file__).parent / "../../datasets/CLOUDf48.bin.f32"
input_data = np.fromfile(input_path, dtype=np.float32).reshape(100, 500, 500)
decomp_data = input_data.copy()

for bound in np.logspace(start=-6, stop=-1, num=6):
    compressor = libpressio.PressioCompressor.from_config({
        # configure which compressor to use
        "compressor_id": "sz",
        # configure the set of metrics to be gathered
        "early_config": {
            "pressio:metric": "composite",
            "composite:plugins": ["size", "error_stat", "time"]
        },
        # configure SZ
        "compressor_config": {
            "pressio:abs": bound,
        }})

    # run compressor to determine metrics
    comp_data = compressor.encode(input_data)
    decomp_data = compressor.decode(comp_data, decomp_data)
    metrics = compressor.get_metrics()
    print(f"bound={bound:1.0e}, metrics={json.dumps(metrics, indent=4)}")
