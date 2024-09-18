#!/usr/bin/env python
print("external:api=5")
import argparse
import numpy as np

TYPES = {
    "int8": np.int8, "int16": np.int16, "int32": np.int32,
    "int64": np.int64, "uint8": np.uint8, "uint16": np.uint16,
    "uint32": np.uint32, "uint64": np.uint64, "float": np.float32,
    "double": np.float64, "byte": np.byte
}


parser = argparse.ArgumentParser()
parser.add_argument("--api", type=int)
parser.add_argument("--config_name")
parser.add_argument("--input")
parser.add_argument("--decompressed")
parser.add_argument("--dim", type=int, action="append")
parser.add_argument("--type", type=lambda x: TYPES[x], choices=list(TYPES.values()))
args = parser.parse_known_args()[0]

def nonzero_mean(data):
    return data[data != 0].mean()


if args.api is not None:
    # compute the metric
    dims = list(reversed(args.dim))
    input_data = np.fromfile(args.input, dtype=args.type).reshape(*dims)
    output_data = np.fromfile(args.decompressed, dtype=args.type).reshape(*dims)

    pre = nonzero_mean(input_data.astype(np.float64))
    post = nonzero_mean(output_data.astype(np.float64))

    print(f"pre={pre}")
    print(f"post={post}")
else:
    # output the defaults
    print("pre=0.0")
    print("post=0.0")
