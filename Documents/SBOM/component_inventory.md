# N9H20 BSP Component Inventory

## Baseline Information

- BSP: N9H20 Non-OS BSP
- Release Tag: V1.03.000
- Git Commit: 49b4adaba4c2e1e5cf556ab055217be5c4049342
- SBOM Format: CycloneDX JSON

## ProductView Components

| Component                   | Version | License          | Integration              | SCA Definition                                |
|-----------------------------|---------|------------------|--------------------------|-----------------------------------------------|
| zlib                        | 0.95P   | Zlib             | Modified vendored source | `components/ProductView/sca_zlib.json`        |
| bzip2                       | 1.0.2   | bzip2-1.0.6      | Modified vendored source | `components/ProductView/sca_bzip2.json`       |
| U-Boot Legacy Image Support | Unknown | GPL-2.0-or-later | Modified vendored source | `components/ProductView/sca_uboot_image.json` |

## TestSampleView Components

| Component       | Version | License | Integration     | SCA Definition                                |
|-----------------|---------|---------|-----------------|-----------------------------------------------|
| FreeRTOS-Kernel | 10.1.1  | MIT     | Vendored source | `components/TestSampleView/sca_freertos.json` |

## Evidence Mapping

| Component                   | Evidence Document                  |
|-----------------------------|------------------------------------|
| zlib                        | `evidence/evidence_zlib.md`        |
| bzip2                       | `evidence/evidence_bzip2.md`       |
| U-Boot Legacy Image Support | `evidence/evidence_uboot_image.md` |
| FreeRTOS-Kernel             | `evidence/evidence_freertos.md`    |

## Excluded First-Party Components

| Component   | Classification | Reason                                  |
|-------------|----------------|-----------------------------------------|
| JPEG Driver | First-party    | Nuvoton JPEG hardware codec driver      |
| AutoWriter  | First-party    | Nuvoton proprietary PC programming tool |
| TurboWriter | First-party    | Nuvoton proprietary PC programming tool |
| NRomMaker   | First-party    | Nuvoton proprietary ROM generation tool |
| LCMTestTool | First-party    | Nuvoton proprietary display test tool   |

## Component Summary

- ProductView third-party components: 3
- TestSampleView third-party components: 1
- Total third-party components: 4
- Reviewed first-party exclusions: 5

## Review Status

- Component identification: Complete
- Version evidence: Complete, except U-Boot upstream release version
- License evidence: Complete
- SHA-256 evidence: Complete
- Status: Approved