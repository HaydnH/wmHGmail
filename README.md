## wmHGmail-1.00 release
Author: Haydn Haines
		
  
## Description
wmHGmail is a Window Maker dockapp to monitor your unread Gmail count using OAuth2 authentication.


## Screenshot
![Alt text](/wmHGmail.gif?raw=true)



## Requirements
wmHGmail requires the following libraries:

    X11/X.h (xorg-x11-proto-devel)
    X11/xpm.h (libXpm-devel)
    X11/extensions/shape.h (libXext-devel)
    cairo.h (cairo-devel)
    curl/curl.h (libcurl-devel)
    json-c/json.h (json-c-devel)

DNF Command for Fedora users:

    # dnf install xorg-x11-proto-devel libXpm-devel libXext-devel cairo-devel libcurl-devel json-c-devel


## Install:
1) git clone https://github.com/HaydnH/wmHGmail.git
2) cd wmHGmail/src/
3) make 
4) make install (must be root) 
5) wmHGmail & (or 'wmHGmail -h' for help, or 'man wmHGmail' for the man page)



## Files
| File			| Description 					|
| --------------------- | --------------------------------------------- |
| **README.md**		| This file. 					|
| **INSTALL**		| Installation instructions. 			|
| **HINTS** 		| Hints about what you can do with wmHGmail. 	|
| **BUGS**		| Bug reports. 					|
| **CHANGES** 		| Change history. 				|
| **COPYING**		| GNU General Public License Version 2. 	|
| **TODO**		| Wish list. 					|
	

## Copyright
wmHGmail is licensed through the GNU General Public License.
Read the COPYING file for the complete GNU license.
