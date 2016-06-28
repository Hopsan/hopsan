This library contains (more or less) experimental components that will be added to the default library in the future.
This makes it possible to easily make new components available in older versions of Hopsan.
That is, you do not need to build a new release of Hopsan just to make your new or updated component available.
This assumes that the new component do no make use of functionality that is only available in newer versions of Hopsan.

******************************
  Requirements:
******************************
  Hopsan 0.8.x or newer

******************************
  To add components:
******************************
1. Add the .hpp .xml .svg and other needed files directly under the correct type subdirectory, or create a new one if one a suitable one does not exist.
2. Run the generateLibraryFiles script "./generateLibraryFiles.py ." or double-click the 'generateLibraryFiles.bat' file on Windows to regenerate the library include files.
3. Add and commit the new and modified files to subversion.
   Note! If you added a new subdirectory, then make sure to add the new .h and .cci files created under this directory.

******************************
  To distribute this library:
******************************   
1. Export the directory from subversion
2. Run the generateLibraryFiles.py script (or .bat) to make sure tat the library files are refreshed (Python interpreter required)
3. The library can now be distributed

Note! After every commit to this library, a post-commit hook will take care of automatically creating a distribution zip, and upload it to 
      http://flumes.iei.liu.se/hopsan/files/extensionLibrary/


******************************
  To use this library:
******************************
1. You will need a Hopsan version with a compiler included, (or have setup the compiler path separately)
2. Click "Load external library" in the library widget in HopsanGUI, choose this directory to load it.
3. The library should be automatically compiled, a pop-up window will tell you if compilation was successful.
4. If you ever update the library files or make your own changes, you may need to manually trigger a recompilation in HopsanGUI.
   Use the right-click menu on the library.

