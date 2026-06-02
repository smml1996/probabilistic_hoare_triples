//
// Created by Stefanie Muroya Lei on 01.06.26.
//

#include "utils.hpp"

inline LOGFile LOG;

#ifndef TEST_UTILS_H
#define TEST_UTILS_H

void test_setup(const string &test_module) {
    MyFloat::precision = 8;
    Config::is_debug = true;
    LOG.open(filesystem::path("..")/"tests"/ "logs" / ("log_"+ test_module + ".txt"));
}

void test_cleanup() {
    LOG.close();
}

#endif
