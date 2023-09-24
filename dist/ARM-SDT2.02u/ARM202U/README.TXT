ARM Unsupported Toolkit release 2.02u
=====================================

This ARM toolkit release is based on the ARM Software Development Toolkit
version 2.02 but it is distributed without charge under the terms of the
ARM Unsupported Toolkit End-User License, for non-commercial usage only in
Universities and other educational establishments.


Release contents
================

  202u_w32_v1.zip        Win32 release (for Windows 95 and NT3.5x)

  202u_sun_v1.tgz        SunOS release (.tar.gz format)


What is provided
================

  - Complete manual set provided in PDF format (for use with Acrobat)
    "Reference Manual"            ARM DUI 0020D
    "Programming Techniques"      ARM DUI 0021A
    "Windows Toolkit Guide"       ARM DUI 0022B
    "ARM Quick Reference Card"    ARM QRC 0001B

  - ARM Debugger for Windows (ADW) and ARM Project Manager (APM) for Win32

  - Full set of command-line tools for ARM:
        armasm        ARM Assembler
        armcc         ARM C compiler
        armlib        ARM Object Librarian
        armlink       ARM Linker
        armmake       ARM Make Utility (not supplied on SunOS)
        armprof       ARM Profiler Data Displayer
        armsd         ARM Source-level Debugger
        decaof        ARM Object File Decoder

  - Support for ARM Architecture v4
    (compiler and assembler support for halfword and signed load/store
    instructions)

  - ARMulator to simulate execution of ARM code (used by default)

  - Supports rebuilding of ARMulator memory system to accurately model target
    memory system, interrupts and peripherals

  - Remote debug capability to execute code on ARM evaluation boards
    (PIE60, PIE7)


Differences from ARM SDT 2.02
=============================

  - UNSUPPORTED

  - Non-commercial usage only

  - Not available for DOS, Windows 3.1x, Alpha/NT or HP-UX

  - No Thumb compiler (tcc) or assembler (tasm) supplied

  - No debug monitor (DEMON) source code

  - No C-library source code

  - Only 2 pre-built C-libraries supplied: armlib.32l & armlib.32b

  - Includes modified remote.dll to allow more reliable use of ARM evaluation
    boards (processes all windows messages to avoid lock-up problem when
    target program executes for several minutes, also fixes problems with
    certain transfer sizes)



Win32 Release
=============

To install the release you will need a copy of the PKZIP package or
equivalent.

   cd \
   pkunzip -d <zipfile>

This creates a directory \arm202u containing the ARM tools.  Please create a
new program manager group to contain the following windows applications:

   \arm202u\bin\apm.exe         (ARM Project Manager)
   \arm202u\bin\windbg.exe      (ARM Debugger for Windows)

The following environment variables need initialising from AUTOEXEC.BAT
(or, under Windows NT, using 'System' in 'Control Panels')

   ARMLIB           c:\arm202u\lib
   PATH             c:\arm202u\bin  (add to existing path)



SunOS release
=============

The binaries supplied are SunOS 4.1.3 executables.  These are compatible
with Solaris 2, except that you may get warning messages from the dynamic
linker (ld.so) about shared libraries being "older revision than expected".
Note that remote debugging is not possible when using Solaris 2.

To install the release you will require the gzip package.

   zcat 202u_sun_v1.tgz | tar xvf -

Note: zcat must be the version supplied with gzip

This creates a directory arm202u containing the ARM tools.  The following
environment variables need initialising for each toolkit user
(e.g. in their .cshrc):

   ARMLIB      <path>/arm202u/lib
   path        <path>/arm202u/bin  (add to existing path)



Structure of this release
=========================

   bin         contains tool binaries (either Win32 exes or SunOS binaries)
   examples    example programs, refer to "Programming Techniques" manual
   include     Standard headers
   lib         Standard libraries
   source      Rebuild kit for armulate.dll  (PC only)
   armsd       Rebuild kit for armsd (SunOS only)
   pdf         Documentation

The documentation is supplied in PDF format for use with Adobe Acrobat.
This can be viewed and searched on-line, or you can print it out.
If you require Acrobat, please download this from:

   http://www.adobe.com/


Rebuilding ARMulator
====================

The ARMulator can be rebuilt with a custom memory model to accurately
simulate a target memory system.  The memory model can simulate I/O devices
by trapping accesses to I/O locations and simulating their operation.
Interrupts and other exceptions can be scheduled to allow full software
simulation of a target system.


a) Rebuilding ARMulator on Win32
   -----------------------------
   The ARMulator is supplied as a DLL (armulate.dll) which is loaded
   dynamically by windbg and armsd.  The directory \arm202u\source contains
   the source and objects to enable ARMulator to be rebuilt.

   Note: MS Visual C++ v2.0 or v4.x is required to rebuild armulate.dll

      cd \arm202u\SOURCE\WIN32\ARMUL\MSVC20\DLL

      \msdev\bin\vcvars32 x86

      nmake -f armulate.mak

   This new armulate.dll should be copied into the bin directory (rename the
   original armulate.dll to keep it safe).


b) Rebuilding ARMulator on SunOS
   -----------------------------
   The ARMulator is statically linked into the command-line debugger (armsd)
   executable.  In order to rebuild the ARMulator memory model, the supplied
   makefile should be used to build a new armsd.

   Note: gcc or standard SunOS cc is required to rebuild armsd

      cd arm202u/armsd/build

      make

   This new armsd should be copied into the bin directory (rename the
   original armsd to keep it safe).


Feedback
========

We regret that we cannot enter into any correspondance relating to this
unsupported toolkit.

For more details about the ARM University Program, please see the ARM
University Program home page:

   http://www.arm.com/DevSupp/UniProg/

Feedback can be sent to ARM using the feedback form available on our
web site:

   http://www.arm.com/DevSupp/UniProg/FeedbackForm.html

If you require assistance with ARM programming, try posting to the internet
newsgroup 'comp.sys.arm'

For more information on ARM or for details of how to purchase a supported
toolkit, please see ARM's web site:

   http://www.arm.com/
