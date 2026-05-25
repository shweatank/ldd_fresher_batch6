#ifndef DASHBOARD_H
#define DASHBOARD_H

#include "ems_data_model.h"

void start_ui(const ems_data_t *data,
              const operating_range_t *range,
              int client_count);

#endif
