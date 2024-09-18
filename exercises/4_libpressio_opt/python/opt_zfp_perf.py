#!/usr/bin/env python
import libpressio
import numpy as np
from pathlib import Path
from pprint import pprint

input_path = Path(__file__).parent / "../../datasets/CLOUDf48.bin.f32"
input_data = np.fromfile(input_path, dtype=np.float32).reshape(100, 500, 500)

decomp_data = input_data.copy()

try: 
    comp = libpressio.PressioCompressor("opt",
        early_config={
          "composite:plugins": ["time", "size", "error_stat"],
          "opt:compressor": "zfp",
          "opt:metric": "composite",
          "opt:search": "fraz",
        },
        compressor_config={
          "opt:do_decompress": 0,
          "opt:objective_mode_name": "min",
          "zfp:execution_name": "omp",
          "zfp:metric": "time",
          "opt:max_iterations": 12,
          "opt:inputs": ["zfp:omp_threads", "zfp:omp_chunk_size"],
          "opt:output": ["time:compress_many"],
          "opt:upper_bound": [12.0, 512.0],
          "opt:lower_bound": [1.0, 1.0],
          "opt:is_integral": [1, 1],
        }
    )
    first = True
    for bound in [1e-6, 1e-5, 1e-4, 1e-3, 1e-2, 1e-1]:
        comp.set_options({"pressio:abs": bound})
        compressed_data = comp.encode(input_data)
        decomp_data = comp.decode(compressed_data, decomp_data)
        metrics = comp.get_metrics()
        pprint(metrics)
        if first:
            first = False
            comp.set_options({
              "opt:search": "guess",
              "opt:search_metrics": "composite_search",
              "opt:prediction": metrics['opt:input'],
            })
except libpressio.PressioException as e:
    print("libpressio had a problem:", e)
    exit()


