## Fast Approximation of Eigenproblem of Laplace-Beltrami Operator from Triangle Mesh

We _approximate_ the lowest part of the _eigenvalues_ and _eigenfunctions_ of **Laplace-Beltrami operator**, to have a faster speed, less storage, and quicker mapping to world coordinates. Our general idea is to construct a **subspace** (from the original space) that can approximate the _low frequency (smooth)_ functions on the manifold. Then we will solve the eigenproblem _restricted to such subspace_.

Here is main component of our approximation scheme:
1. Construct a subspace
   1. sampling
   2. radial basis functions
   3. partition of unity
2. Solve the eigenproblem in subpace
   1. low-dim eigenproblem
   2. lift the eigenfunction (only when necessary/as requested since it's expensive for memory)

Please note that this program requires **Eigen**, **LibIGL**, and an **eigensolver** (e.g. **CUDA's cuSOLVER** function).

## How to compile and run our implementation

We use **CMake** for easy installation/deployment on your machine, by creating a CMakeFiles.txt that hopefully do (almost) everything to make our code runs in your machine. Here are the steps to get our program runs:

1. **Clone** our repository **recursively** using `git clone https://github.com/a-nasikun/FastSpectrum --recursive`.
2. Ensure that you have **CUDA set-up** on your machine. You can download from [here](https://developer.nvidia.com/cuda-zone "CUDA Developer Zone") if you haven't done so.
3. **Update dependency** submodule(s). Among many external libraries in LibIGL, we'll be using eigen, glfw, glad, and imgui only. During our test, the commit version of eigen in libIGL is an old one and doesn't support `Eigen::SparseMatrix.` We can solve this by simply updating the eigen, using `git submodule update --remote external\eigen`.
4. Run the Cmake command and it should be working and **running** without any problem.
   If some headers/libraries are missing (perhaps because you locate the wrong folder), you could specify it directly in your application. For example, in Visual Studio you could go to `Project` > `Properties` > `C/C++` > `General` > `Additional Include Directories`.

Having followed those steps, our program **should compile and run** well. This program is still under continuous development, so please expect some errors/bugs/mistakes within it. If you have any problem compiling it or having some nice suggestion(s) or question(s), please drop me an **email** at `a.nasikun[at]tudelft.nl`.

## SGP 2018 (Symposium on Geometry Processing 2018)

This work was presented at [SGP 2018](https://sgp2018.sciencesconf.org/resource/page/id/10#s4 "SGP 2018") at Paris TelecomTech, organized by EuroGraphics and ACM SIGGRAPH, and will be published at [Computer Graphics Forum (CGF) Vol.37(5)](https://onlinelibrary.wiley.com/loi/14678659 "CGF Volume 37 Issue 5"). You can read the paper from [our website](https://graphics.tudelft.nl/ahmad-nasikun/ "Fast Approximation of Laplace-Beltrami Eigenproblems") for a more detailed explation of our approximation algorithm.

If you use our application in your work, please kindly cite our work.
```
@article{nasikun2018fastspectrum,
  title={Fast Approximation of Laplace-–Beltrami Eigenproblems},
  author={Nasikun, Ahmad and Brandt, Christopher and Hildebrandt, Klaus},
  journal={Computer Graphics Forum},
  volume={37},
  number={5},
  year={2018},
  publisher={Wiley}
}
```
