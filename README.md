An Image Processing Scheme (IPS) to Visualize and Analyze Time Resolved TEM Video Images
========================================================================================

This writeup is a short description of the initial implemenation of an image
processing scheme (software tool) for the visualization and analysis of time
resolved TEM video images.

This software has been written by the High Performance and Visualization Group
(HPCVG) in the Applied and Computational Mathematics Division (ACMD) of the
Information Technology Lab (ITL). This software was developed in collaboration
with the CNST-Nanofabrication Research Group.

This is a preliminary version of the software. There are possible bugs. Please
ensure you have a backup of your data prior to using this software.

This discussion is in these sections:

0. Install the Software
1. A Simple Scenario
2. The Organization of the Software and Some Conventions
3. The Processing Modules
4. The Display of Images and Atom Positions
5. Building the Software from Source (Advanced Users)

## 0. Installation Instructions

1. Download the latest release zip file from the [Releases
page](https://github.com/usnistgov/tem_analysis/releases)
2. Extract the contents of the zip file to a directory.
3. Run the `centroid.exe` executable.

## 1. A Simple Scenario

Here is a simple scenario of how you might use the software. In this example
scenario, we'll just use all of the defaults, but you should be aware that
there are various processing parameters that can be changed. Each of the
parameters will be discussed in more detail in subsequent sections.

* Create a folder to encompass the project. Let's call it "Example1"

* Create a sub-folder called "original" inside of the folder Example1

* Move or copy a sequence of raw TEM images into the sub-folder original.

* Start up the TEM Analysis software.

* Go to the File menu and select "New Project".  This brings up
        a dialog where you are asked to select a "base directory" and to
        specify a "short tag".
  - press the "Select Base Directory" button and select the Example1 folder
    that you created.
  - put "ex1" in the short tag field. (You could put anything you want.)
  - hit "Save". (You can use any file extension you want.)

* You should now see a list of "Data Processing Modules" at the right.

* Click on the module "Subset", then click execute. This uses the default
  settings to process the images in the folder "original" and places the
  output images in a sub-folder called "subset". By default, it removes
  duplicate images, singleton images, and any images which have differing R,
  G, and B channels.

* In the list of icons at the upper left, click on the eye icon.
  This brings up a file dialog box. Select the "subset" folder. You should see
  the first of the subsetted image sequence. You can use the controls below
  the image display to run through the image sequence, and you can use the -
  and + controls at the top to zoom in and out.

* Click on the module Subset button to close up the Subset 
  interface.

* Click on AtomCorrelation to open up it's interface, and hit the
  Execute button to run it.  By default it processes the images in the subset
  folder and it creates the subfolder atomCorrelation and places its output
  there.

* At any point, you can view the images as described above.

* Click on the AtomPosition module, and hit its Execute button.
  By default, this writes output to the sub-folder called atomPositions. In
  this case, the output is a set of files that give derived atom positions for
  each input file.

* Click on the RegisterImages module, then hit its Execute button.
  By default this produces transformations for each image that us to transform
  image coordinates between each image and the coordinate system of the first
  image in the sequence.

  The default inputs to the registration process are the images produced by
  the AtomCorrelation module.  The output transformations are placed in the
  sub-folder called registrationTransformations. By default it also transforms
  the input images and places the registered images into the folder called
  atomCorrelationRegistered.

  You will see an ImageJ window launch after clicking the Execute button. This
  is expected, and the ImageJ window should disappear after the registration
  is complete.

* Click on the ApplyRegistration module, and hit its Exectute 
  button. By default, this transforms the images in the subset folder based on
  the transformations created by the RegisterImages module. The output images
  are placed in a folder called subsetRegistered.

* We're now going to run ApplyRegistration again, but instead of
  applying the registration transformations to image data, we'll apply them to
  the atom position files that were created by the AtomPosition module. This
  is done in the ApplyRegistration interface with these steps:
  - Hit the "..." button to the right of the "Input Folder" field.
    This opens a dialog; select the atomPositions folder.
  - Hit the "..." button to the right of the "Output Folder" 
    field. This opens a dialog; in this dialog hit the button or icon for
    creating a new folder. Name the new folder "atomPosRegistered".  Select
    this folder in the dialog.
  - Hit the Execute button.  This will transform the coordinates
    in the files in the atomPositions folders and place its output in the
    atomPosRegistered folder.

* Go back to the image viewer and its set of controls.

* Hit the eye icon and select the subsetRegistered folder

* Hit the icon that looks like a bunch of dots and select the
  atomPosRegistered folder.

* You should see the atom positions overlayed as dots on top
  of the registered TEM images.  You can use the controls at the bottom to
  play through the image sequence and examine individual frames.  You can zoom
  in and out to look at more detail.  The other image viewer controls will be
  described below.

## 2. The Organization of the Software and Some Conventions

The user interface is divided into four sections:

  - a menu bar at the top, which includes a few basic operations
  - the viewer, which includes a large area for the display of images
    and related data with an icon bar at the top and a image sequence player
    at the bottom
  - the module interface at the right that is used to invoke the
    various processing steps
  - a log panel at the bottom where messages from the processing
    modules is displayed.

Note: In the discussion below, we use the term "folder" and "directory"
interchangeably. These two terms are intended to be entirely equivalent.

The software has the concept of a "project".  Projects are just all of the
files associated with a given TEM image sequence. These files include all of
the files produced by the processing steps included in this software.  A
project has a "base directory" and a "short tag". All of the files will be
placed under base directory (folder) and the short tag should be a short
string that will be used (when necessary) to make file names for output files.
You specify the base directory and short tag when you initiate a project. This
is described below.

To set up your project, we suggest that you create a folder for the project
(this will be the base directory), then a sub-folder called "original" in
which you place the raw TEM image sequence. (The software currently handles
bmp, png, tif, and gif images; it writes images as png or tif.)

This software is expecting to deal with sequences of TEM images, and we think
of an entire image sequence as being a single data set. A processing step is
likely to take an entire image sequence as input and produce another image
sequence as output. Each image sequence resides in it's own sub-folder
(sub-directory) under the project base directory. When you're asked to specify
input images to a processing step, you will typically select a folder; all of
the images within that folder will then be used as input.

We also use a naming convention for the files output by a processing step. The
names typically look like this:

    seqNum.shortTag.procTag.fileType

where:

- seqNum      is a zero-filled sequence number, e.g. "000020"
- shortTag    is determined by the project, e.g. "set01"
- procTag     is generated by the processing step and will
              typically indicate what was done to produce the data, e.g.
              "registered"
- fileType    is the extension that indicates the type of data
              in the file, e.g. "png" or "tif".

So a generated image file may have a name like: `000020.set01.registered.png`

We also generate data files of other types. For example when we derive atom
positions from a sequence of image files, we will generate individual data
files with names like: `000020.set01.atomPos.apos`.

Note that this naming is strictly a convention and our processing modules do
not require input file names to be in this format. In particular, we don't
expect that the raw TEM images that you bring into this software will conform
to this naming convention; however we do expect that the normal sorting order
for file names will put images into time-sequence order.

Even within the naming convention we describe above, we do not require that
the sequence numbers increment by one.  In other words, it is perfectly OK for
there to be gaps in the sequence numbering. This is important because it
enables us to remove bad frames while retaining consistent sequence numbers
between processing steps. In some contexts, the sequence numbers may serve as
time-stamps in the original image data.

To set up your project, we suggest that you create a folder for the project,
then a sub-folder called "original" in which you place the original TEM image
sequence. 

### Repository Layout
- centroid: main code, include modules
- data: sample data
- sdk: third party libraries
- old: initial versions of code, for reference

## 3. The Processing Modules

The data analysis provided by this tool is provided as a serise of "Modules",
which are accessed by the sub-panel at the right. In this initial version of
the software, there are five modules:

- Subset
- AtomCorrelation
- AtomPositions
- RegisterImages
- Apply Registration

This is typically the order in which you would execute the modules

You can access the parameters for each module by clicking on the bar for that
module.  This will open up a panel where you can specify the parameters. If
you want to hide this panel, just click again on the bar for the module.

After setting a module's processing parameters, you press the "Execute" button
to run the module.


Here's a description for each module:

### Subset

This module enables you to remove redundant frames and to crop the remaining
images. You would typically use this to process the raw TEM images to produce
a working set of images.

The parameters are:

- *Input Folder*            The folder with the images to be subsetted
  - Default: a folder named "original" under the project base folder

- *Output Folder*           The folder where the output subset images
                            will be written.
  - Default: a folder named "subset" under the project base folder

- *Remove Duplicates*       Remove any image that is identical to the
                            previous image

- *Remove Singles*          Removes any image that is present only
                            once in the image sequence

- *Remove Imgs with Color*  Removes any images that contain pixels
                            that are not strictly gray-level


- *Crop Lower/Upper X/Y*    Four numeric entries that indicate the
                            pixel coordinates of the cropping limit.
  - Defaults: 0, 0, 0, 0
  - If the crop coords are not changed and the defaults
    of 0,0,0,0 are used, then no cropping is done. We have noticed that it is
    not necessary to crop the images at this stage. In fact, in the cases that
    we've looked at, having the full image area available improves the
    subsequent image registration step.

### AtomCorrelation

This module generates images that have bright clusters of pixels that indicate
the likely locations of atoms in the images. This is done by generating a
template image that corresponds to a single atom and then determining the
correlation of that small template image at each position in each input image.

The parameters are:

- *Input Folder*            The folder with the images to be processed.
  - Default: folder named "subset" under the project base folder.

- *Output Folder*           The folder where the output atom correlation
                            images will be written.
  - Default: folder named "atomCorrelation" under the project base folder.

- *Atom Template Radius*    The radius of the atom in the template 
                            image that is generated.
  - Default: 3.0

This is the most compute-intensive step; it may take several seconds to
process each input image. The output images are TIFF files with floating point
correlation values at each pixel position.

It may be useful to play with the radius and see how it affects the detection
of atoms in the output correlation images.

Note: We plan to expand the capability of this module in various ways in
subsequent versions of the software.

### AtomPositions

This module processes the images produced by AtomCorrelation to produce lists
of (possible) atom positions. One list of X,Y positions is produced for each
input image.

Atom locations are determined by finding clusters of bright pixels. Each
cluster is assumed to correspond to an atom. A centroid X,Y position is
determined for each cluster and is written to an atom position list file.

The parameters are:

- *Input Folder*            The folder with the images to be processed.
  - Default: folder named "atomCorrelation" under the project base folder.

- *Output Folder*           The folder where the output atom position
                            files will be written.
  - Default: folder named "atomPositions" under the project base folder.

- *Percent Threshold*       The percent of the input image range to be
                            excluded. The range of correlation values in each
                            input image is determined and this parameter is
                            used to determine a threshold pixel value. Pixels
                            below the threshold are assumed to not be in any
                            atom cluster.
  - Default: 5%

### RegisterImages

This module registers a sequence of images; it determines a series of image
transformations that indicates how to spatially transform the each image into
the coordinate system of the first image in the sequence. This is currently
being accomplished by using ImageJ with the StackReg plugin.

Note that the input to this registration should typically be the correlation
images produced by AtomCorrelation. We have had good results using these
images for registration and rather bad results using the original TEM images
(even when cropped).

The parameters are:

- *Input Image Folder*       The folder with the images to be registered.
  - Default: folder named "atomCorrelation" under the project base folder.

- *Output Transform Folder*  The folder where the output image
                             transform files will be written. These transform
                             files describe the spatial transformation
                             (typically XY translation) that can be used to
                             transform between the pixel coordinates of the
                             first image and those of each subsequent image.
  - Default: folder named "registrationTransforms" under the project base
    folder.

- *Output Image Folder*     The folder where the transformed image
                            files will be written. Each input image is
                            resampled based on the transformation determined
                            in this processing step.
  - Default: folder named "atomCorrelationRegistered" under the project base
    folder.

- *Allow Rotation*          If this is checked then the method allows
                            rotations as well as translations to be introduced
                            as it tries to match up the images.
  - Default: Rotations not allowed (translation only)

### ApplyRegistration

This module applies the registration transforms generated by the
RegisterImages module. The registration transforms can be applied either to an
image sequence or to an atom position sequence (as output by the module
AtomPositions).

The parameters are:

- *Input Folder*             The folder with the images or atom position 
                             files to which to apply the registration
                             transforms.
  - Default: folder named "atomPositions" under the project base folder.

- *Input Transforms Folder*  The folder containing the registration
                             transform files as output by the module
                             RegisterImages.
  - Default: folder named "registrationTransforms" under the project base
    folder.

- *Output Folder*            The folder where the output image or atom
                             position files will be written.
  - Default: folder named "atomPosRegistered" under the project base folder.

*NOTE*: This is the module that you are most likely to want to run several
times with inputs and outputs that are different than the defaults.

In particular, you will probably want to run this again to process the
"subset" images to produce images that are visually like the raw TEM images
but that are registered to the same spatial coordinate system.

To do this, you could set:

- Input Folder:  "subset" sub-folder of the base project folder
- Output Folder: "subsetRegistered" sub-folder of the base project folder

## 4. The Display of Images and Atom Positions

This is the most visual and interactive part of the software. This enables you
to:
- look at an image
- play through the images
- zoom in and out
- overlay atom positions
- select and remove unwanted atoms
- look at overlaid trianglulations based on atom positions

There are a series of icons at the top of the display panel:

- Create a new project file

- Open an existing project file
  - When you create a new project, a project file is written. This enables to
    you to read that project file and to place you back in that project folder.

- Display an image sequence
  - Reads in an image sequence and displays it

- Display an atom position sequence
  - Reads in atom positions and overlays them on the images

- Zoom Out
- Zoom In

- Scroll a zoomed image by dragging
  - Enables you to scroll around a zoomed image just by clicking and dragging on
    the image. This does not work when you're in one of the select/deselect
    modes described below.

- Select atom positions through all frame
- Deselect atom positions through all frame
- Select atom positions only in current frame
- Deselect atom positions only in current frame
  - These four selection modes let you sweep out a rectangular region to select
    and deselect atom positions. See additional comments below. Selected atoms
    are shown in green. Unselected atoms are shown in red.

- Remove selected atom positions 
  - This removes the selected (green) atoms from the display. See additional
    comments below.

- Turn on/off triangulation display
  - Displays a triangulation based on all of the atoms remaining in the display

- Write image based on current display
  - Creates an image file of what is currently displayed.

At the bottom of the display panel are some "VCR" controls that enable
you to play or step through the image sequence.

When you use the "Remove" feature, keep in mind that you are removing the
SELECTED atoms (green), not the unselected atoms. This means that if you want
to remove everything but a small region, it may take several selection and
removal steps.  One way of handling this might be to select ALL of the atoms,
then unselect your region of interest. At this point the selected atoms are
the atoms you are not interested in, so you can use the remove icon to delete
them. Also, note that it's easy to select one atome at a time to fine-tune
your selection.  We don't yet have an undo function here, so it's best to
proceed with care.  Of course, you can just re-read the atom position files to
restore the full set of atom positions.

## 5. Build Instructions

The [Releases page](https://github.com/usnistgov/tem_analysis/releases) will
always have the latest released version. If you need to build from source, the
instructions are below. Building from source is for advanced users only.

### Build Dependencies
- QT 5
- CMake 2.8
- Java 7 JRE

### UNIX Build Instructions
1. cd sdk
2. ./compile
3. cd ../centroid
4. mkdir build
5. cd build
6. cmake ..
7. make -j8

### Windows Build Environment
1. Download Java SE 7 JRE and install with the default options.
   - Make sure the java\bin directory is present on your PATH
2. Download Qt (5.3 is the latest version of Qt5 at this writing) and install
   with the default options.
3. Download CMake 2.8 and installed with the default options.
4. Download the latest version of Git
   - This is only necessary if you are going to use the Git repository to
     update the source tree. Configuring Git on Windows to use SSH is not
     covered here.
5. Download and install 7-zip: http://www.7-zip.org/
   - The sdk\compile.bat file expects to use 7-zip to extract the tar files.
6. To open a Qt-enabled command prompt:
   - Start -> All Programs -> Qt -> 5.3 -> MinGW 4.8 (32bit) ->
     Qt 5.3 for Desktop (MinGW 4.8 32bit)
7. Verify you can run 7z, cmake, and java from the command prompt.

### Windows Build Instructions
1. cd sdk
2. compile.bat
3. cd ..\centroid
4. mkdir build
5. cd build
6. cmake -G "MinGW Makefiles" ..
7. make -j8

## Contributors
- Tomasz Bednarz, CSIRO, tomasz.bednarz@gmail.com
- Wesley Griffin, NIST, wesley.griffin@nist.gov
- Zahra Hussaini, NIST
- John Hagedorn, NIST

## PI

Wesley Griffin
301.975.8855
wesley.griffin@nist.gov

**Updated:** 2017 Mar 30

