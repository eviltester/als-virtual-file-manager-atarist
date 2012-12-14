Instructions for Feedbackware version 1.1 of AVFM:

SPECIAL ATARI WORLD EDITION: Has the cut & paste bugs fixed and code
to squish up the path names when displayed.

Al's Virtual File Manager:
-------------------------

Warning, these are stopgap documents until a full manual is created.
Chances are that that won't be until a full version of the program is
created.

They can be a little random when reading them but the program is
simple and I'm sure you'll get to grips with it pretty quickly.

Excuse the spelling mistooks but I didn't spull chick this duckument.

Disclaimer
----------
I make no promises about this program other than that I wrote it. 
I like it and use it and it has never caused me any trouble. 

However if you invite it into your home and it does something to your
disk drive (it shouldn't!) or your carpet (it won't!) then I will not
be held responsible.

Nope not me.

Well now that that little bit of pleasantness is out of the way.


What is AVFM?
-----------

It's a program launcher... with a difference (otherwise there would be
no point writing it)




Why is it called a Virtual File Manager instead of a program launcher?
----------------------------------------------------------------------

'cause that's cooler and that is the side of the coin that came up.

But it looks more like the normal desktop file management functions
than any other program launcher so...

Also it handles virtual files. AVFM introduces the concept (and I use
the word loosely here) of a virtual file. A virtual file is something
which AVFM thinks is a file, it has a link to a physical file but it
also allows files to have 30 character names (virtually) and to have
comments and packages associated with them.




How to install
--------------

Put the AVFM.PRG, AFVM.RSC in the same directory.

Run the prog.

The program will automatically create a file called AVFM.VFS. This
file holds all the information that you create and is affectionately
called the virtual file system.

AVFM automatically creates a backup of this file before it saves any
new data. This file is called AVFM.VFB. (It shouldn't be necessary
but this is still an early version of the software and bugs might
have crept in.


Keyboard Commands:
------------------
INSERT:     Add Item
UNDO:       Edit Highlighted item
DELETE:     Delete Highlighted item
CURSOR UP:  Highlight previous item
CURSOR DOWN: Highlight next item
ENTER:      Enter a directory or run a prog
ESC:        Go up a directory or exit
CNTRL C:    copy highlighted
CNTRL X:    Cut highlighted
CNTRL V:    Paste copy/cut item




Mouse commands:
---------------
Double click:   on dir (enter dir)
                on prog (run prog)
                on blank line (add item)

Right mouse button & click or double click - edit item




Menu commands:
--------------
Save:   Saves the virtual file system
Other:  Executes another program
Quit:   Quit the program

Cut:    Cut the highlighted item
Copy:   Copy the highlighted item
Paste:  Paste a previously cut or copied item

Edit:   Edit the highlighted item
New:    Create a new item




Using the prog.
---------------

Add your virtual progs. either dirs, progs or docs. 

There is no limit (that I know of) to the number of dirs you can have,
or progs. 

Click on directories to enter them, click on progs to execute them,
click on docs to execute the associated prog and pass the doc name as
a parameter.



Things in the args:
--------------------

A number of things will be expanded in the arguments these are a %
followed by a code (f,F,p,P,%)

%%  -   %
%f  -   file selector and insert just the filename
%F  -   file selector and insert filename with full path
%p  -   fileselector and insert just the path
%P  -   fileselector and insert the path with filemask

e.g an args string of

-x %F

will pass -x and whatever filename you choose on the fileselector.



Working dirs
------------

Normally this will be the program dir.

But some progs are happy to allow this to be a different dir (e.g
tempus). If you do this with tempus the tempus will have it's file
selector set to the working dir.

e.g working dir is E:\docs\textfile\

but tempus is in D:\tempus\tempus2.prg

the tempus fileselector is set to e:\docs\textfile\

Protext hates this and thinks it isn't installed but some progs are
happy enough like this.



What else?
----------

Args Ask... when the program is run you get asked for args, the
special codes still work here (%f,%F etc)

To input a working dir or program path, click on the FS box to call
up the file selector.

The .vfs is automatically saved when you exit or run a prog (well you
are asked if you want to save it, you can still say no!)

a backup of .vfs is made into .vfb before .vfs is written.

Other (in the menu bar) lets you run other progs.


What is a virtual Directory?
----------------------------

A virtual directory is a directory in AVFM which can contain
programs, documents or other directories (all virtual)

What is a virtual Program?
--------------------------

A virtual program is an entry in the virtual file system which has a
link to a physical program on the disk. double clicking on it runs
the program with the arguments specified in the virtual program
description.

What is a virtual Document?
---------------------------

Ah!

This is the main reason that Al's virtual file manager was created.
At the moment it is little more than a means of providing a virtual
program with a predefined argument (the virtual document) in fact you
can get exactly the same results as a virtual document by creating a
program with the document name as a parameter.

But in the future Al's virtual file manager will act as a simple
version control system for these virtual documents.

This is currently being developed and is the main reason for writing
this system.

Why?
----

Why what?

Just, why!

Oh! Because I have never used an alternative desktop (I'm a memory
miser). I used to use ST_Whiz2 (nice, I liked it a lot) but I
couldn't run Kobold 2 as an accessory from it. So I developed this
instead.

I was originally planning on using this as a testing ground for
routines that I would incorporate into a version control system but
the version control system will end up being incorporated into this.

I rarely hit the desktop. All my file copying, moving and tidying etc
is done using kobold2, and all my program execution is done using
AVFM.


Future Plans
------------

The document version control system.

Multitasking compatibility (instead of using pexec to execute the
progs). I don't have a multitasking operating sys yet, but I will do
eventually so when I do that will be included.

File Selector support, to use all the replacement file selectors to
their full advantage.

Kobold Support, this will be used when the version control system is
added to move and backup the various versions of files.

On line help.

AV Protocol - oooh you could drag progs onto the AVFM!
no need to hit that insert key (docs too)!

NKCC

font support (speedo and non-speedo & system)

extra options & prompts for floppy drive users

proper cut and paste

directory support (what? I'll explain in a later version! :)

directory scanning agent (Again I'll explain later!)

One day I hope to find a modern GEM library that does everything I
want, and when I do this prog will have a good interface and windowed
dialogues. Until then I can only say sorry to those people who
multitask and are annoyed by the modality of this program.

etc.

What is feedbackware?
---------------------

Oh yeah! :]

Well basically it is freeware but... I WANT FEEDBACK.

Tell me what you think, tell me what you like, use and test the
program, break it, snap it, love it, hate it. Whatever.

Just tell me about it.

That is the only way this program will improve the way you want it
to. Plus it provides me with encouragement. Otherwise future versions
will be geared towards me only.

without feedback I may never tidy up the manual (I don't need one, I
know how it works!)

Without feedback I may never release the bug fixes (Heck I can avoid
the bugs if I know where they are! (and I know where they are!))

Eventually this program will be a fully featured document control
system for normal documents. If you think you would like to see this
feature or have some ideas for what it should do then get in touch.

Eventually this prog will be shareware (when it has the version
control system). But at the moment I am Altruistic Al and you get
this for no moneys, only a little bit of response.

This is an experiment and such altruism may never come again from
Al's software development factory. (Hey if you're really good then I
might get around to releasing my hard drive boot program (what
another one? (yup!, but mine is great :)))

Thanx to all those people providing feedback it is all  useful and
pushes me forwards more than you could believe. The Future's list is
growing (and so is the bug list :(  ,  thank you)


Testing
-------

This program has been tested on an Atari ST 520FM (with 4 meg of
memory, high density floppy, and hard drive) in mono, low and medium
resolutions, with and without NVDI.

It was written using Lattice C 5.06.02 (One day I'll upgrade)


Thanks go to
------------
Denesh Bhabuta (sorry it aint shareware yet! and for mussin' up the
hensa abbreviations)
&
The author of ST_Whiz2 for providing me with a nice solution for a
while.
&
Everyone who provided feedback on my Guitar Reference Program. The
current version is down to you guys, and personal laziness. :]  
 

I think thats it.
-----------------

Any feedback to...

mr_richardson@cix.compulink.co.uk 

or

Alan Richardson
55 Alderbrook Road
Clapham South
London
SW12 8AD


Thanks,

Alan Richardson



This program and all associated stuff is copyrighted to Alan
Richardson in the year of our lord nineteen hundred and ninety five,
or thereabouts.
