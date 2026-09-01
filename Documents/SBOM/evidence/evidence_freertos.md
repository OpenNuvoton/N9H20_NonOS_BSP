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

- `FreeRTOS.h`: `E0FB5EE2E925D2F4978EAF6EF45FD8A866BD540A6514EAD83B806D98CE8E7844`
- `license.txt`: `8F2533FC5028B960D793DDB4525ECEB8CA9DC0FA917A764C64F59744D4E9217A`
- `tasks.c`: `EC0C0C1D79EF14A3D024ED06C512BE8C201855856CA23C6B84809C9933EF818A`

## Canonical Source Closure Hash

- Algorithm: SHA-256
- Value: `719b5aa3603aa829187466b3c0159ca298a7727933caf079fc7549607cc16c92`
- Hash Method: Sorted UTF-8 normalized repository-relative paths followed by exact file bytes

## SBOM Mapping

- SCA Definition: `Documents/SBOM/components/TestSampleView/sca_freertos.json`
- Component Declaration: `Documents/SBOM/components/TestSampleView/sca_freertos.md`
- Package URL: `pkg:generic/freertos-kernel@10.1.1`

## Review Result

- Status: Approved