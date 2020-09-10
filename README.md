# eHeat Source Code

## Overview

TBD

## Usage

### Configuring the Project

`make menuconfig`

* Opens a text-based configuration menu for the project.
* Use up & down arrow keys to navigate the menu.
* Use Enter key to go into a submenu, Escape key to go out or to exit.
* Type `?` to see a help screen. Enter key exits the help screen.
* Use Space key, or `Y` and `N` keys to enable (Yes) and disable (No) configuration items with checkboxes "`[*]`"
* Pressing `?` while highlighting a configuration item displays help about that item.
* Type `/` to search the configuration items.

Once done configuring, press Escape multiple times to exit and say "Yes" to save the new configuration when prompted.

### Compiling the Project

* `make all` - compiles the app and bootloader and generates a partition table based on the configuration
* `make bootloader` - compiles the bootloader only
* `make app` - builds the app only

### Flashing the Project

* `make flash` - updates both the application and bootloader
* `make bootloader-flash` - updates the bootloader only
* `make app-flash` - updates the application only

### Erasing Flash

`make erase_flash`

### Viewing the Serial Output

`make monitor`




[![Lucidtron](http://git.lucidtron.com/uploads/appearance/header_logo/1/Lucidtron_WHITE-tight-350x40.png)](http://lucidtron.com)

Code in this repository is Copyright (C) 2016-17 Lucidtron Ltd., licensed under the Apache License 2.0 as described in the file LICENSE.

For more info visit the [Lucidtron Web Site](http://Lucidtron.com) or send an email to
tony@lucidtron.com
