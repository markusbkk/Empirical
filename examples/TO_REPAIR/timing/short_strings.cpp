/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file short_strings.cpp
 *  @brief Code comparing various methods of accessing entries in an unsorted_map.
 *
 *  We are comparing the timings for accessing an unsorted map using:
 *  * Numerical IDs
 *  * Short strings
 *  * Longer strings
 *  * Floating point IDs
 */

#include <algorithm>     // For std::sort
#include <ctime>         // For std::clock
#include <functional>    // For std::function
#include <iostream>      // For std::cout, etc.
#include <string>
#include <unordered_map>
#include <vector>

#include "emp/datastructs/StringMap.hpp"

using emp::StringID;

constexpr size_t NUM_ENTRIES = 10000;
constexpr size_t LONG_STR_SIZE = 40;
constexpr size_t EVAL_STEPS = 10000000;

int ToIntID(int id) { return id; }
std::string ToStringID(int id) { return std::to_string(id); }
std::string ToLongStringID(int id) {
  std::string out_str = ToStringID(id);
  out_str += std::string('*', LONG_STR_SIZE - out_str.size() );
  return out_str;
}
double ToFloatID(int id) { return 0.5 + (double) id; }

void TimeFun(const std::string & name, std::function<int()> fun) {
  std::clock_t base_start_time = std::clock();

  int result = fun();

  std::clock_t base_tot_time = std::clock() - base_start_time;
  std::cout << name
            << " result = " << result
            << "   time = " << ((double) base_tot_time) / (double) CLOCKS_PER_SEC
            << " seconds." << std::endl;
}

template <typename CTYPE, typename ITYPE>
void RunTest(const std::string & name, CTYPE & container, std::function<ITYPE(int)> index_fun) {
  TimeFun(name, [&container, &index_fun](){
    ITYPE id1 = index_fun(42), id2 = index_fun(100), id3 = index_fun(1000);
    for (size_t i = 0; i < EVAL_STEPS; i++) {
      container[id1] += container[id2];
      container[id3] -= container[id2];
      container[id2] = container[id3] / 2 + 1000;
    }
    return container[id1];
  });
}

int main()
{
  // Create the maps.
  std::unordered_map<int, int> int_ids;
  std::unordered_map<std::string, int> short_strings;
  std::unordered_map<std::string, int> long_strings;
  std::unordered_map<double, int> float_ids;
  std::vector<int> vector_index(NUM_ENTRIES);
  std::unordered_map<size_t, int> emp_string_ids;
  emp::StringMap<int> string_map;
  emp::StringMap<int> string_map_pp;

  // Fill out the maps.
  for (size_t i = 0; i < NUM_ENTRIES; i++) {
    int_ids[i] = i;
    short_strings[ToStringID(i)] = i;
    long_strings[ToLongStringID(i)] = i;
    float_ids[ToFloatID(i)] = i;
    vector_index[i] = i;
    emp_string_ids[StringID(ToStringID(i)).ToValue()] = i;
    string_map[ToStringID(i)] = i;
    string_map_pp[ToStringID(i)] = i;
  }

  std::cout << "Starting tests!" << std::endl;

  TimeFun("Direct Variables      ", []() {
    int val1 = 42, val2 = 100, val3 = 1000;
    for (size_t i = 0; i < EVAL_STEPS; i++) {
      val1 += val2;
      val3 -= val2;
      val2 = val3 / 2 + 1000;
    }
    return val1;
  });

  RunTest<std::vector<int>, int>                            ("Vector Indexing       ", vector_index, ToIntID);

  RunTest<std::unordered_map<int, int>, int>                ("std::map<int> IDs     ", int_ids, ToIntID);
  RunTest<std::unordered_map<std::string, int>, std::string>("map of short strings  ", short_strings, ToStringID);
  RunTest<std::unordered_map<std::string, int>, std::string>("map of long string    ", long_strings, ToLongStringID);
  RunTest<std::unordered_map<double, int>, double>          ("map of doubles        ", float_ids, ToFloatID);


  TimeFun("str map w/literal IDs ", [&short_strings]() {
    for (size_t i = 0; i < EVAL_STEPS; i++) {
      short_strings["42"] += short_strings["100"];
      short_strings["1000"] -= short_strings["100"];
      short_strings["100"] = short_strings["1000"] / 2 + 1000;
    }
    return short_strings["42"];
  });

  TimeFun("emp::StringIDs        ", [&emp_string_ids]() {
    for (size_t i = 0; i < EVAL_STEPS; i++) {
      emp_string_ids[EMP_STRING_ID("42").ToValue()] += emp_string_ids[EMP_STRING_ID("100").ToValue()];
      emp_string_ids[EMP_STRING_ID("1000").ToValue()] -= emp_string_ids[EMP_STRING_ID("100").ToValue()];
      emp_string_ids[EMP_STRING_ID("100").ToValue()] = emp_string_ids[EMP_STRING_ID("1000").ToValue()] / 2 + 1000;
    }
    return emp_string_ids[EMP_STRING_ID("42").ToValue()];
  });

  TimeFun("emp::StringMap        ", [&string_map]() {
    for (size_t i = 0; i < EVAL_STEPS; i++) {
      string_map["42"] += string_map["100"];
      string_map["1000"] -= string_map["100"];
      string_map["100"] = string_map["1000"] / 2 + 1000;
    }
    return string_map["42"];
  });

  TimeFun("emp::StringMap PreProc", [&string_map]() {
    for (size_t i = 0; i < EVAL_STEPS; i++) {
      string_map[EMP_STRING_ID("42")] += string_map[EMP_STRING_ID("100")];
      string_map[EMP_STRING_ID("1000")] -= string_map[EMP_STRING_ID("100")];
      string_map[EMP_STRING_ID("100")] = string_map[EMP_STRING_ID("1000")] / 2 + 1000;
    }
    return string_map[EMP_STRING_ID("42")];
  });
}
