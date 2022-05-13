
HEBench Report Compiler Overview                {#report_compiler_overview}
========================

[TOC]

## Description

The Report Compiler is a component of the HEBench frontend. For other frontend components refer to @ref frontend_overview .

The Report Compiler is designed to aid Test Harness in the generation of human-readable summaries, computation of in-depth statistics from benchmark reports, and an overview of a Test Harness run.

Users should use the report compiler to convert Test Harness run reports into human-readable data and easy-to-parse statistics and overview files. In a default run, Test Harness will produce compiled reports using default report compiler options; however, if users require a different configuration, they can run the report compiler manually through its command line interface to produce the desired data. See the user guide below for information on using the report compiler.

## User Guides

The documents and pages listed in this section detail how to use the report compiler to generate summaries, statistics and run overviews from Test Harness benchmark reports.
 
 - @ref report_compiler_usage_guide
 - @ref stats_n_summary_format
 - @ref overview_file_format

## Namespace

[Report generation namespace](@ref hebench::ReportGen)
[Report compiler library](@ref hebench::ReportGen::Compiler)

