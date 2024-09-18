# 2. Comparing Compressors

The goal of this part of the tutorial is to understand how write code that targets multiple compressors simultaniously.  This section of the tutorial has examples in Python and C++.

In the directory, you should have the following files:

1. `./visualize.py` -- a script that generates a couple different visualizations.  We'll learn to write this file later, but for now know that it generates a couple of different visualizations for different compressor configurations.
2. `./python/compare_compressors.py` a script that shows how to compare the compressors using LibPressio's high level python interface.

First run `./compare_compressors.py`.  You should get some output that looks like this:

```console
    {'bound': 1e-06,
 'compressor_id': 'sz',
 'metrics': {'composite:compression_rate': 92506.93802035153,
             'composite:compression_rate_many': None,
             'composite:decompression_rate': 213219.61620469083,
             'composite:decompression_rate_many': None,
             'error_stat:average_difference': -7.94428545037717e-09,
             'error_stat:average_error': 6.66561610708336e-08,
             'error_stat:difference_range': 1.999999312829459e-06,
             'error_stat:error_range': 9.99999883788405e-07,
             'error_stat:max_error': 9.99999883788405e-07,
             'error_stat:max_rel_error': 0.000488292296970302,
             'error_stat:min_error': 0.0,
             'error_stat:min_rel_error': 0.0,
             'error_stat:mse': 3.632067750660244e-14,
             'error_stat:n': 25000000,
             'error_stat:psnr': 80.62486211740608,
             'error_stat:rmse': 1.9057984548897724e-07,
             'error_stat:value_max': 0.0020479534287005663,
             'error_stat:value_mean': 8.596909993949838e-06,
             'error_stat:value_min': 0.0,
             'error_stat:value_range': 0.0020479534287005663,
             'error_stat:value_std': 5.2587475532997e-05,
             'external:error_code': 0,
             'external:return_code': 0,
             'external:runtime': 3.691854284,
             'external:stderr': "Namespace(api=5, config_name='sz-1.0e-06', "
                                "decompressed=PosixPath('/home/runderwood/git/play/libpressio_tutorial/exercises/2_comparing_compressors/.pressiooutJ396bo'), "
                                'dim=[100, 500, 500], '
                                "input=PosixPath('/home/runderwood/git/play/libpressio_tutorial/exercises/2_comparing_compressors/.pressioinijr1Un'), "
                                "type=<class 'numpy.float32'>)\n",
             'size:bit_rate': 0.92011584,
             'size:compressed_size': 2875362,
             'size:compression_ratio': 34.77822966290853,
             'size:decompressed_size': 100000000,
             'size:uncompressed_size': 100000000,
             'time:check_options': None,
             'time:compress': 1081,
             'time:compress_many': None,
             'time:decompress': 469,
             'time:decompress_many': None,
             'time:get_options': 1,
             'time:set_options': 0}}
{'bound': 1e-06,
 'compressor_id': 'zfp',
 'metrics': {'composite:compression_rate': 72939.46024799417,
             'composite:compression_rate_many': None,
             'composite:decompression_rate': 140056.0224089636,
             'composite:decompression_rate_many': None,
             'error_stat:average_difference': 2.450353253564193e-11,
             'error_stat:average_error': 2.349406530304832e-08,
             'error_stat:difference_range': 7.767230272293091e-07,
             'error_stat:error_range': 3.916211426258087e-07,
             'error_stat:max_error': 3.916211426258087e-07,
             'error_stat:max_rel_error': 0.00019122560949752344,
             'error_stat:min_error': 0.0,
             'error_stat:min_rel_error': 0.0,
             'error_stat:mse': 1.2520077877666111e-15,
             'error_stat:n': 25000000,
             'error_stat:psnr': 95.25033122472536,
             'error_stat:rmse': 3.538372207338582e-08,
             'error_stat:value_max': 0.0020479534287005663,
             'error_stat:value_mean': 8.596909993949838e-06,
             'error_stat:value_min': 0.0,
             'error_stat:value_range': 0.0020479534287005663,
             'error_stat:value_std': 5.2587475532997e-05,
             'external:error_code': 0,
             'external:return_code': 0,
             'external:runtime': 4.088178271,
             'external:stderr': "Namespace(api=5, config_name='zfp-1.0e-06', "
                                "decompressed=PosixPath('/home/runderwood/git/play/libpressio_tutorial/exercises/2_comparing_compressors/.pressiooutM1FSrp'), "
                                'dim=[100, 500, 500], '
                                "input=PosixPath('/home/runderwood/git/play/libpressio_tutorial/exercises/2_comparing_compressors/.pressioinp0oICo'), "
                                "type=<class 'numpy.float32'>)\n",
             'size:bit_rate': 7.03059712,
             'size:compressed_size': 21970616,
             'size:compression_ratio': 4.551533739427242,
             'size:decompressed_size': 100000000,
             'size:uncompressed_size': 100000000,
             'time:check_options': None,
             'time:compress': 1371,
             'time:compress_many': None,
             'time:decompress': 714,
             'time:decompress_many': None,
             'time:get_options': 0,
             'time:set_options': 0}}
... <snip>
```

As you can see, we have detailed statistics comparing ZFP and SZ.  You will also notice that the `figures` directory has been populated with histograms of the errors introduced, and spatial plots showing were errors where introduced.  The figures directory was popluated by the `visualize.py` script and demonstrates the power of custom user-defined metrics.

Now to how did we do this.  In the `comparing_compressors` examples we use MPI4py's MPICommExecutor or `libdistributed` in C++ to run a series of different configurations on different MPI ranks.
We have a function that `make_config` generates a configuration from an interator over the set of configurations wer are interested in.  From that, we can configure the compressor in the `run_compressor` function.

## Exercises

1. Compare SZ and ZFP's compression ratios, and the figures generated in the `figures` directory.  What is SZ better at preserving?  What is ZFP better at presserving?
2. Download some other datafiles from [SDRBench](https://sdrbench.github.io/).  Modify `comparing_compressors.py` to include these files.  Does your analysis of SZ or ZFP change with including these additional file types.
3. Add another configuration to this example (i.e. try MGARD or a different mode of SZ or ZFP), and run `comparing_compressors.py` again.  What do you find?
