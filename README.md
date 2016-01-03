LazyCopy
=============

An NTFS [minifilter driver](https://msdn.microsoft.com/en-us/library/windows/hardware/ff540402%28v=vs.85%29.aspx?f=255&MSPPError=-2147217396) that can replace file contents, when it is accessed for the first time.

For example, with the included C# client you can create an empty file and "map" it to a remote location. When that file is opened, its contents will be downloaded by the driver.

Prerequisites
-------

1. Windows 7+
2. [Visual Studio 2015 & WDK 10](https://msdn.microsoft.com/en-us/windows/hardware/dn913721.aspx)
3. [WiX toolset](https://wix.codeplex.com/releases/view/610859)

Folder structure
-------

- `Driver`
  - `LazyCopyDriver` - minifilter driver that can track file open operations and fetch remote file contents.
  - `LazyCopyDriverInstall` - generates driver installation package.
  - `DriverClientLibrary` - C# library that provides base classes to communicate with the minifilter driver via the [Communication ports](https://msdn.microsoft.com/en-us/library/windows/hardware/ff541931(v=vs.85).aspx).
  - `LazyCopyDriverClient` - LazyCopy C# driver client based on the `DriverClientLibrary`.
- `ToolsAndLibraries`
  - `Utilities` - contains shared helper classes.
  - `EventTracing` - allows collecting and decoding of the [ETW](https://msdn.microsoft.com/en-us/library/windows/desktop/bb968803(v=vs.85).aspx) events generated by the driver.
  - `SampleClient` - a basic console C# application that can create files that are understood by the driver.
- `Service\LazyCopySvc` - a user-mode system service that manages the lifetime and configuration of the driver. It can also open files that cannot opened by the driver on behalf of the currently logged in user.
- `Setup` - [WiX](http://wixtoolset.org/) installation package to install driver, service and client applications.

Installation
-------

* Allow unsigned driver installation:
   1. Open CMD as Admin and type: `bcdedit -set TESTSIGNING ON` ([info](https://msdn.microsoft.com/en-us/library/windows/hardware/ff553484(v=vs.85).aspx)).
   2. Reboot. This will allow Windows to load drivers signed with the test certificates.
* Compile the entire solution in the Visual Studio for your architecture.
* You can manually install the driver by right clicking on the `.inf` file and choosing `Install`.
* Check that LazyCopyDriver appeared in the `fltmc` command output.
<br/>Refer to the [MSDN](https://msdn.microsoft.com/en-us/library/windows/hardware/ff548166(v=vs.85).aspx) for additional information.
<br/>From the Admin CMD:
```
> fltmc
Filter Name                     Num Instances    Altitude    Frame
------------------------------  -------------  ------------  -----
LazyCopyDriver                          7       370021         0
FileInfo                                8        45000         0
```
* Install and start the LazyCopySvc (optional):
```
> sc create LazyCopySvc binPath="<path to LazyCopySvc.exe>" DisplayName="LazyCopySvc"
> sc start LazyCopySvc
```

Usage
-------

Create an empty file that will be fetched on the first access (admin permissions are required):
```
bin\SampleClient\SampleClient.exe <original file> <new empty file>
```

Driver signing
-------

* [Get](https://msdn.microsoft.com/en-us/library/windows/hardware/hh801887.aspx) a code signing certificate.
* Sign the driver and, optionally, other binaries.
   For example, if you purchased a Symantec certificate, you can use the following command to sign the driver in the post-build step:
```
signtool sign /v /ac "C:\Program Files (x86)\Windows Kits\10\CrossCertificates\VRSN_C3_PCA_G5_Root_CA_Cross.cer" /s my /n "<YOUR NAME>" /t http://timestamp.verisign.com/scripts/timestamp.dll /sha1 "<YOUR CERT THUMBNAIL>" "$(TargetPath)\LazyCopyDriver.sys"
&
signtool sign /v /ac "C:\Program Files (x86)\Windows Kits\10\CrossCertificates\VRSN_C3_PCA_G5_Root_CA_Cross.cer" /s my /n "<YOUR NAME>" /t http://timestamp.verisign.com/scripts/timestamp.dll /sha1 "<YOUR CERT THUMBNAIL>" "$(TargetPath)\LazyCopyDriver.cat"
```

Want to reuse the project files?
-------

* Change the [name](Driver/LazyCopyDriver/LazyCopyDriver.inf) of the driver and rename binaries.
* Generate a new GUID for the [ETW provider](Driver/LazyCopyDriver/LazyCopyEtw.mc) and re-create header:
```
mc.exe -z LazyCopyEtw -n -km LazyCopyEtw.mc
```
* Contact Microsoft and get the following values:
  - [LC_REPARSE_GUID](Driver/LazyCopyDriver/LazyCopyDriver.c) - from [here](https://msdn.microsoft.com/en-us/library/windows/hardware/dn641624(v=vs.85).aspx)
  - [Instance1.Altitude](Driver/LazyCopyDriver/LazyCopyDriver.inf) - from [here](https://msdn.microsoft.com/en-us/library/windows/hardware/dn508284(v=vs.85).aspx)
* (Optional) Change the reparse point tag: [LC_REPARSE_TAG](Driver/LazyCopyDriver/Globals.h).
* Update the client code with the new reparse GUID and tag: [LazyCopyFileHelper.cs](Driver/LazyCopyDriverClient/LazyCopyFileHelper.cs).
