Welcome to Open Source Surrogate Safety Assessment Model (SSAM) 3.0 project. We refer that as SSAM3 in this document 

SSAM3 is used to perform safety analysis of traffic facilities combining with microsimulation software. This technique may be of use to researchers, transportation engineers, and safety engineers.
 There are two versions of SSAM3, single threading version (SSAM3.exe) and multi-threading version (SSAM3M.exe).

Section 1. Folder Description

======================================================

Documentation: this folder contains documents of SSAM 3.0 project.
	
SSAM3: this folder contains the Visual Studio 2012 solution files for SSAM3, the GUI version of SSAM 3.0. The project in this folder can be used to modify GUI and add roadway 3D models. Please download SSAM3.zip to obtain all files in this folder.
	SSAMAPP: this folder contains source code of SSAM3.
	WorkingDir: this folder is the output path for release mode of SSAM3 executable file.
	WorkingDirDebug: this folder is the output path for debug mode of SSAM3 executable file.

SSAM3EXE: this folder contains two versions of SSAM3 executable files and dependency files. Please download SSAM3EXE.zip to obtain all files in this folder.

SSAM3Setup: this folder contains the installation package for SSAM3.

SSAMDLL: this folder contains the Visual Studio 2012 solution files for SSAMDLL and a console version of SSAM 3.0. The project in this folder can be used to integrate SSAM analysis engine into other applications. Please download SSAMDLL.zip to obtain all files in this folder.
	bin: this folder is the output path for SSAMDLL DLL and SSAM executable files.
	include: this folder contains the header files of SSAMDLL.
	lib: this folder is the output path for SSAMDLL lib file.
	src: this folder contains the definition files of SSAMDLL.
	SSAM: this folder contains the source code of the console version of SSAM.

SSAMEXE: this folder contains a copy of SSAM executable and DLL file. This working command line version of SSAM can be used to run batch file. Please download SSAMEXE.zip to obtain all files in this folder.


Section 2. Compilation Requirements

======================================================

SSAMDLL and SSAM should be compiled in Visual Studio 2012 in Windows. They can be configured to compile in 32 or 64 bit mode.

SSAM3 compilation requires the installation of OpenSceneGraph. SSAM3 should be compiled in Visual Studio 2012 in Windows, and it can be configured to compile in 32 or 64 bit mode.

For details on how to download and install OpenSceneGraph, please see the documentation on the OpenSceneGraph website: http://www.openscenegraph.org/index.php/documentation.    


Section 3. SSAM3 Installation Instructions
======================================================
Users have two options for utilizing SSAM: via an executable and the necessary libraries or through an installation package.

Should the user not wish to install SSAM to their hard drive, they can run the SSAM analysis using the executable file and associated libraries. 
1.            Ensure that Visual C++ Redistributable for Visual Studio 2012 (x64) is installed as the prerequisite.
2.	Download the SSAM3 executable file and its dependency files. They are found in the SSAM3EXE folder. 
3.            Unzip the package and then double click SSAM.exe or SSAM3M.exe (multi-threading).
4.            The GUI for SSAM should open and you're ready to perform your analyses.

If the user chooses to install SSAM3 to their hard drive:
1.            Download the SSAM3 installation package from SSAM3Setup.
2.            Unzip the package, and then double click “setup.exe” to start installation. The installation process will first check dependencies for SSAM3, so choose to install if any dependency is missing.
3.            Follow the wizard to finish the installation.
4.            Find SSAM3 and SSAM3M in Start Menu >> All Programs >> NGSIM.
5.            The GUI for SSAM should open and you're ready to perform your analyses.

