#include "chg_hardware_data.h"
#include "chg_hardware_data_array.c"

#define BATTERY_DATA_ARRY_SIZE sizeof(chg_hardware_data_array)/sizeof(chg_hardware_data_array[0])

struct chg_hardware_data *chg_get_hardware_data(unsigned int id_voltage)
{
    int i;

    for (i=(BATTERY_DATA_ARRY_SIZE - 1); i>0; i--){
        if ((id_voltage >= chg_hardware_data_array[i]->id_voltage_min)
            && (id_voltage <= chg_hardware_data_array[i]->id_voltage_max)){
            break;
        }
    }

    return chg_hardware_data_array[i];
}

int chg_get_hardware_data_id(void)
{
    // TODO: get chg hardware data by product hardware id or something esle.
    /*stub*/
    return 777;
}
