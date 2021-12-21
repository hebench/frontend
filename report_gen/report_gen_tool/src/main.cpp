
// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include <iostream>
#include <stdexcept>

#include "hebench_report_cpp.h"

using namespace hebench::TestHarness::Report::cpp;

int main(int argc, char **argv)
{
    int retval = 0;

    try
    {
        std::string csv_filename;

        if (argc > 1)
        {
            csv_filename = argv[1];
        }
        else
        {
            csv_filename = "/data/storage/git-repos/hebench/frontend/test_harness/test.tmp/element_wise_addition_1000_3/latency/float32/5001/all_plain/plain/none/0/report.csv";
        }

        TimingReport report =
            TimingReport::loadReportFromCSVFile(csv_filename);

        std::cout << report.convert2CSV() << std::endl;

        if (report.getEventCount() <= 0)
        {
            std::cout << std::endl
                      << "The loaded report belongs to a failed task." << std::endl;
        } // end if
        else
        {
            hebench::TestHarness::Report::TimingReportEventSummaryC tre;
            std::cout << report.generateSummaryCSV(tre) << std::endl;
            std::cout << "Main event: " << tre.event_id << std::endl;
        } // end else
    }
    catch (std::exception &ex)
    {
        std::cout << "Error occurred with message: " << std::endl
                  << ex.what() << std::endl;
        retval = -1;
    }
    catch (...)
    {
        std::cout << "Unexpected error occurred!" << std::endl;
        retval = -1;
    }

    if (retval)
        std::cout << std::endl
                  << "Terminated with errors." << std::endl;
    else
        std::cout << std::endl
                  << "Complete!" << std::endl;

    return retval;
}
