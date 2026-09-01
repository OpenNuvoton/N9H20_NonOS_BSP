# Legacy USB Host Source Evidence

## Linux USB HIDBP Keyboard Support

- Scope: TestSampleView
- Source: `BSP/SampleCode/USBH/usb_keyboard.c`
- Author and supplier evidence: Vojtech Pavlik
- Version: Unknown
- License: GPL-2.0-or-later
- Source SHA-256: `2e4d3a966f19c874638bd1676c0ad5ce312d3d712625a3f73e123ef174db28b4`
- Closure SHA-256: `baf3c34fa9299f616633efcdfb4b003f53ad043feb3e1277bdaba5f61b739fc9`

The source identifies USB HID boot-protocol keyboard support, copyright
1999-2000 Vojtech Pavlik, and GPL version 2 or later terms. Its CVS file
revision is not treated as an upstream component version. The source is
included by both GCC and KEIL USB host sample projects.

`BSP/SampleCode/USBH/usbkbd.h` is Nuvoton-owned Apache-2.0 integration code
and is not included in the third-party component closure.

## USB Video Class Header Support

- Scope: TestSampleView
- Source: `BSP/SampleCode/USBH/usbvideo.h`
- Supplier: Not determined from the distributed source
- Version: Unknown
- License: GPL-2.0-or-later
- Source SHA-256: `082a69fd2d18a56191da9570ab2127506bf47bf91d682594966a528c8323a9ec`
- Closure SHA-256: `c285684c598985751ee40aa40046ebad7994e5c2e5f02e530b8525e70b4b4d1a`

The header contains GPL version 2 or later terms and is used by the Nuvoton
USB host sample implementation in `w99683.c`. The distributed header does not
provide enough evidence to assert an upstream supplier or release version.

## Hash Method

Each closure hash is SHA-256 over sorted UTF-8 normalized repository-relative
paths followed by the exact bytes of each covered file.
