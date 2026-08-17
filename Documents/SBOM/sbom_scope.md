# N9H20 BSP SBOM Scope Definition

## BSP Information

- BSP Name: N9H20 Non-OS BSP
- Release Tag: V1.03.000
- Git Commit: 49b4adaba4c2e1e5cf556ab055217be5c4049342
- Architecture: ARM9
- SBOM Format: CycloneDX JSON

## ProductView

The Product SBOM includes software components that are distributed as
runtime libraries, device drivers, build scripts, loaders, or components
that may be integrated into N9H20 product firmware.

Included paths:

- BSP/Driver
- BSP/Library
- BSP/Library_GCC
- BSP/Script
- Loaders

Known third-party components:

- zlib
- bzip2

## TestSampleView

The Test Sample SBOM includes examples, demonstrations, validation
projects, third-party sample middleware, manufacturing tools, and
host-side utilities that are not included in product runtime firmware
unless explicitly integrated by the user.

Included paths:

- BSP/SampleCode
- BSP/ThirdParty
- Tools

Known third-party components:

- FreeRTOS-Kernel 10.1.1

## Scope Exclusions

Generated output, SBOM governance files, and content outside the
corresponding view are excluded from each scan view.

## Third-Party Component Management

Third-party components that cannot be reliably identified by Syft are
declared through manual SCA component JSON files under:

- Documents/SBOM/components/ProductView
- Documents/SBOM/components/TestSampleView
