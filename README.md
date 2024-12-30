# Bagfile Parser
## Initial Setup
There is a script in this package that can be used to install the package dependencies. This script can be run using the folling command inside the root of this package.
```
./install_depends.sh
```
If you are less familiar with ROS and linux you can run these following commands to do the full setup of this package. These commands need to be run inside the src/ directory of a ROS2 workspace.
```
git clone https://gitlab.com/kat0028/bagfile_parser_qt.git
cd bagfile_parser_qt
./install_depends.sh
```

## Running the Package
This package creates and run parsers for ros2 bagfiles. Currently only SQLite3 bags can be run in the parser, but this will be updated in the near future. 

This package compiles as part of a ROS workspace and can be run using 
```
ros2 run bagfile_parser_qt bagfile_parser_qt
```

This will open a GUI menu that can be used to configure a parser to handle a given a bagfile. 

The GUI should be navigated from top to bottom. Not all steps are dependent on previous ones but many are. All menus besides the select bagfile either dynamically react to other choices or remember previous inputs for convenience. The one button that needs to be pressed every time is the "Select Bagfile" button since the parsers directly access information from this step. 

The data files (.csv, .mat) are output to the same directory that the bagfile directory is located in. The output location will be configurable in the future.

## Select Bagfile
This button opens a menu for selecting a bagfile using the Ubuntu files system. The interface only allows the selection of directories. To select a bagfile, navigate to where the bag's .db3 file is located. Once in that folder, press the "open" button at the top right of the window. This will select that folder as the bagfile folder.

After this is complete the path to the bagfile will be displayed under the button on the main window. If there was a problem loading the bag, an error output should be displayed in the terminal that ran this program.

## Select Topics to Parse
Once a bagfile is selected, this button will pull up a checkable list to select which topics should be parsed from the bag. Due to parsing times and data storage space it may be advantageous to only select a subset of the available topics. 

* Note: if a bag was loaded in a previous instance of this executable, the logfile from that instance may still exsit. You should select a bagfile in your window before doing anything else.

## Select Dependencies
The parsers that are created using this tool are structured as a ROS package. As a result they need to be set up with a list of dependencies. This tool automatically scans the bagfile topic's message namespace data for recommended dependencies. These are displayed after pressing the button. After verifying that all dependencies shown are valid click "Approve Recommendations."

After this a new window will pop up. This window exists to allow the user to modify the list of dependencies in addition to the recommendations. At any point while this window is visible, the "Show Tracked Dependencies" button can be pressed to see the list of current tracked dependencies.

If this system is relaunched there is no issue accepting the recommendations again. Duplicate names are not tracked in the system.

### Adding Dependencies
New dependencies can be added by typing them in the editable window (case sensitive) and pressing "Add Dependency." 

### Removing Dependencies
Dependencies can be removed from the list using the "Remove Dependencies" button. This will open a new window with a checkbox list. Select the dependencies you wish to remove and press "Remove Selected Depends." 

### Reset
Pressing "Reset Tracked Dependencies" will clear all tracked dependencies and start over with the recommended dependency window.

## Manage Package Dependencies
In the previous step the list of dependencies for the parser package was configured. This step exists to locate external packages that are required. In almost every case these will be custom message packages but there are a few exceptions where custom message packages may have dependency on code packages as well. <!--while this is typically considered poor practice this tool needs to be ready for those suboptimal conditions-->

This step requires the user to know which packages they will need to parse a bag that are not already available in the ROS libraries. A good test is to go back to the previous step and check each dependency to see if they exist in ROS by using ROS commands with the package name in them. Note: if a workspace is sourced in your terminal before you do this custom packages from that workspace may appear to be native ROS packages. 

Using the system that pops up when the button is pressed, you can add packages by navigating to the package location in the file system, similar to the bagfile selector. This window is also looking for the package directory. This will almost always be the directory that contains the package's CMakeLists.txt file.

Packages can be removed in this window using a similar method to the dependency list manager from the previous step. 

Before leaving this window, build the packages that were added. Once building begins a message will appear letting you know that the system is building. Do not click the window while this is running as it will probably through an OS "window has taken too long to respond" prompt<!--because I am bad at programming-->. This will be fixed soon but if it does appear you can verify that the packages are still building by checking the terminal this program was run from. Once the build is complete a message will appear to say that it was successfull. There is a chance<!--because colcon for some reason--> that it will always say successfull. Just verify this with the terminal window before leaving. If something failed it is likely that some dependency was missing. Just add this new dependency using the method from this step. It should only be needed here and not in the dependency list from the previous step.
The build needs to be successful to do the next step.

## Run Message Analysis 
This button configures resource files for the message structures used in the bagfile. As a user you only need to hit this button and wait until the window says ready and the bottom again. Once it is complete check the terminal window for messages indicating any errors. 

## Parser Menu
At this point the workflow diverts to two different methods of parsing: CSV and MATLAB. The CSV parser is newer and not as well tested. It experiences some edge case issues but it runs much faster. If you are being selective in your topic selection it should be safe to use. Once the CSV parser finishes it's initial parsing it can also generate a file that can be used to convert the CSV's to .mat files. This is the step where the most errors tend to occur, and mostly with messages with nested array fields. 

The MATLAB parser takes longer but is a more robust. It only writes directly to .mat files as well, whether you see this as an advantage or disadvantage. 

Both of these parsers can be run by stepping down through generate to build to run. Ensure that you wait for the ready prompt at the bottom of the window before starting the next step. The generate steps are extremely fast so it may almost look like nothing happened <!--I dont believe in putting in fake delays to make people feel better. I deal with cpp because its fast not because its easy-->. The CSV parser has the additional step where it generates a .m MATLAB file that can be ran in MATLAB to convert the .csv to a .mat with the original field names from ROS. 

## Reset Parser
This does a clean setup of the parser and restarts the window. When the window restarts it will still push output to the terminal but the process is no long attached. This means that you have to close the window in addition to pressing CTRL+C in the termianl. 