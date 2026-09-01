# N9H20 BSP SBOM Review

## Review Information
- BSP: N9H20 Non-OS BSP
- SBOM Baseline Version: 1.1
- SBOM Format: CycloneDX JSON 1.6
- Source Baseline: The clean Git commit and Git-derived version recorded in each generated release manifest
- Review Status: Technical review complete; Product Security approval remains pending for bzip2 dispositions

## Scope Review

## ProductView
Included paths:
- BSP/Driver
- BSP/Library
- BSP/Library_GCC
- BSP/Script
- Loaders

Scope definition:
- Documents/SBOM/product-scope.yaml

Known third-party components:
- zlib 0.95P
- bzip2 1.0.2
- U-Boot Legacy Image Support, version Unknown

## TestSampleView
Included paths:
- BSP/SampleCode
- BSP/ThirdParty
- Tools

Scope definition:
- Documents/SBOM/test-sample-scope.yaml

Known third-party component:
- FreeRTOS-Kernel 10.1.1
- Linux USB HIDBP Keyboard Support, version Unknown
- USB Video Class Header Support, version Unknown

## Component Review

| View           | Component                   | Version | License                | Review Result                                                |
|----------------|-----------------------------|---------|------------------------|--------------------------------------------------------------|
| ProductView    | zlib                        | 0.95P   | Zlib                   | Approved                                                     |
| ProductView    | bzip2                       | 1.0.2   | LicenseRef-bzip2-1.0.2 | Technical review complete; Product Security approval pending |
| ProductView    | U-Boot Legacy Image Support | Unknown | GPL-2.0-or-later       | Approved with documented unknown version                     |
| TestSampleView | FreeRTOS-Kernel             | 10.1.1  | MIT                    | Approved                                                     |
| TestSampleView | Linux USB HIDBP Keyboard Support | Unknown | GPL-2.0-or-later | Identified from source evidence |
| TestSampleView | USB Video Class Header Support | Unknown | GPL-2.0-or-later | Supplier and upstream version require manual confirmation |

## bzip2 Build-Scope Review
- The modified bzip2 source is distributed under `Loaders/SpiLoader_gzip/ziplib` and remains in ProductView.
- Inclusion of the bzip2 runtime in inspected default GCC and KEIL project configurations was not demonstrated.
- The observed default runtime decompression path uses zlib.
- Distributed source capability is not treated as proof of default-build linkage.

## bzip2 Vulnerability Review

| Vulnerability  | Technical Disposition       | Source Remediation | Technical Review | Approval |
|----------------|-----------------------------|--------------------|------------------|----------|
| CVE-2019-12900 | patched-by-source-backport  | verified           | complete         | pending  |
| CVE-2005-0953  | vulnerable-code-not-present | not applicable     | complete         | pending  |
| CVE-2005-1260 | not-in-default-build | not assessed | complete | pending |
| CVE-2008-1372 | not-in-default-build | not assessed | complete | pending |
| CVE-2010-0405 | not-in-default-build | unverified | complete | pending |
| CVE-2011-4089  | vulnerable-code-not-present | not applicable     | complete         | pending  |

Verification boundaries:
- Source presence, guard ordering, array-bound consistency, inspected build scope, evidence integrity, and technical dispositions were reviewed.
- Target compilation, SAST, malformed-input negative testing, and normal decompression regression testing remain pending unless separate evidence is provided.
- Any future bzlib enablement requires renewed source-remediation, build-inclusion, SAST, and regression review.

## Evidence Review
Reviewed evidence documents:
- Documents/SBOM/evidence/evidence_zlib.md
- Documents/SBOM/evidence/evidence_bzip2.md
- Documents/SBOM/evidence/evidence_uboot_image.md
- Documents/SBOM/evidence/evidence_freertos.md
- Documents/SBOM/evidence/evidence_usb_host_legacy.md

Evidence status:
- Component identification: Complete
- Version evidence: Complete, except documented unknown U-Boot version
- License evidence: Complete for current declarations
- bzip2 technical evidence: Complete
- Evidence source hashes: Maintained in the evidence and SCA declarations and verified against the selected clean release commit

## Exclusion Review
The following items are classified as Nuvoton first-party components or tools:
- JPEG Driver
- AutoWriter
- TurboWriter
- NRomMaker
- LCMTest
- HID Transfer FS
- HID Transfer HS
- HID Firmware Update FS
- HID Firmware Update HS

Detailed records are maintained in `Documents/SBOM/exclusions.yaml`. Exact tool paths must match the distributed Git tree during release generation.

## Durable Input Review
- Scope definitions: Complete
- ProductView SCA declarations: Complete
- TestSampleView SCA declarations: Complete
- Component declarations: Complete
- Evidence documents: Complete
- Exclusion review: Complete
- Component inventory: Complete
- Baseline document: Complete
- Maintenance policy: Complete
- `.gitattributes` rule `Documents/SBOM/** -text`: Present

## Release Workflow
For each release:
1. Confirm the Git working tree is clean.
2. Record the full Git commit and Git-derived version.
3. Revalidate all evidence-source hashes and component closure hashes.
4. Generate ProductView and TestSampleView SBOMs from the selected commit.
5. Generate the SBOM manifest.
6. Run ProductView and TestSampleView vulnerability scans.
7. Validate all CycloneDX documents.
8. Regenerate VEX and Product Security evidence against the new release identity.
9. Generate both SBOM checker JSON files.
10. Build and verify the versioned SVN release directory and checksum manifest.
11. Obtain Product Security approval.
12. Commit and verify the SVN release directory.
13. Update `latest-release.txt` only after verification.

## Review Conclusion
The durable Git SBOM inputs are technically reviewed. Release-specific generation and validation records are maintained in the SVN release package. Product Security approval of the documented bzip2 dispositions is required before formal publication.
