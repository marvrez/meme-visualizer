# meme-visualizer
Visualize different ML and image processing memes

You can currently do the following:
* Generate covariance data
* Do PCA on data and visualize the eigenvectors pointing in the two direction with most variance
* Generate gaussian clusters
* Do K-Means(or K-Means++) Clustering and choose between L1, L2 and IOU as distance metrics.
* Load images and do different kinds of thresholding(binary, b/w, r/g/b).
* Find connected components in an image, based on its colors.
* Apply predefined convolution filters or customize your own filters.
* Visualize how the margins an SVM creates using either a gaussian or linear kernel changes when you add points and/or adjust an SVM's parameters.
* Generate linear data and perform linear or logistic regression on it

## Installation

You need CMake and SDL2 to build this project. When you have those dependencies, do the following:

```sh
git clone git@github.com:marvrez/meme-visualizer.git
cd meme-visualizer/
mkdir build
cd build
cmake ..
make
```

If the project is sucessfully built, you can run the program by using the following command:

```sh
./visualizer
```

TODO:
* (H)DBSCAN
