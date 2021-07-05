/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
 *
 *  @file  UrlParams.hpp
 *  @brief Get an unordered_map containing url query key/value parameters.
 *
 */

#ifndef EMP_WEB_UrlParams_H
#define EMP_WEB_UrlParams_H

#include <map>
#include <string>

#include "JSWrap.hpp"
#include "js_utils.hpp"

namespace emp {
namespace web {

  std::multimap<std::string, emp::vector<std::string>> GetUrlParams() {

    emp::vector<emp::vector<std::string>> incoming;

    MAIN_THREAD_EM_ASM({
      // Custom parsing of query string to use '+' as key separator
      // and %20 as regular space so that strings can have spaces
      emp_i.__outgoing_array = location.search.substring(1).split('&'
      ).map(
        expr => expr.split("=")
      ).map(
        (list) => [list[0].split("+").join(" ")].concat(!list[1] ? [""] : list[1].split('+'))
      ).map(
        list => list.map(decodeURIComponent)
      ).map(
        p => p[0].split(" ").join("").length == 0
          ?  ["_illegal", "_empty=" + p[1]] : p
      ).map(
        p => p[0].includes(" ") ? ["_illegal", p[0] + "=" + p[1]] : p
      ).map(
        p => p.filter(Boolean)
      );
    });

    emp::pass_vector_to_cpp(incoming);

    std::multimap<std::string, emp::vector<std::string>> res;

    for (const auto & pack : incoming) {
      res.insert({
        pack.front(),
        emp::vector<std::string>(
          std::next(std::begin(pack)),
          std::end(pack)
        )
      });
    }

    return res;

  }

}
}

#endif
