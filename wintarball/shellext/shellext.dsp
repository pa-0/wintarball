# Microsoft Developer Studio Project File - Name="shellext" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=shellext - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "shellext.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "shellext.mak" CFG="shellext - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "shellext - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "shellext - Win32 Release MinSize" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "shellext - Win32 Release MinDependency" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "shellext - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /I "zlib-1.1.3" /I "bzip2-1.0.1" /D "_DEBUG" /D "_MBCS" /D "WIN32" /D "_WINDOWS" /D "_USRDLL" /D "STRICT" /D "WIN32_LEAN_AND_MEAN" /D "VC_EXTRALEAN" /FD /GZ /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept

!ELSEIF  "$(CFG)" == "shellext - Win32 Release MinSize"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "ReleaseMinSize"
# PROP BASE Intermediate_Dir "ReleaseMinSize"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ReleaseMinSize"
# PROP Intermediate_Dir "ReleaseMinSize"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /O1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "_ATL_DLL" /D "_ATL_MIN_CRT" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /W3 /GX /I "zlib-1.1.3" /I "bzip2-1.0.1" /D "NDEBUG" /D "_ATL_DLL" /D "_ATL_MIN_CRT" /D "_MBCS" /D "WIN32" /D "_WINDOWS" /D "_USRDLL" /D "STRICT" /D "WIN32_LEAN_AND_MEAN" /D "VC_EXTRALEAN" /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386

!ELSEIF  "$(CFG)" == "shellext - Win32 Release MinDependency"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "ReleaseMinDependency"
# PROP BASE Intermediate_Dir "ReleaseMinDependency"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ReleaseMinDependency"
# PROP Intermediate_Dir "ReleaseMinDependency"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /O1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "_ATL_STATIC_REGISTRY" /D "_ATL_MIN_CRT" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /W3 /GX /I "zlib-1.1.3" /I "bzip2-1.0.1" /D "NDEBUG" /D "_ATL_STATIC_REGISTRY" /D "_ATL_MIN_CRT" /D "_MBCS" /D "WIN32" /D "_WINDOWS" /D "_USRDLL" /D "STRICT" /D "WIN32_LEAN_AND_MEAN" /D "VC_EXTRALEAN" /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386

!ENDIF 

# Begin Target

# Name "shellext - Win32 Debug"
# Name "shellext - Win32 Release MinSize"
# Name "shellext - Win32 Release MinDependency"
# Begin Group "Source"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\common\Configuration.cpp
# End Source File
# Begin Source File

SOURCE=..\common\Configuration.hpp
# End Source File
# Begin Source File

SOURCE=.\Progress.hpp
# End Source File
# Begin Source File

SOURCE=.\shellext.cpp
# End Source File
# Begin Source File

SOURCE=.\shellext.def
# End Source File
# Begin Source File

SOURCE=.\ShellExtension.cpp
# End Source File
# Begin Source File

SOURCE=.\ShellExtension.hpp
# End Source File
# End Group
# Begin Group "Resources"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\menu_icon.bmp
# End Source File
# Begin Source File

SOURCE=.\shell_extension.rgs
# End Source File
# Begin Source File

SOURCE=.\shellext.rc
# End Source File
# Begin Source File

SOURCE=.\tarball.ico
# End Source File
# End Group
# Begin Group "bzip2"

# PROP Default_Filter ""
# Begin Source File

SOURCE=".\bzip2-1.0.1\blocksort.c"
# End Source File
# Begin Source File

SOURCE=".\bzip2-1.0.1\bz_compress.c"
# End Source File
# Begin Source File

SOURCE=".\bzip2-1.0.1\bzlib.c"
# End Source File
# Begin Source File

SOURCE=".\bzip2-1.0.1\bzlib.h"
# End Source File
# Begin Source File

SOURCE=".\bzip2-1.0.1\bzlib_private.h"
# End Source File
# Begin Source File

SOURCE=".\bzip2-1.0.1\crctable.c"
# End Source File
# Begin Source File

SOURCE=".\bzip2-1.0.1\decompress.c"
# End Source File
# Begin Source File

SOURCE=".\bzip2-1.0.1\huffman.c"
# End Source File
# Begin Source File

SOURCE=".\bzip2-1.0.1\randtable.c"
# End Source File
# End Group
# Begin Group "zlib"

# PROP Default_Filter ""
# Begin Source File

SOURCE=".\zlib-1.1.3\adler32.c"
# End Source File
# Begin Source File

SOURCE=".\zlib-1.1.3\compress.c"
# End Source File
# Begin Source File

SOURCE=".\zlib-1.1.3\crc32.c"
# End Source File
# Begin Source File

SOURCE=".\zlib-1.1.3\deflate.c"
# End Source File
# Begin Source File

SOURCE=".\zlib-1.1.3\deflate.h"
# End Source File
# Begin Source File

SOURCE=".\zlib-1.1.3\gzio.c"
# End Source File
# Begin Source File

SOURCE=".\zlib-1.1.3\infblock.c"
# End Source File
# Begin Source File

SOURCE=".\zlib-1.1.3\infblock.h"
# End Source File
# Begin Source File

SOURCE=".\zlib-1.1.3\infcodes.c"
# End Source File
# Begin Source File

SOURCE=".\zlib-1.1.3\infcodes.h"
# End Source File
# Begin Source File

SOURCE=".\zlib-1.1.3\inffast.c"
# End Source File
# Begin Source File

SOURCE=".\zlib-1.1.3\inffast.h"
# End Source File
# Begin Source File

SOURCE=".\zlib-1.1.3\inffixed.h"
# End Source File
# Begin Source File

SOURCE=".\zlib-1.1.3\inflate.c"
# End Source File
# Begin Source File

SOURCE=".\zlib-1.1.3\inftrees.c"
# End Source File
# Begin Source File

SOURCE=".\zlib-1.1.3\inftrees.h"
# End Source File
# Begin Source File

SOURCE=".\zlib-1.1.3\infutil.c"
# End Source File
# Begin Source File

SOURCE=".\zlib-1.1.3\infutil.h"
# End Source File
# Begin Source File

SOURCE=".\zlib-1.1.3\trees.c"
# End Source File
# Begin Source File

SOURCE=".\zlib-1.1.3\trees.h"
# End Source File
# Begin Source File

SOURCE=".\zlib-1.1.3\uncompr.c"
# End Source File
# Begin Source File

SOURCE=".\zlib-1.1.3\zconf.h"
# End Source File
# Begin Source File

SOURCE=".\zlib-1.1.3\zlib.h"
# End Source File
# Begin Source File

SOURCE=".\zlib-1.1.3\zutil.c"
# End Source File
# Begin Source File

SOURCE=".\zlib-1.1.3\zutil.h"
# End Source File
# End Group
# End Target
# End Project
