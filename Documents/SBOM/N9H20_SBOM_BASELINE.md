# N9H20 BSP SBOM Baseline

## Baseline Information
- BSP: N9H20 Non-OS BSP
- SBOM Baseline Version: 1.1
- SBOM Format: CycloneDX JSON 1.6
- Baseline Status: Under review

## Baseline Rule
This document defines the durable SBOM input baseline. Release-specific Git commit IDs, generated SBOM serial numbers, scanner database information, and output hashes are recorded in the generated manifest and SVN release package rather than hard-coded here.

## Scope Definitions
- ProductView Scope: product-scope.yaml
- TestSampleView Scope: test-sample-scope.yaml
- Scope Description: sbom_scope.md

## Included Third-Party Components

## ProductView
- zlib 0.95P, license `Zlib`
- bzip2 1.0.2, license `LicenseRef-bzip2-1.0.2`
- U-Boot Legacy Image Support, version `Unknown`, license `GPL-2.0-or-later`

## TestSampleView
- FreeRTOS-Kernel 10.1.1, license `MIT`

## Component Definitions

## ProductView
- components/ProductView/sca_zlib.json
- components/ProductView/sca_bzip2.json
- components/ProductView/sca_uboot_image.json

## TestSampleView
- components/TestSampleView/sca_freertos.json

## Evidence Documents
- evidence/evidence_zlib.md
- evidence/evidence_bzip2.md
- evidence/evidence_uboot_image.md
- evidence/evidence_freertos.md

## bzip2 Security Baseline
- CVE-2019-12900: `patched-by-source-backport`; source remediation verified.
- CVE-2005-0953: `vulnerable-code-not-present`.
- CVE-2005-1260: `not-in-default-build`.
- CVE-2008-1372: `not-in-default-build`.
- CVE-2010-0405: `not-in-default-build`; source remediation unverified.
- CVE-2011-4089: `vulnerable-code-not-present`.
- Inspected default GCC and KEIL build inclusion of bzip2 was not demonstrated.
- The observed default runtime decompression path uses zlib.
- Any future bzlib enablement requires renewed source, build, SAST, and regression review.
- Product Security approval remains required before formal release publication.

## Exclusion Review
The following reviewed items are classified as Nuvoton first-party components or tools:
- JPEG Driver
- AutoWriter
- TurboWriter
- NRomMaker
- LCMTest
- HID Transfer FS
- HID Transfer HS
- HID Firmware Update FS
- HID Firmware Update HS

Detailed exclusion records are maintained in `exclusions.yaml`.

## Supporting Governance Documents
- component_inventory.md
- SBOM_Maintenance_Policy.md
- review/N9H20_SBOM_Review.md

## Expected SVN Release Deliverables
- ProductView CycloneDX SBOM
- TestSampleView CycloneDX SBOM
- SBOM manifest
- ProductView vulnerability-scan result
- TestSampleView vulnerability-scan result
- Vulnerability-scan summary
- `sbom-checker-product.json`
- `sbom-checker-test-sample.json`
- README
- SHA-256 checksum list
- Approved VEX and Product Security evidence when required

## Validation Requirements
Before release:
- Generate both SBOM views from a clean Git source baseline.
- Merge all approved manual SCA declarations.
- Validate all CycloneDX documents.
- Verify component and dependency references.
- Confirm required licenses and evidence.
- Confirm that no developer-specific absolute path appears in final deliverables.
- Run vulnerability scans for both views.
- Complete technical review for accepted findings.
- Obtain Product Security approval when required.
- Verify every release artifact against the final checksum manifest.
- Update `latest-release.txt` only after the release directory is committed and verified in SVN.

## Current Review Status
- Scope definitions: Complete
- ProductView component declarations: Complete
- TestSampleView component declarations: Complete
- Component evidence: Complete, subject to source-hash revalidation after commit
- Exclusion review: Complete
- Component inventory: Update required in the same Git change set
- bzip2 technical review: Complete
- Product Security approval: Pending
- SBOM regeneration from the new Git commit: Pending
- Formal SVN release package: Pending
