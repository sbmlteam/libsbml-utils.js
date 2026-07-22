---
title: 'Validation & Conversion'
tags:
  - libsbml
  - systems biology
  - computational modeling
authors:
  - name: Frank T. Bergmann
    orcid: 0000-0001-5553-4702
    affiliation: 1
  - name: Sarah M. Keating
    orcid: 0000-0002-3356-3542
    affiliation: 2

affiliations:
 - name: BioQUANT/COS, Heidelberg University, Heidelberg, Germany
   index: 1
 - name: University College London, London, GB
   index: 2
date: 28 May 2026
bibliography: paper.bib

---

# Summary

libsbml-utils.js is a lightweight, modern TypeScript/JavaScript toolkit for reading, inspecting, transforming, and validating SBML (Systems Biology Markup Language) models in both browser and Node.js environments. It provides a simple, composable API for common SBML tasks—parsing models, querying species and reactions, editing annotations, validating against the SBML specification, and exporting subsets—without native bindings or heavyweight runtimes. The SBML online Validator is built to use this library as its client-side engine, demonstrating production-grade, in-browser validation at scale and validating the library’s portability and performance in constrained environments. By bridging SBML workflows into the JavaScript ecosystem, libsbml-utils.js enables reproducible browser-based analyses, interactive teaching materials, and integration with front-end applications, notebooks, and CI pipelines.

# Statement of need

SBML is a community standard for computational models of biochemical networks. A mature library (libSML - NEED CITATION) exists in C/C++ with multiple language bindings, but many research, education, and dissemination workflows now run in web contexts or JavaScript-based stacks (front-end applications, serverless functions, CI, interactive docs). Practitioners face three recurrent pain points:

- Limited first-class JS/TS support: Browser-centric prototyping and visualization often require server-side roundtrips or native builds just to parse or transform SBML.
- Friction for lightweight tasks: Simple operations—schema validation, extracting subnetworks, checking units/annotations, or normalizing identifiers—shouldn’t require complex toolchains.
- Integration gaps: Teaching materials, dashboards, and data portals need JS-native SBML utilities that compose with modern bundlers, type systems, test frameworks, and web workers, and that run securely in sandboxed contexts.

libsbml-utils.js addresses these needs with a pure JS/TS utility layer focused on portability and ergonomics:

Runs in both browser and Node.js, supporting interactive, in-page exploration and server-side automation.
Typed, composable utilities that mirror common SBML concepts (models, compartments, species, parameters, reactions, rules) and tasks (parse, query, edit, validate, export).
Standards-conscious validation workflows that surface schema and consistency issues early—powering the SBML online Validator, which showcases real-time, client-side validation using this library.
Seamless interop with the broader JS ecosystem (e.g., D3/Vega for visualization; modern build tools; testing in CI), reducing the barrier to integrate SBML into web apps, lessons, and pipelines.

By lowering the barrier to standards-based modeling in JavaScript, libsbml-utils.js broadens access across research, education, and tooling, from web-based editors and viewers to automated checks in continuous integration.

# Target Audience

- Researchers and practitioners in systems and synthetic biology who need to parse, validate, and transform SBML within JavaScript-driven analysis or visualization workflows.

- Web application and portal developers who require client- or server-side SBML utilities with no native dependencies.

- Educators building interactive, browser-based teaching materials, assignments, or tutorials that illustrate SBML structure and model concepts.

- Tool builders and CI maintainers implementing lightweight SBML checks, schema validation, and transformations in Node.js pipelines.

- Interoperability advocates integrating SBML with adjacent JS ecosystems (graph libraries, annotation/provenance tools, documentation frameworks).

# Acknowledgements

This project has been possible thanks to LiBiS, the Baden-Württemberg Institute for Bioinformatics Infrastructure and to the BMBF funded de.NBI initiative (031L0104A, W-de.NBI-016).



# References
