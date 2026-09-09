# N9H20 BSP Component Inventory

## Inventory Purpose
This document defines the durable third-party component inventory used to generate the N9H20 ProductView and TestSampleView SBOMs. Release-specific Git commit IDs, generated serial numbers, scan database information, and output hashes are recorded in generated release artifacts rather than maintained in this inventory.

## ProductView Components

| Component                   | Version | License                | Integration              | SCA Definition                              | Review Status                                                |
|-----------------------------|---------|------------------------|--------------------------|---------------------------------------------|--------------------------------------------------------------|
| zlib                        | 0.95P   | Zlib                   | Modified vendored source | components/ProductView/sca_zlib.json        | Approved                                                     |
| bzip2                       | 1.0.2   | LicenseRef-bzip2-1.0.2 | Modified vendored source | components/ProductView/sca_bzip2.json       | Technical review complete; known issues accepted/deferred for release follow-up |
| U-Boot Legacy Image Support | Unknown | GPL-2.0-or-later       | Modified vendored source | components/ProductView/sca_uboot_image.json | Approved with documented unknown upstream version            |

## TestSampleView Components

| Component       | Version | License | Integration     | SCA Definition                              | Review Status |
|-----------------|---------|---------|-----------------|---------------------------------------------|---------------|
| FreeRTOS-Kernel | 10.1.1  | MIT     | Vendored source | components/TestSampleView/sca_freertos.json | Approved      |
| Linux USB HIDBP Keyboard Support | Unknown | GPL-2.0-or-later | Modified vendored source | components/TestSampleView/sca_usb_hidbp_keyboard.json | Identified from source evidence |
| USB Video Class Header Support | Unknown | GPL-2.0-or-later | Modified vendored source | components/TestSampleView/sca_usb_video_header.json | Supplier and upstream version require manual confirmation |

## Evidence Mapping

| Component                   | Evidence Document                |
|-----------------------------|----------------------------------|
| zlib                        | evidence/evidence_zlib.md        |
| bzip2                       | evidence/evidence_bzip2.md       |
| U-Boot Legacy Image Support | evidence/evidence_uboot_image.md |
| FreeRTOS-Kernel             | evidence/evidence_freertos.md    |
| Linux USB HIDBP Keyboard Support | evidence/evidence_usb_host_legacy.md |
| USB Video Class Header Support | evidence/evidence_usb_host_legacy.md |

## ProductView Build-Scope Notes
- The ProductView inventory includes distributed vendored source even when inclusion in an inspected default build is not demonstrated.
- The bzip2 source remains in the Product SBOM because the source is distributed in `Loaders/SpiLoader_gzip/ziplib`.
- Inclusion of the bzip2 runtime in the inspected default GCC and KEIL project configurations was not demonstrated.
- The observed default runtime decompression path uses zlib.
- Source-level compatibility with bzip2 in U-Boot-derived boot support does not by itself demonstrate default-build linkage.

## bzip2 Vulnerability Dispositions

| Vulnerability  | Technical Disposition       | Source Remediation | Release Disposition |
|----------------|-----------------------------|--------------------|---------------------|
| CVE-2019-12900 | patched-by-source-backport  | verified           | accepted/deferred follow-up |
| CVE-2005-0953  | vulnerable-code-not-present | not applicable     | accepted/deferred follow-up |
| CVE-2005-1260 | not-in-default-build | not assessed | accepted/deferred follow-up |
| CVE-2008-1372 | not-in-default-build | not assessed | accepted/deferred follow-up |
| CVE-2010-0405 | not-in-default-build | unverified | accepted/deferred follow-up |
| CVE-2011-4089  | vulnerable-code-not-present | not applicable     | accepted/deferred follow-up |

## Reviewed First-Party Exclusions

| Component or Tool      | Classification | Reason                                                                |
|------------------------|----------------|-----------------------------------------------------------------------|
| JPEG Driver            | First-party    | Nuvoton JPEG hardware codec driver                                    |
| AutoWriter             | First-party    | Nuvoton proprietary PC programming tool                               |
| TurboWriter            | First-party    | Nuvoton proprietary PC programming tool                               |
| NRomMaker              | First-party    | Nuvoton proprietary ROM generation tool                               |
| LCMTest                | First-party    | Nuvoton proprietary display test tool                                 |
| HID Transfer FS        | First-party    | Nuvoton proprietary HID transfer tool for full-speed operation        |
| HID Transfer HS        | First-party    | Nuvoton proprietary HID transfer tool for high-speed operation        |
| HID Firmware Update FS | First-party    | Nuvoton proprietary HID firmware update tool for full-speed operation |
| HID Firmware Update HS | First-party    | Nuvoton proprietary HID firmware update tool for high-speed operation |

Detailed paths and exclusion reasons are maintained in `exclusions.yaml`.

## Component Summary
- ProductView third-party components: 3
- TestSampleView third-party components: 3
- Total third-party components: 6
- Reviewed first-party exclusions: 9

## Review Status
- Component identification: Complete
- Version evidence: Complete, except the documented unknown U-Boot upstream release version
- License evidence: Complete for the current declarations
- Evidence hashes: Maintained in the evidence and SCA declarations and verified against the selected clean release commit
- bzip2 technical review: Complete
- bzip2 release disposition: Accepted/deferred by the N9H20 BSP release owner
- Product Security approval: Not recorded
- Approved VEX: Not issued
- Release-specific generation and validation status: Recorded in the SVN release package
