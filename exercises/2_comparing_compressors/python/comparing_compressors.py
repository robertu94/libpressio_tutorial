#!/usr/bin/env python

from pathlib import Path
from pprint import pprint
import csv
import libpressio
import numpy as np
import itertools
from mpi4py.futures import MPICommExecutor

# load dataset, create output path
input_path = Path(__file__).parent / "../../datasets/CLOUDf48.bin.f32"
input_data = np.fromfile(input_path, dtype=np.float32).reshape(100, 500, 500)


configs = [{
        "compressor_id": compressor_id,
        "compressor_config": {
            "pressio:abs": bound
        },
        "bound": bound
    } for bound, compressor_id in
        itertools.product(
            np.logspace(-7, -3, num=5),
            ["sz", "sz3", "mgard", "zfp"]
        )
    ]


def run_compressor(args):
    compressor = libpressio.PressioCompressor.from_config({
        # configure which compressor to use
        "compressor_id": args['compressor_id'],
        # configure the set of metrics to be gathered
        "early_config": {
            "pressio:metric": "composite",
            "composite:plugins": ["time", "size", "error_stat", "external"],
            "external:config_name": f"{args['compressor_id']}-{args['bound']:1.1e}",
            "external:command": str(Path(__file__).absolute().parent.parent / "visualize.py")
        },
        # configure the compressor
        "compressor_config": args['compressor_config']
        })

    # run compressor to determine metrics
    decomp_data = input_data.copy()
    comp_data = compressor.encode(input_data)
    decomp_data = compressor.decode(comp_data, decomp_data)
    metrics = compressor.get_metrics()

    return {
        "compressor_id": args['compressor_id'],
        "bound": args['bound'],
        "metrics": metrics
    }

with  open(Path(__file__).absolute().parent.parent / "figures/results.csv", 'w') as csvfile:
    with MPICommExecutor() as pool:
        fieldnames = ['compression_ratio', 'bound', "psnr", "compressor_id"]
        writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
        writer.writeheader()
        for result in pool.map(run_compressor, configs, unordered=True):
            writer.writerow({
                "compression_ratio": result['metrics']['size:compression_ratio'],
                "psnr": result['metrics']['error_stat:psnr'],
                "compressor_id": result['compressor_id'],
                "bound": result['bound'],
            })
            pprint(result)

