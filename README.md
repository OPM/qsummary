#Qsummary 

Qsummary is a lightweight application for plotting summary vectors from reservoir simulations. The main building blocks for this program is the Ecl/IO routines from opm-common and Qt5.

The applications Qsummary don't have a well design graphical user interface with command buttons and menus. If this is what you are looking for, please consider the ResInsight 

https://github.com/OPM/ResInsight

With Qsummary you can create a number of charts from the command line as shown in the example below

```
qsummary NORNE_ATW2013.ESMRY NORNE_ATW2013_SENS1.SMSPEC -v WOPT:B-* -z
```

This command will create 7 charts (WOPT for wells B-1BH, B-1H, B-2H, B-3H, B-4BH, B-4DH and B 4H) with two summary curves on all charts. 


- Qsummary supports both SMSPEC and ESMRY summary files. 
- Option -z ignors all summary vectors which only holds zero values.
- You can easily export all charts to a PDF file 
   * <ctrl> + p and specify file name in file save dialog
   * :pdf <file name> on the application command line 


Use option -h on the command line to get help one command line options, commands and key controls.

Clone opm-common and qsummary and make sure that these two repos a are located next to each other. Start with building opm-common before building qsummary.

