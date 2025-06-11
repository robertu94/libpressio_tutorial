# 1. The Basics of LibPressio

The goal of this part of the tutorial is to get you acquainted with LibPressio's
features for abstracting between compressors.  This tutorial is has examples in C and Python.  The instructions are largely the same except the python files end in `.py` and have slightly different formatted output.
In each directory, you should
have 3 files:

1. list_compressors -- lists the compressors, metrics, and IO modules
   compiled with this version of LibPressio.
2. get_compressor_options -- shows the list of compressor options available
   for each compressor
3. basics -- demonstrates how to call a compressor -- specifically SZ --
   using libpressio's higher level python interface


First run `./list_compressors`.  You should get output that looks something
like this.  Please note that as versions and compilation options this output may
have slightly different output.  Please ensure that `sz` and `zfp` are present
in the compressors list.

```console
# ./list_compressors
compressors ['SZauto', 'bit_grooming', 'blosc', 'chunking', 'digit_rounding', 'fpzip', 'magick', 'many_dependent', 'many_independent', 'many_independent_threaded', 'mgard', 'noop', 'resize', 'sample', 'sz', 'transpose', 'zfp']
metrics ['autocorr', 'composite', 'diff_pdf', 'error_stat', 'external', 'kl_divergence', 'ks_test', 'kth_error', 'mask', 'memory', 'noop', 'pearson', 'printer', 'region_of_interest', 'size', 'spatial_error', 'time']
io ['copy_template', 'csv', 'empty', 'hdf5', 'iota', 'mmap', 'noop', 'numpy', 'petsc', 'posix', 'select']
```

This output lists the implementations of the three major abstractions exposed by
the python interface.

1. Compressors -- what actually perform compression
2. Metrics -- which measure the performance of compressors in various ways
3. IO -- which help get standard types of files formats into and out of formats
   that LibPressio understands.
   
Next, run `get_compressor_options sz` this will list the compressor options
for the SZ compressor.  Again your output may very slightly across versions of
the compressor and LibPressio.

```console
# ./get_compressor_options sz
runtime settings
{'sz:abs_err_bound': 0.0001,
 'sz:accelerate_pw_rel_compression': 1,
 'sz:app': 'SZ',
 'sz:config_file': None,
 'sz:config_struct': None,
 'sz:data_type': None,
 'sz:error_bound_mode': 4,
 'sz:error_bound_mode_str': None,
 'sz:exafel:bin_size': 0,
 'sz:exafel:calib_panel': b'',
 'sz:exafel:num_peaks': 0,
 'sz:exafel:peak_size': 0,
 'sz:exafel:peaks_cols': b'',
 'sz:exafel:peaks_rows': b'',
 'sz:exafel:peaks_segs': b'',
 'sz:exafel:sz_dim': 0,
 'sz:exafel:tolerance': 0.0,
 'sz:gzip_mode': 3,
 'sz:lossless_compressor': 1,
 'sz:max_quant_intervals': 65536,
 'sz:metric': 'noop',
 'sz:pred_threshold': 0.9900000095367432,
 'sz:prediction_mode': None,
 'sz:protect_value_range': 0,
 'sz:psnr_err_bound': 90.0,
 'sz:pw_rel_err_bound': 0.001,
 'sz:quantization_intervals': 0,
 'sz:rel_err_bound': 0.0001,
 'sz:sample_distance': 100,
 'sz:segment_size': 36,
 'sz:snapshot_cmpr_step': 5,
 'sz:sol_id': 101,
 'sz:sz_mode': 1}
compile-time settings
{'pressio:stability': 'stable',
 'pressio:thread_safe': 1,
 'sz:error_bound_mode_str': ['abs',
                             'abs_and_pw_rel',
                             'abs_and_rel',
                             'abs_or_pw_rel',
                             'abs_or_rel',
                             'norm',
                             'psnr',
                             'pw_rel',
                             'rel',
                             'rel_and_pw_rel',
                             'rel_or_pw_rel',
                             'vr_rel'],
 'sz:pastri': 0,
 'sz:random_access_enabled': 0,
 'sz:timecmpr': 0,
 'sz:write_stats': 0}
```

LibPressio divides configuration into two types:

1. Runtime Configuration -- also called options in the lower-level python as
   well as the C/C++ bindings.  The are modifiable at runtime.
2. Compile Time Configuration -- The reflect compilation options that cannot be
   changed without recompiling the libraries involved.

Each compressor has its own configuration options with their own
interpretations.  Compressor options that are specific to a compressor begin
with a prefix.  For SZ that prefix is `sz`.  Configuration options that are
possible for any compressor to implement begin with the `pressio` prefix.
You can find the meaning of the various compressor options [in the documentation.](https://robertu94.github.io/libpressio/pressiooptions.html)

Lastly run `./basics`.  This should execute the SZ compressor with an
assortment of different error bounds, and report some basic statistics about the
errors that were introduced by the compressors and the resources used.

```console
# ./basics.py

bound=1e-06, metrics={
    "composite:compression_rate": 233100.2331002331,
    "composite:compression_rate_many": null,
    "composite:decompression_rate": 476190.4761904762,
    "composite:decompression_rate_many": null,
    "error_stat:average_difference": -7.94428545037717e-09,
    "error_stat:average_error": 6.66561610708336e-08,
    "error_stat:difference_range": 1.999999312829459e-06,
    "error_stat:error_range": 9.99999883788405e-07,
    "error_stat:max_error": 9.99999883788405e-07,
    "error_stat:max_rel_error": 0.000488292296970302,
    "error_stat:min_error": 0.0,
    "error_stat:min_rel_error": 0.0,
    "error_stat:mse": 3.632067750660244e-14,
    "error_stat:n": 25000000,
    "error_stat:psnr": 80.62486211740608,
    "error_stat:rmse": 1.9057984548897724e-07,
    "error_stat:value_max": 0.0020479534287005663,
    "error_stat:value_mean": 8.596909993949838e-06,
    "error_stat:value_min": 0.0,
    "error_stat:value_range": 0.0020479534287005663,
    "error_stat:value_std": 5.2587475532997e-05,
    "size:bit_rate": 0.92011584,
    "size:compressed_size": 2875362,
    "size:compression_ratio": 34.77822966290853,
    "size:decompressed_size": 100000000,
    "size:uncompressed_size": 100000000,
    "time:check_options": null,
    "time:compress": 429,
    "time:compress_many": null,
    "time:decompress": 210,
    "time:decompress_many": null,
    "time:get_options": 0,
    "time:set_options": 0
}
... <snipped remaining output>
```

Note that ordering can matter.  Metrics are called from in order from first to last then last to first around each call.
This means if you want to measure time, you need to do that as the **last** call in composite plugins.


## Exercises

1. Review the documentation for SZ.  Try repeating the experiment in `basic.py` or `basic.c` with SZ's Value Range relative mode. What happens to the error statistics?  (note, get_compressor_options.py may help you here)
2. Review the documentation for ZFP.  Try repeating the experiment in `basic.py` or `basic.c` with ZFP's accuracy mode. What happens to the error statistics? (note, get_compressor_options.py may help you here)
3. Try changing the shape of the data passed into the compressors.  What happens to the error statistics?
