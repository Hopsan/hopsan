This library contains (more or less) experimental components that will be added to the default library in the future.
This makes it possible to easily make new components available in older versions of Hopsan.
That is, you do not need to build a new release of Hopsan just to make your new or updated component available.

To add components:

1. Add the .hpp .xml .svg and other needed files directly under the correct type subdirectory, or create a new one if one a suitable one does not exist.
2. Run the generateLibraryFiles script "./generateLibraryFiles.py ." or double-click the 'generateLibraryFiles.bat' file on Windows to regenerate the library include files.
3. Add and commit the new and modified files to subversion.
   Note! If you added a new subdirectory, then make sure to add the new .h and .cci files created under this directory.
   
The library can now be exported and distributed.

To use this library:
Load this directory from within HopsanGUI, (make sure that you have Hopsan with a compiler). The library will then be automatically compiled and loaded.

