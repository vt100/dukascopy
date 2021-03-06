/*
Copyright 2013 Michael O'Keeffe (a.k.a. ninety47).

This file is part of ninety47 Dukascopy toolbox.

The "ninety47 Dukascopy toolbox" is free software: you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation, either version 3 of the License,
or any later version.

"ninety47 Dukascopy toolbox" is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
Public License for more details.

You should have received a copy of the GNU General Public License along with
"ninety47 Dukascopy toolbox".  If not, see <http://www.gnu.org/licenses/>.
*/

#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem.hpp>
#include <ninety47/dukascopy.h>
#include <ninety47/dukascopy/defs.h>
#include <cstring>
#include <iostream>
#include <fstream>

#define STRINGIZE(x) #x
#define STRINGIZE_VALUE_OF(x) STRINGIZE(x)

#ifndef TEST_DATA_PREFIX
#define TEST_DATA_PREFIX ..
#endif

namespace fs = boost::filesystem;
namespace pt = boost::posix_time;
namespace gr = boost::gregorian;


int main(void) {
    unsigned char *buffer;
    size_t buffer_size;
    const char *test_data_prefix = STRINGIZE_VALUE_OF(TEST_DATA_PREFIX);
    const char *test_data_suffix =
        "/test/data/compressed/GBPJPY/2012/11/03/01h_ticks.bi5";
    int counter;
    size_t raw_size = 0;
    size_t filename_len = strlen(test_data_prefix) + strlen(test_data_suffix) + 1;
    char *filename = new char[filename_len];
    snprintf(
        filename, filename_len, "%s%s", test_data_prefix, test_data_suffix);

    fs::path p(filename);
    if (fs::exists(p) && fs::is_regular(p)) {
        buffer_size = fs::file_size(p);
        buffer = new unsigned char[ buffer_size ];
    } else {
        std::cout << "Error: couldn't access the data file. |"
                  << filename << "|" <<  std::endl;
        return 2;
    }

    std::ifstream fin;
    fin.open(filename, std::ifstream::binary);
    fin.read(reinterpret_cast<char*>(buffer), buffer_size);
    fin.close();

    pt::ptime epoch(gr::date(2012, 12, 3), pt::hours(1));
    n47::tick_data *data = n47::read_bi5(
            buffer, buffer_size, epoch, PV_YEN_PAIR, &raw_size);
    n47::tick_data_iterator iter;

    if (data == 0) {
        std::cout << "Failure: Failed to load the data!" << std::endl;
        return 0;
    }

    if (data->size() != (raw_size / n47::ROW_SIZE)) {
        std::cout << "Failure: Loaded " << data->size()
                  << " ticks but file size indicates we should have loaded "
                  << (raw_size / n47::ROW_SIZE) << std::endl;
        return 0;
    }

    std::cout << "time, bid, bid_vol, ask, ask_vol" << std::endl;
    counter = 0;
    for (iter = data->begin(); iter != data->end(); iter++) {
        std::cout << ((*iter)->epoch + (*iter)->td) << ", "
                  << (*iter)->bid << ", " << (*iter)->bidv << ", "
                  << (*iter)->ask << ", " << (*iter)->askv << std::endl;
        counter++;
    }
    std::cout << ".end." << std::endl << std::endl
              << "From " << buffer_size << " bytes we read " << counter
              << " records." << std::endl
              << raw_size << " / " << n47::ROW_SIZE << " = "
              << (raw_size / n47::ROW_SIZE) << std::endl;

    delete data;
    delete[] buffer;
    delete filename;
}
