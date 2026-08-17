# N9H20 BSP SBOM Review

## Review Information

- BSP: N9H20 Non-OS BSP
- BSP Release Tag: V1.03.000
- BSP Git Commit: 49b4adaba4c2e1e5cf556ab055217be5c4049342
- SBOM Baseline Version: 1.0
- SBOM Format: CycloneDX JSON
- Review Status: Generation and vulnerability-scan review complete

## Scope Review

### ProductView

Included paths:

- `BSP/Driver`
- `BSP/Library`
- `BSP/Library_GCC`
- `BSP/Script`
- `Loaders`

Scope definition:

- `Documents/SBOM/product-scope.yaml`

### TestSampleView

Included paths:

- `BSP/SampleCode`
- `BSP/ThirdParty`
- `Tools`

Scope definition:

- `Documents/SBOM/test-sample-scope.yaml`

## Included Third-Party Components

### ProductView

| Component                   | Version | License          | Review Result                          |
|-----------------------------|---------|------------------|----------------------------------------|
| zlib                        | 0.95P   | Zlib             | Approved                               |
| bzip2                       | 1.0.2   | bzip2-1.0.6      | Approved                               |
| U-Boot Legacy Image Support | Unknown | GPL-2.0-or-later | Approved with unknown upstream version |

### TestSampleView

| Component       | Version | License | Review Result |
|-----------------|---------|---------|---------------|
| FreeRTOS-Kernel | 10.1.1  | MIT     | Approved      |

## Version Review

### zlib

- Runtime/library version: `0.95P`
- Upstream distribution: `zlib-0.95`
- Modification status: Modified
- Version evidence: `Loaders/SpiLoader_gzip/ziplib/zlib.h`

### bzip2

- Component version: `1.0.2`
- Upstream distribution: `bzip2-1.0.2`
- Modification status: Modified
- Version evidence:
  - `Loaders/SpiLoader_gzip/ziplib/bzlib.h`
  - `Loaders/SpiLoader_gzip/ziplib/bzlib.c`

### U-Boot Legacy Image Support

- Component version: `Unknown`
- Modification status: Modified
- Unknown-version justification: No explicit upstream U-Boot release
  version was identified in the vendored source files.
- Evidence:
  - `Loaders/SpiLoader_gzip/cmd_bootm.c`
  - `Loaders/SpiLoader_gzip/ziplib/image.h`

### FreeRTOS-Kernel

- Component version: `10.1.1`
- Modification status: Vendored
- Version evidence:
  - `BSP/ThirdParty/FreeRTOS/Source/include/FreeRTOS.h`
  - `BSP/ThirdParty/FreeRTOS/Source/tasks.c`

## License Review

| Component                   | SPDX License     | License Evidence                                                              |
|-----------------------------|------------------|-------------------------------------------------------------------------------|
| zlib                        | Zlib             | `Loaders/SpiLoader_gzip/ziplib/zlib.h`                                        |
| bzip2                       | bzip2-1.0.6      | `Loaders/SpiLoader_gzip/ziplib/bzlib.h`                                       |
| U-Boot Legacy Image Support | GPL-2.0-or-later | `Loaders/SpiLoader_gzip/cmd_bootm.c`; `Loaders/SpiLoader_gzip/ziplib/image.h` |
| FreeRTOS-Kernel             | MIT              | `BSP/ThirdParty/FreeRTOS/License/license.txt`                                 |

## Evidence Review

The following evidence documents were reviewed:

- `Documents/SBOM/evidence/evidence_zlib.md`
- `Documents/SBOM/evidence/evidence_bzip2.md`
- `Documents/SBOM/evidence/evidence_uboot_image.md`
- `Documents/SBOM/evidence/evidence_freertos.md`

Evidence status:

- Component identification evidence: Complete
- Version evidence: Complete, except identified U-Boot release version
- License evidence: Complete
- SHA-256 evidence: Complete
- Evidence review status: Approved

## Exclusion Review

The following items were reviewed and classified as Nuvoton
first-party components:

| Component   | Classification | Review Result                       |
|-------------|----------------|-------------------------------------|
| JPEG Driver | First-party    | Excluded from third-party inventory |
| AutoWriter  | First-party    | Excluded from third-party inventory |
| TurboWriter | First-party    | Excluded from third-party inventory |
| NRomMaker   | First-party    | Excluded from third-party inventory |
| LCMTestTool | First-party    | Excluded from third-party inventory |

Detailed exclusion information is maintained in:

- `Documents/SBOM/exclusions.yaml`

## Pre-Generation Review Result

- Scope definitions: PASS
- ProductView SCA declarations: PASS
- TestSampleView SCA declarations: PASS
- JSON syntax validation: PASS
- Component declarations: PASS
- Evidence documents: PASS
- Evidence SHA-256 values: PASS
- Exclusion review: PASS
- Component inventory: PASS
- Baseline document: PASS
- Maintenance policy: PASS

## Pending Execution Results

The following results must be updated after running the SBOM generation
pipeline:

- ProductView SBOM generation: PASS
- TestSampleView SBOM generation: PASS
- Manual SCA merge verification: PASS
- Binary metadata enrichment: PASS (no database matches)
- License enrichment: PASS (no database matches)
- CycloneDX validation: PASS
- Manifest generation: PASS
- ProductView vulnerability scan: PASS (0 detected)
- TestSampleView vulnerability scan: PASS (0 detected)
- Release checksum validation: PENDING
- Release package publication: PENDING

## Review Conclusion

The N9H20 manual SCA definitions, evidence, scope definitions, component
inventory, exclusions, and governance documents are ready for SBOM
generation.

Final release approval remains pending until generation, validation,
manifest, vulnerability scan, checksum, and release-package checks are
completed.