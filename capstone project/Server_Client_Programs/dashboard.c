#include <stdio.h>
#include <time.h>
#include "dashboard.h"

// unicodes for colors
#define GREEN  "\033[1;32m"
#define RED    "\033[1;31m"
#define CYAN   "\033[1;36m"
#define RESET  "\033[0m"

static const char *get_status(float value, float min, float max)
{
    return (value >= min && value <= max) ? "NORMAL" : "ALERT";
}

static const char *get_color(float value, float min, float max)
{
    return (value >= min && value <= max) ? GREEN : RED;
}

void start_ui(const ems_data_t *data,
              const operating_range_t *range,
              int client_count)
{
    time_t ts = data->timestamp;
    struct tm *tm_info = localtime(&ts);

    printf(CYAN "============================================\n" RESET);
    printf(CYAN "     ENVIRONMENT MONITORING DASHBOARD\n" RESET);
    printf(CYAN "============================================\n" RESET);

    printf(" Industry      : %s\n", range->industry);
    printf(" Active Clients: %d\n", client_count);

    printf(" Time          : %02d:%02d\n\n",
           tm_info->tm_hour,
           tm_info->tm_min
           );

    printf("%s Temperature   : %.2f C  [%s]\n" RESET,
           get_color(data->temp, range->min_temp, range->max_temp),
           data->temp,
           get_status(data->temp, range->min_temp, range->max_temp));

    printf(" Operating Range: %.1f C - %.1f C\n\n",
           range->min_temp,
           range->max_temp);

    printf("%s Humidity      : %.2f %% [%s]\n" RESET,
           get_color(data->humidity, range->min_humidity, range->max_humidity),
           data->humidity,
           get_status(data->humidity,
                      range->min_humidity,
                      range->max_humidity));

    printf(" Operating Range: %.1f %% - %.1f %%\n",
           range->min_humidity,
           range->max_humidity);

    printf(CYAN "============================================\n" RESET);
}
