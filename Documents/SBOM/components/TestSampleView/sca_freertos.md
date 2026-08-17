# FreeRTOS-Kernel Component Declaration

## Component Information

- Name: FreeRTOS-Kernel
- SBOM Version: 10.1.1
- Component Type: Library
- Scope: TestSampleView
- Integration: Vendored source
- Modification Status: Vendored
- License: MIT
- Author: Amazon.com, Inc. (FreeRTOS project)
- Supplier: Amazon Web Services

## Source Location

- `BSP/ThirdParty/FreeRTOS/Source`
- `BSP/ThirdParty/FreeRTOS/Demo`
- `BSP/ThirdParty/FreeRTOS/License`

## Version Evidence

The following source files explicitly identify the component as
FreeRTOS Kernel V10.1.1:

- `BSP/ThirdParty/FreeRTOS/Source/include/FreeRTOS.h`
- `BSP/ThirdParty/FreeRTOS/Source/tasks.c`

The SBOM version is therefore recorded as:

- `10.1.1`

## License Evidence

The FreeRTOS kernel license file states that the kernel is released
under the MIT open source license.

License evidence:

- `BSP/ThirdParty/FreeRTOS/License/license.txt`

The kernel source files also contain the standard MIT permission and
warranty disclaimer text.

The applicable SPDX license identifier is:

- `MIT`

## Usage

The FreeRTOS kernel is included for N9H20 RTOS sample, demonstration,
and integration projects.

Relevant sample location:

- `BSP/SampleCode/FreeRTOS`

The component is classified under TestSampleView because the kernel is
used by sample and demonstration projects rather than the default
Non-OS product runtime.

## SBOM Mapping

- SCA Definition: `Documents/SBOM/components/TestSampleView/sca_freertos.json`
- Package URL: `pkg:generic/freertos-kernel@10.1.1`
- Component Origin: Third-party
- Required Scope: TestSampleView

## Review Result

- Classification: Third-Party OSS
- Status: Approved