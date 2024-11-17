#!/usr/bin/env python

import multiprocessing
import libpressio as lp
import numpy as np
from pprint import pprint
input = np.fromfile("/home/runderwood/git/datasets/hurricane/100x500x500/CLOUDf48.bin.f32", dtype=np.float32).reshape(100, 500, 500)
SCRIPT = """
local psnr = metrics['error_stat:psnr'];
local cr = metrics['size:compression_ratio'];
local objective = 0;
if psnr ~= nil then
    if psnr > 80 then
        objective = cr;
    else
        objective = math.abs(psnr)-80;
    end
end
return "objective", objective
"""
inputs = ["switch:active_id", "pressio:abs"]
outputs = ["composite:objective", "size:compression_ratio", "error_stat:psnr"]
compressors = ["sz3", "zfp"]
compressor = lp.PressioCompressor.from_config({
    "compressor_id": "pressio",
    "early_config": {
        "pressio:compressor": "opt",
        "opt:compressor": "switch",
        "opt:search": "fraz",
        "opt:search_metrics": "noop",
        "switch:metric": "composite",
        "composite:plugins": ["size", "error_stat", "time"],
        "switch:compressors": compressors,
    },
    "compressor_config": {
        "composite:scripts": [SCRIPT],
        "opt:inputs": inputs,
        "opt:output": outputs,
        "opt:is_integral": [1, 0],
        "opt:lower_bound": [0, 1e-8],
        "opt:upper_bound": [len(compressors)-1, 1e-2],
        "opt:max_iterations": 30,
        "opt:objective_mode_name": "max",
        "pressio:abs": 1e-5,
        "fraz:nthreads": multiprocessing.cpu_count()
    }
})
decompressed = input.copy()
compressed = compressor.encode(input)
decompressed = compressor.decode(compressed, decompressed)

metrics = compressor.get_metrics()
configuration = dict(zip(inputs, metrics["opt:input"]))
configuration['switch:active_id'] = compressors[int(configuration['switch:active_id'])]
pprint({
    "configuration": configuration,
    "performance": dict(zip(outputs, metrics["opt:output"]))
})
