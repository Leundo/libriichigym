//
//  riichi_util.cpp
//  libriichigym
//
//  Created by Leundo on 2024/04/23.
//

#include <cstring>
#include <climits>
#include <cctype>
#include "riichi_util.hpp"


namespace riichi {

int strncmpci(const char * lhs, const char * rhs, size_t num) {
    int ret_code = 0;
    size_t chars_compared = 0;

    if (!lhs || !rhs) {
        ret_code = INT_MIN;
        return ret_code;
    }

    while ((chars_compared < num) && (*lhs || *rhs)) {
        ret_code = tolower((int)(*lhs)) - tolower((int)(*rhs));
        if (ret_code != 0) {
            break;
        }
        chars_compared++;
        lhs++;
        rhs++;
    }

    return ret_code;
}

}
