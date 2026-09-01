# BSP SBOM Maintenance Policy

## Purpose
This policy defines maintenance, review, validation, security review, and release requirements for N9H20 BSP Software Bill of Materials deliverables.

## Scope
This policy applies to:
- N9H20 ProductView SBOM
- N9H20 TestSampleView SBOM
- Manual SCA component declarations
- Component evidence documents
- Exclusion records
- Vulnerability-scan results
- CycloneDX VEX documents
- SBOM checker evidence
- Versioned SVN SBOM release packages

## Repository Responsibilities

## Git
Git stores durable SBOM generation inputs and governance records under `Documents/SBOM`, including scope definitions, manual SCA declarations, evidence, exclusions, inventory, baseline, policy, and review records.

Generated release SBOMs, scan outputs, VEX documents, validation reports, release README files, and release checksum files are not committed to Git as release outputs.

## SVN
SVN stores immutable, versioned SBOM release outputs under `bsp/<bsp-name>/<release-id>`. The outer `latest-release.txt` points to the verified latest release and is updated only after the versioned release directory is committed and verified.

## Maintenance Triggers
The SBOM baseline must be reviewed and regenerated when any of the following occurs:
- A BSP release tag or Git commit changes
- A third-party component is added or removed
- A third-party component version changes
- Component source code is modified
- Component license information changes
- ProductView or TestSampleView scope changes
- Binary libraries or executable files change
- The SBOM generation, validation, or vulnerability-scanning toolchain changes
- A vulnerability finding requires metadata correction or technical disposition
- Evidence files or exclusion decisions change

## Required Activities for Every BSP Release
- Confirm the clean BSP Git commit and Git-derived release identifier.
- Review ProductView and TestSampleView scope definitions.
- Validate manual SCA JSON and corresponding Markdown declarations.
- Revalidate source evidence and canonical component hashes.
- Regenerate ProductView and TestSampleView SBOM files.
- Merge all approved manual SCA component declarations.
- Apply required binary and license enrichment.
- Validate final CycloneDX documents with an approved validator.
- Generate the SBOM manifest.
- Run vulnerability scans for both SBOM views.
- Review newly introduced or changed findings.
- Preserve original scanner findings without deletion or permanent suppression unless formally approved.
- Prepare VEX when contextual exploitability analysis is required.
- Generate `sbom-checker-product.json` and `sbom-checker-test-sample.json`.
- Prepare README and release checksum manifest.
- Verify all release artifacts byte-for-byte.
- Obtain Product Security approval when required.
- Commit and verify the versioned SVN release directory.
- Update `latest-release.txt` only after the release directory is verified.

## Manual SCA Maintenance
Each manually declared component must include:
- Component name and version, or `Unknown` with justification
- Component type and scope
- Supplier or author
- Stable BOM reference and package URL
- License identifier
- Source location
- Version and license evidence
- Modification status and component origin
- Usage or integration context

Each SCA JSON file must have a corresponding Markdown declaration and approved evidence document.

## Evidence Requirements
Evidence documents must identify:
- Source files used for component identification
- Version and license determination methods
- Integration and usage location
- Modification status
- SHA-256 values for primary evidence files
- SCA definition mapping
- Review status and unresolved verification boundaries

Evidence hashes must be regenerated whenever a covered source file changes.

## Exclusion Requirements
An exclusion may be recorded only after manual review confirms that the item is first-party, proprietary, generated, or outside the relevant third-party inventory. Each exclusion must document the item name, source path, classification, reason, and evidence when available. An exclusion must not suppress an unresolved third-party, license, or vulnerability finding.

## Vulnerability Management
Vulnerability scans must run against both ProductView and TestSampleView SBOMs. SBOM generation and vulnerability scanning are separate DevSecOps stages. Scanner database or network failure does not invalidate an already generated and validated SBOM, but the release package must record the incomplete scan state.

Release criteria include:
- Scanner execution completes successfully or an incomplete status is explicitly recorded
- Critical findings are reviewed before release
- Accepted findings have documented technical dispositions
- Product Security approval is recorded when required
- Suppressions have documented justification, ownership, and approval
- Original scanner evidence is preserved
- VEX states are not finalized before approval

## Validation Requirements
Before release:
- CycloneDX schema validation must pass
- Manual SCA merge results must match the component inventory
- BOM references and dependency references must resolve
- Required license declarations must be present
- Developer-specific absolute paths must not appear in final deliverables
- Manifest references must resolve
- Product and Test Sample checker evidence must report success
- Release checksums must be generated and independently verified

## Release Package Requirements
Each versioned SVN release package must contain:
- ProductView CycloneDX SBOM
- TestSampleView CycloneDX SBOM
- SBOM manifest
- README
- `sbom-checker-product.json`
- `sbom-checker-test-sample.json`
- SHA-256 checksum manifest
- ProductView vulnerability-scan result
- TestSampleView vulnerability-scan result
- Vulnerability-scan summary
- Approved VEX and Product Security review evidence when required

The release directory name must use the Git-derived BSP version. Checksum entries use `<hash> *relative-path` format.

## Roles and Responsibilities

## BSP Maintainer
- Maintains source and release information
- Confirms the clean BSP Git baseline
- Reviews scope changes
- Supports component-source identification

## SBOM Maintainer
- Maintains SCA definitions, evidence, inventory, exclusions, and governance records
- Generates and validates SBOM files
- Prepares the SVN release package and checksums

## Security Reviewer
- Reviews vulnerability results and contextual dispositions
- Reviews license and component-classification exceptions
- Approves security dispositions and VEX final states when required

## Review Frequency
The SBOM must be reviewed for every BSP release, when a third-party component or relevant advisory changes, when the pipeline changes, and at least once during each maintained release cycle.
