#ifndef __HISI_SMARTSTAR_COUL_H__
#define __HISI_SMARTSTAR_COUL_H__


/*DEBUG LEVEL*/
typedef enum
{
    COUL_MSG_ERR = 0,
    COUL_MSG_DEBUG,
    COUL_MSG_INFO,
    COUL_MSG_MAX
}COUL_MSG_TYPE;
/*******************************************************************
Function:      smartstar_battery_capacity
Description:   get the battery soc
Calls:         Battery management module
Input:         none
Output:        none
Return:        return the capacity of battery.
*******************************************************************/
int smartstar_battery_capacity(void);

#endif