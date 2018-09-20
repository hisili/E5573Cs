#include "hisi_battery_data.h"
#include "auto-generate/hisi_battery_data_array.c"

#define BATTERY_DATA_ARRY_SIZE sizeof(battery_data_array)/sizeof(battery_data_array[0])

struct hisi_smartstar_coul_battery_data *get_battery_data(unsigned int id_voltage)
{
    int i;

    for (i=(BATTERY_DATA_ARRY_SIZE - 1); i>0; i--){
        if ((id_voltage >= battery_data_array[i]->id_voltage_min)
            && (id_voltage <= battery_data_array[i]->id_voltage_max)){
            break;
        }
    }

    return battery_data_array[i];
}


