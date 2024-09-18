#!/usr/bin/env python
import cupy as cp
import numpy as np
import uuid
import libpressio as lp
from pprint import pprint
from pathlib import Path
#TODO Remove
__file__='.'

#load the data and move it to the GPU
input_path = Path(__file__).parent / "../../datasets/CLOUDf48.bin.f32"
input_data = np.fromfile(input_path, dtype=np.float32).reshape(100, 500, 500)
gpu_input = cp.array(input_data)
gpu_output = cp.ndarray(gpu_input.shape, gpu_input.dtype)

#configure the compressor
comp = lp.PressioCompressor("zfp",
    compressor_config={
        "zfp:rate": 16.0,
        "zfp:execution_name": "cuda",
    },
    early_config={
        "pressio:metric": "composite",
        "composite:plugins": ["time", "size"],
        "time:metric": "error_stat"
    }
)

#optimization: first determine how much output to allocate to skip allocation on compression
empty_input = lp.pressio.data_new_empty(lp.pressio.float_dtype, lp.pressio.vector_uint64_t(input_data.shape))
output_shape = comp.encode(empty_input)
compressed = cp.ndarray(
            lp.pressio.data_dimensions(output_shape),
            lp.pressio_dtype_to_python(lp.pressio.data_dtype(output_shape))
        )
lp.pressio.data_free(output_shape)


#run compression and decompression
compressed = comp.encode(gpu_input, compressed)
gpu_output = cp.asarray(comp.decode(compressed, gpu_output))

#copy to the host
io = lp.PressioIO("posix", io_config={"io:path": f"/tmp/{uuid.uuid1()}"})
io.write(gpu_output)

#get metrics for compression
pprint(comp.get_metrics())


