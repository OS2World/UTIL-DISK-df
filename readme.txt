The DF utility
==============

Overview
--------

DF displays statistics on disk volume usage. It is similar to one of the
UNIX programs of the same name.

The information displayed includes the volume letter, total, used and
available space on the volume (in kilobytes), size of an allocation
unit, percentage of space used, volume label and file system type.

Using the program
-----------------

Synopsis: df [options] [volume] ...
 where:
    options are:
    -h       display help

 and:
    volume   is one or more volume letters (with optional colon)

If no volumes are specified, information is displayed about all non-removable
volumes.

Versions
--------
1.0     - Initial version.
1.1     - Fixed for volumes over 4GB.
1.2     - Fixed failure to check for error gettig file system details.

Bob Eager
rde@tavi.co.uk
12th April 2003

