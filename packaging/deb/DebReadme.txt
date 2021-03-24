----------------------------------------
 Package Info:
----------------------------------------
These DEB packages are meant for installation on Debian or *Ubuntu (GNU Linux based) operating systems.

Hopsan will be installed to /opt/hopsan
A "shortcut" to HopsanGUI should end up under the Science, Engineering or Education category in your menu.
HopsanCLI, network executables and libraries can be found in /opt/hopsan/bin

----------------------------------------
 What version to choose:
----------------------------------------
The deb packages have been build in 32-bit and 64-bit for a few different distributions.
If you do not know which *Ubuntu version you have, run the following commands in a terminal:

Ubuntu version: "lsb_release -a"
Architecture: "uname -m", (x86_64 means 64-bit, choose amd64)


----------------------------------------
 How to Install:
----------------------------------------
There are different installation methods. Method 1 or 2 is recommended. Use method 5 if you do not have install permission.

Method 1:
In Ubuntu, double click the package file to open it in the "Ubuntu Software Center" where you can choose to install it.
Dependencies will be automatically installed.

Method 2:
Install in the terminal using apt, "sudo apt install ./hopsan-***.deb",  Be sure to include ./ if the deb file is in the current directory.
Dependencies will be automatically installed.

Method 3:
If apt does not work for you, you can use a tool such as "gdebi" or "QApt" (KDE) to install the package.
"apt-get install gdebi"

Method 4:
You can also install manually using dpkg directly, "dpkg -i hopsanPackageName".
But you will need to install dependencies manually, dpkg will tell you what you need. (Use apt-get install for dependencies)

Method 5:
If you do not have permission to install, you can unpack the .deb contents by using "dpkg -x hopsan-x.x.x.deb".
You can find all Hopsan files in the "data.tar.gz" archive.


----------------------------------------
 How to Upgrade:
----------------------------------------
Download a new version and then just install it over the older version.


----------------------------------------
 How to Remove:
----------------------------------------
Method 1:
Use the Ubuntu Software Center to uninstall.

Method 2:
Use a terminal to uninstall, "apt-get remove hopsan" followed by "apt-get autoremove" to automatically remove dependencies.
Dependencies will only be removed if install method 1,2 or 3 was used, and if no other package is using them.


----------------------------------------
 Additional Information:
----------------------------------------
Hopsan will be installed to /opt/hopsan
To avoid that /opt is removed when Hopsan is uninstalled a .hopsan_placeholder file will be create in the /opt directory.
This placeholder file will be automatically removed after Hopsan has been uninstalled.
