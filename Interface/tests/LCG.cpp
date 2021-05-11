#include <iostream>
#include <array>
#include <boost/histogram.hpp>
#include <boost/format.hpp>
#include <boost/progress.hpp>

constexpr long int addresses = (2 << 21);

std::array<int, addresses> visits = {0};

constexpr inline long int LCG(long int previous) {
    constexpr long int a = 5;
    constexpr long int c = 3;
    constexpr long int m = addresses;

    return (a * previous + c) % m;
}

int main() {
    long int x = 1;

    boost::progress_display show_progress(2 * addresses);
    for (int i = 0; i < addresses; i++) {
        x = LCG(x);
        visits.at(x)++;
        ++show_progress;
    }

    // Histogram
    auto h = boost::histogram::make_histogram(
            boost::histogram::axis::regular<>(5, 0, 5)
    );

    for (auto&& addressVisits : visits) {
        h(addressVisits);
        ++show_progress;
    }

    // iterate over bins
    for (auto&& bin : indexed(h)) {
        std::cout << boost::format("bin %i [ %.1f, %.1f ): %i\n")
                     % bin.index() % bin.bin().lower() % bin.bin().upper() % *bin;
    }
}