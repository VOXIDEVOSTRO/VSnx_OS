SO the driver will be orginating from the: (I may have to add enviourmental variables for this one later...)
/MODULES/SYS/DRIVERS/

MAIN driver order manager or also called hookorder.

this is what gonna gather information of the dvices and load the drivers
The way is by using a driver config file for each corresposding class and witin it a subclass...
Like simple single line one for each correponding one:
The CFG format will be something like this: eg.

CLASS:GPU:VBOX>/MODULES/SYS/DRIVERS/VBOX/VBOXSVGA/VBOXSVGA.ELF
CLASS:USB:EHCI>/MODULES/SYS/DRIVERS/USB/EHCI/EHCI.ELF

And in typical format it will be this:
CLASS:<Subclass>:<driver>:<path_to_driver>

also this config file wil be here: /KERNEL/CONFIGS/HOOKORDER.CFG

So using and sticking with this format we can have list or data base of drivers and organize based on its class and subclass
and the hookorder device detectors will look for thier class, and subclass ONLY and what ever path it is, we spawn up the driver. This is the most basic and simple we can get and maybe the best solution for the hookorder.