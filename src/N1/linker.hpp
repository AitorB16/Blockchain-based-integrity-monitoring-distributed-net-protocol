#ifndef LINKER_HPP
#define LINKER_HPP

#include <syslog.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <unistd.h>
#include <iostream>
#include <systemd/sd-bus.h>

#include "network.hpp"

static sd_bus_slot *slot = NULL;
static sd_bus *bus = NULL;

int method_updateHash(sd_bus_message *m, void *userdata, sd_bus_error *ret_error);

/* Definition of the bus object and method*/
static const sd_bus_vtable fscheck_vtable[] = {
    SD_BUS_VTABLE_START(0),
    SD_BUS_METHOD("updateHash", "s", "i", method_updateHash, SD_BUS_VTABLE_UNPRIVILEGED | SD_BUS_VTABLE_METHOD_NO_REPLY),
    SD_BUS_VTABLE_END};

void thread_process();

#endif