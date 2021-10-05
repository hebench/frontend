// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef __COMMON_No_Copy_H_7e5fa8c2415240ea93eff148ed73539b
#define __COMMON_No_Copy_H_7e5fa8c2415240ea93eff148ed73539b

// remove copy mechanisms
#define DISABLE_COPY(className)            \
    className(const className &) = delete; \
    className &operator=(const className &) = delete;

// remove move mechanisms
#define DISABLE_MOVE(className)       \
    className(className &&) = delete; \
    className &operator=(className &&) = delete;

#endif // defined __COMMON_No_Copy_H_7e5fa8c2415240ea93eff148ed73539b
