# 3. External Metrics

The goal of this part of the tutorial is to learn the basics of writing an external metric that can be used to analyze the difference between compressors.  This part of the tutorial is implemented in C, C++, and Python with the external metrics script being implemented in both C and Python.

In this directory you should have a couple of files:

1. `external_metric` which computes the external metric that we've written.
2. `run_external` which uses the external metric that we've written with SZ.

First, run `./run_external` and see what output you get.  You should get something like this:

```console
bound=1.0e-06
composite:compression_rate <double> = <empty>
composite:compression_rate_many <double> = <empty>
composite:decompression_rate <double> = <empty>
composite:decompression_rate_many <double> = <empty>
external:error_code <int32> = 0
external:proc_results_opts:post <double> = 2.28517e-12
external:proc_results_opts:pre <double> = 1.52727e-12
external:return_code <int32> = 0
external:runtime <double> = 0.357082
external:stderr <char*> = ""

bound=1.0e-05
composite:compression_rate <double> = <empty>
composite:compression_rate_many <double> = <empty>
composite:decompression_rate <double> = <empty>
composite:decompression_rate_many <double> = <empty>
external:error_code <int32> = 0
external:proc_results_opts:post <double> = 2.30747e-12
external:proc_results_opts:pre <double> = 1.52727e-12
external:return_code <int32> = 0
external:runtime <double> = 0.378958
external:stderr <char*> = ""
... <snip>
```

After that run ` ./external_metric --input ../../datasets/CLOUDf48.bin.f32 --decompressed ../../datasets/CLOUDf48.bin.f32 --dim 500 --dim 500 --dim 100 --type float`.  You should get output like this:

```console
external:api=5
pre=1.527247e-12
post=2.280860e-12
```

And if you run it without arguments which should give you the defaults:

```console
external:api=5
pre=0.0
post=0.0
```


Now, open up the `external_metric` code, and notice the following:

1. Before anything else we output the api version we support in this case `external:api=5`. This lets LibPressio know what version we support
2. If we get any other command line argument, we actually run the metric.
3. If we don't we just output what the names of the variables are.
4. Outputting a metric is as easy as printing a single line to the console with format `<name>=<value>\n`.

Next open up, `run_external` you'll notice that the code is eairly similar to `basics` from exercise 1.  All we changed is the code in the `NEW` comments to tell the library about the new metric.  Wasn't that easy!

## Exercises

1. Take the existing `external_metric` code provided an add the positive geometric mean.  Its formula is $\exp{\left(\frac{1}{N}\sum\log{|a_i|}\right)}$ where $a_i > 0$ 
2. Redo the previous exercise in a different programming language.
