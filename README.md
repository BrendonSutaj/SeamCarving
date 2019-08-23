# SeamCarving


Seam Carving is an algorithm for content-aware image resizing.
The project is written in C and can be used on files with PPM P3 Format.

## Getting Started

* Clone the Repository
* Run the command "gcc main.c -o seamcarving"
* Start the Seam Carving process on any PPM P3 format image with the command "./seamcarving \<Path to IMG\> -n \<Iterationcount\>"
* The output will be stored in a file named out.ppm
* Have fun!


### Prerequisites

* GCC Compiler (https://gcc.gnu.org/)
* ImageMagick (https://imagemagick.org/index.php), in case you want to convert your jpg files to PPM P3 format using the command "convert \<Path to JPG\> -compress none \<Path to PPM\>"

## Authors

* **Brendon Sutaj** 

## Seam Carving in Action
Original Image             |  Seam Carving (200 Iterations)
:-------------------------:|:------------------------------:
![](Images/aurora.jpg)     |  ![](Images/auroraSC.jpg)
![](Images/landscape.jpg)  |  ![](Images/landscapeSC.jpg)
