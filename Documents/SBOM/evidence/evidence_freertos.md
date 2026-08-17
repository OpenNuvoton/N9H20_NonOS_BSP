# FreeRTOS-Kernel Evidence

## Component Identification

- Component: FreeRTOS-Kernel
- SBOM Version: 10.1.1
- Scope: TestSampleView
- Classification: Third-Party OSS
- Integration: Vendored source
- Modification Status: Vendored
- License: MIT
- Supplier: Amazon Web Services

## Evidence Location

- `BSP/ThirdParty/FreeRTOS/Source/include/FreeRTOS.h`
- `BSP/ThirdParty/FreeRTOS/Source/tasks.c`
- `BSP/ThirdParty/FreeRTOS/License/license.txt`

## Version Evidence

Both `FreeRTOS.h` and `tasks.c` explicitly identify the component as:

- `FreeRTOS Kernel V10.1.1`

The SBOM version is therefore recorded as `10.1.1`.

## License Evidence

The license file explicitly states that the FreeRTOS kernel is released
under the MIT open source license.

The license file contains the standard MIT permission statement:

- Permission is granted to use, copy, modify, merge, publish,
  distribute, sublicense, and sell copies of the software.

License evidence:

- `BSP/ThirdParty/FreeRTOS/License/license.txt`

## Usage Evidence

The FreeRTOS kernel is included for N9H20 RTOS sample and demonstration
projects.

Related BSP sample location:

- `BSP/SampleCode/FreeRTOS`

The component is assigned to TestSampleView.

## SHA-256 Evidence

- `FreeRTOS.h`: `B60E763E4D23758A8330BA283C096F62871F91C48AEE8D47106C10CA67E2C816`
- `license.txt`: `2817E4A1F20D66DA691361156DA03DDFD7A98B502104D9C09F257FCA230D3DE9`
- `tasks.c`: `086C3691A3D6D715B4EC0998C73A26EBCC3E8B1A04C961B0EAE7A2FB105075D9`

## SBOM Mapping

- SCA Definition: `Documents/SBOM/components/TestSampleView/sca_freertos.json`
- Component Declaration: `Documents/SBOM/components/TestSampleView/sca_freertos.md`
- Package URL: `pkg:generic/freertos-kernel@10.1.1`

## Review Result

- Status: Approved