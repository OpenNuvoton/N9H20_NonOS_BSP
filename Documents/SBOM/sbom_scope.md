# N9H20 BSP SBOM Scope Definition

## BSP Information
- BSP Name: N9H20 Non-OS BSP
- Architecture: ARM9
- SBOM Format: CycloneDX JSON 1.6
- Release identity: Derived from the clean Git source baseline at generation time

## Scope Principle
The Git repository stores durable scope definitions, manual SCA declarations, component evidence, exclusions, and governance records. Generated ProductView and TestSampleView SBOMs, manifests, vulnerability-scan results, VEX documents, validation evidence, and release checksums are stored in the versioned SVN release package.

## ProductView
The Product SBOM includes software distributed as runtime libraries, device drivers, build scripts, loaders, and vendored components that may be integrated into N9H20 product firmware.

Included paths:
- BSP/Driver
- BSP/Library
- BSP/Library_GCC
- BSP/Script
- Loaders

Excluded paths:
- BSP/SampleCode
- BSP/ThirdParty
- Tools
- Documents

Known third-party components:
- zlib 0.95P
- bzip2 1.0.2
- U-Boot Legacy Image Support, version Unknown

The bzip2 source is included because the vendored source is distributed in ProductView. Inclusion in the inspected default GCC and KEIL builds was not demonstrated, and the observed default runtime decompression path uses zlib.

## TestSampleView
The Test Sample SBOM includes examples, demonstrations, validation projects, third-party sample middleware, manufacturing tools, and host-side utilities that are not included in the default product runtime unless explicitly integrated by the user.

Included paths:
- BSP/SampleCode
- BSP/ThirdParty
- Tools

Excluded paths:
- BSP/Driver
- BSP/Library
- BSP/Library_GCC
- BSP/Script
- Loaders
- Documents

Known third-party components:
- FreeRTOS-Kernel 10.1.1

## Scope Exclusions
Generated output, SBOM governance files, and content outside the corresponding view are excluded from each scan view. First-party classifications are recorded in `exclusions.yaml`. An exclusion must not suppress an unresolved third-party component, license issue, or vulnerability finding.

## Manual SCA Component Management
Third-party components that cannot be reliably identified by automated discovery are declared through manual SCA component JSON files under:
- Documents/SBOM/components/ProductView
- Documents/SBOM/components/TestSampleView

Each JSON declaration must have a corresponding Markdown declaration and evidence document. Stable BOM references, package URLs, version evidence, license evidence, source locations, modification status, and component origin must be maintained consistently.

## Release Traceability
Every generated release package must record:
- Clean source Git commit and Git-derived version
- ProductView and TestSampleView SBOM serial numbers
- Generator and validator versions
- Product and Test Sample SBOM hashes
- Vulnerability scanner and database metadata
- Technical dispositions and Product Security approval state when required
- Release checksum manifest

`latest-release.txt` is updated only after the versioned SVN release directory has been committed and verified.
