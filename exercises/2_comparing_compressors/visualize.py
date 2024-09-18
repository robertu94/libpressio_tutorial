#!/usr/bin/env python

import matplotlib.pyplot as plt
import argparse
import numpy as np
import sys
from pathlib import Path
print("external:api=5")

WORKDIR = Path(__file__).parent

TYPES = {
        "int8": np.int8,
        "int16": np.int16,
        "int32": np.int32,
        "int64": np.int64,
        "uint8": np.uint8,
        "uint16": np.uint16,
        "uint32": np.uint32,
        "uint64": np.uint64,
        "float": np.float32,
        "double": np.float64
}


def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("--api", type=int)
    parser.add_argument("--config_name")
    parser.add_argument("--input", type=Path)
    parser.add_argument("--decompressed", type=Path)
    parser.add_argument("--dim", type=int, action="append", default=[])
    parser.add_argument("--type", type=lambda name: TYPES[name], choices=TYPES.values())
    return parser.parse_known_args()[0]


def select_slice(arr: np.ndarray) -> np.ndarray:
    if len(arr.shape) == 2:
        return arr
    elif len(arr.shape) == 3:
        x, y, z = arr.shape
        return arr[x//2, :, :]


def plot_data(args):
    print(args, file=sys.stderr)
    dims = list(reversed(args.dim))
    # load the data
    uncompressed = np.fromfile(args.input, dtype=args.type).reshape(dims)
    decompressed = np.fromfile(args.decompressed, dtype=args.type).reshape(dims)
    orig_value_range = uncompressed.max() - uncompressed.min()
    vmin = uncompressed.min() - .1*orig_value_range
    vmax = uncompressed.max() + .1*orig_value_range

    uncompressed = select_slice(uncompressed)
    decompressed = select_slice(decompressed)
    errors = uncompressed - decompressed

    fig, ax = plt.subplots()
    ax.hist(errors.ravel(), range=(-.008*orig_value_range, .008*orig_value_range), bins=100)
    ax.set_title(args.config_name)
    fig.savefig(WORKDIR / "figures" / (args.config_name + "-hist.png"))

    # plot the error in place
    fig, ax = plt.subplots()
    ax.set_title(args.config_name)
    ax.imshow(np.abs(errors), vmin=0, vmax=orig_value_range*.01)
    fig.savefig(WORKDIR / "figures" / (args.config_name + "-err.png"))

    # plot a side-by-side visualization

    fig, ax = plt.subplots(1, 2)
    ax[0].set_title(args.config_name + " uncompressed")
    ax[0].imshow(uncompressed, vmax=vmax, vmin=vmin)
    ax[1].set_title(args.config_name + " decompressed")
    ax[1].imshow(decompressed, vmax=vmax, vmin=vmin)
    fig.savefig(WORKDIR / "figures" / (args.config_name + "-compare.png"))


if __name__ == "__main__":
    args = parse_args()
    if args.input:
        plot_data(args)

