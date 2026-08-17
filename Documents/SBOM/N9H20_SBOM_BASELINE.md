# N9H20 BSP SBOM Baseline

## Baseline Information

- BSP: N9H20 Non-OS BSP
- BSP Release Tag: V1.03.000
- BSP Git Commit: 49b4adaba4c2e1e5cf556ab055217be5c4049342
- SBOM Baseline Version: 1.0
- SBOM Format: CycloneDX JSON
- Baseline Status: Ready for SBOM generation

## Scope Definitions

- ProductView Scope: `product-scope.yaml`
- TestSampleView Scope: `test-sample-scope.yaml`
- Scope Description: `sbom_scope.md`

## Included Third-Party Components

### ProductView

| Component                   | Version | License          |
|-----------------------------|---------|------------------|
| zlib                        | 0.95P   | Zlib             |
| bzip2                       | 1.0.2   | bzip2-1.0.6      |
| U-Boot Legacy Image Support | Unknown | GPL-2.0-or-later |

### TestSampleView

| Component       | Version | License |
|-----------------|---------|---------|
| FreeRTOS-Kernel | 10.1.1  | MIT     |

## Component Definitions

### ProductView

- `components/ProductView/sca_zlib.json`
- `components/ProductView/sca_bzip2.json`
- `components/ProductView/sca_uboot_image.json`

### TestSampleView

- `components/TestSampleView/sca_freertos.json`

## Evidence Documents

- `evidence/evidence_zlib.md`
- `evidence/evidence_bzip2.md`
- `evidence/evidence_uboot_image.md`
- `evidence/evidence_freertos.md`

## Exclusion Review

The following reviewed items are classified as Nuvoton first-party
components:

- JPEG Driver
- AutoWriter
- TurboWriter
- NRomMaker
- LCMTestTool

The detailed exclusion record is maintained in:

- `exclusions.yaml`

## Supporting Governance Documents

- `component_inventory.md`
- `SBOM_Maintenance_Policy.md`
- `review/N9H20_SBOM_Review.md`

## Expected SBOM Deliverables

The SBOM generation process is expected to produce:

- ProductView CycloneDX SBOM
- TestSampleView CycloneDX SBOM
- SBOM Manifest
- ProductView vulnerability-scan result
- TestSampleView vulnerability-scan result
- Vulnerability-scan summary
- SHA-256 checksum list
- Release README

## Validation Requirements

The following criteria must be satisfied before release:

- ProductView SBOM generation succeeds
- TestSampleView SBOM generation succeeds
- All manual SCA components are merged
- CycloneDX validation passes
- Manifest generation succeeds
- Vulnerability scanning completes
- No unresolved Critical vulnerability is accepted without review
- Generated SBOM contains no developer-specific absolute path
- Release checksums are generated and verified

## Baseline Maintenance

The baseline must be reviewed when:

- The BSP release tag or Git commit changes
- A third-party component is added, removed, or upgraded
- Component license information changes
- Scope definitions change
- Evidence files change
- The SBOM generation pipeline changes
- Vulnerability results require component metadata correction

## Current Review Status

- Scope definitions: Complete
- ProductView component declarations: Complete
- TestSampleView component declarations: Complete
- Component evidence: Complete
- Exclusion review: Complete
- Component inventory: Complete
- SBOM generation: Complete
- CycloneDX validation: Complete
- Vulnerability scan: Complete (0 detected)
- Release package: Pending