# Linking Against SZ3

First, build and install SZ3.
Then, build the SZ3 example as follows:

```
$ build/cmake ..
$ build/make
```
Note: 
* If your SZ3 is installed in a customized folder, please use -DCMAKE_PREFIX_PATH=Your_SZ3_INSTALL_PATH to specify the path when calling cmake.
* You may need to install zstd if cmake cannot find it.

The sz3demo.cpp shows an example of how to write customized compressor. The compressor contains three major components: Decomposition, Encoding, and Lossless. In this example,
* Decomposition converts data into another type/domain to make it easier for lossless compression. This example has two  methods for demonstration. 
  * TruncationDecomposition is a simple method which cast the float data to int data. As a result, the error bound always equals to 1.
  * SZBioMDDecomposition is a specialized decomposition method that will be integrated in GROMACS for compression biology data.
  * The method can by changed by setting DEMO_MODE demo = BIO or DEMO_MODE demo = TRUNCATION in sz3demo.cpp;
  * A best practice to create your own decomposition method is to follow the code style of the truncation method.
* Encoding compress the data losslessly by method such as Huffman or fix-length. In this example, it uses Huffman encoding. 
* Lossless further compress the data by existing state-of-the-art lossless compressors. In this example, it sses ZSTD.

In terms of data, it generates a 3D data with random values, compresses it, and reconstructs it to check the errors.
The compression ratio and errors are displayed in the output.
