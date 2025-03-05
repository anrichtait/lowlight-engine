/**
  \file test/printhelpers.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/

#include <algorithm>
#include <array>
#include <chrono>
#include <string>
#include <utility>

void printLeader(const std::string& leader);

template<size_t N>
void printTextColumns(const std::string (&a)[N]) {
    std::for_each(std::begin(a), std::end(a), [](const std::string& s) { printf(" %12s", s.c_str()); });
}

template<class... Cols>
void printTextColumns(Cols... columns) {
    std::array<std::string, sizeof...(columns)> a{std::forward<std::string>(columns)...};
    std::for_each(a.begin(), a.end(), [](const std::string& s) { printf(" %12s", s.c_str()); });
}

template<class P, class Ds, size_t N>
void printDurationColumns(const Ds (&a)[N]) {
    using D = std::chrono::duration<double, P>;
    std::for_each(std::begin(a), std::end(a), [](const D& d) { printf(" %12.5f", std::chrono::duration_cast<D>(d).count()); });
}

template<class P, class... Ds>
void printDurationColumns(Ds... durations) {
    using D = std::chrono::duration<double, P>;
    std::array<D, sizeof...(durations)> a{ std::chrono::duration_cast<D>(durations)... };
    std::for_each(a.begin(), a.end(), [](const D& d) { printf(" %12.5f", d.count()); });
}


#define PRINT_SECTION(name, text) \
    printf("\n\n"); \
    printf("--------------------------\n"); \
    printf("%s\n", name); \
    printf("--------------------------\n"); \
    printf("%s\n", text); \
    printf("--------------------------\n"); \
    printf("\n");

#define PRINT_HEADER(header) printf("\n%s\n--\n", header);
#define PRINT_TEXT(leader, ...) printLeader(leader); printTextColumns(__VA_ARGS__); printf("\n");

#define PRINT_NANO(leader, footer, ...) printLeader(leader); printDurationColumns<std::nano>(__VA_ARGS__); printf(" %s\n", footer);
#define PRINT_MICRO(leader, footer, ...) printLeader(leader); printDurationColumns<std::micro>(__VA_ARGS__); printf(" %s\n", footer);
#define PRINT_MILLI(leader, footer, ...) printLeader(leader); printDurationColumns<std::milli>(__VA_ARGS__); printf(" %s\n", footer);
#define PRINT_SECOND(leader, footer, ...) printLeader(leader); printDurationColumns<std::ratio<1>(__VA_ARGS__); printf(" %s\n", footer);
