# GPU_SGM
---
A CUDA implementation performing Semi-Global Matching.

## Features
---
As it uses CUDA, we try to compute the disparity map at high speed.

## Requirements
GPU_SGM needs CUDA (compute capabilities >= 3.0) to be installed.  
Moreover, to build the sample, we need the following libraries:
- OpenCV
- OpenGL
- GLFW3
- GLEW

## Build Instructions

```
$ cd libSGM
$ mkdir build
$ cd build
$ cmake ../
$ make
```

## Sample Execution
```
$ pwd
.../GPU_SGM
$ cd build
$ cd sample/movie/
$ ./stereo_movie <left image path format> <right image path format> <disparity> <frame count>
left image path format: the format used for the file paths to the left input images
right image path format: the format used for the file paths to the right input images
disparity: the maximum number of disparities (optional)
frame count: the total number of images (optional)
```

"disparity" and "frame count" are optional. By default, they are 64 and 100, respectively.

Next, we explain the meaning of the "left image path format" and "right image path format".  
When provided with the following set of files, we should pass the "path formats" given below.
```
left_image_0000.pgm
left_image_0001.pgm
left_image_0002.pgm
left_image_0003.pgm
...

right_image_0000.pgm
right_image_0001.pgm
right_image_0002.pgm
right_image_0003.pgm
```

```
$ ./stereo_movie left_image_%04d.pgm right_image_%04d.pgm
```

The sample movie images available at
http://www.6d-vision.com/scene-labeling
under "Daimler Urban Scene Segmentation Benchmark Dataset"
are used to test the software.

## Authors
The "GPU_SGM Team": Zhang Handuo, Hasith

## License
Apache License 2.0
