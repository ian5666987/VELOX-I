// code by ntu ael

#include <c8051f120.h>
#include <string.h>

#include "Velox_checksum.h"
#include "Velox_uptime.h"

#include "obdh_constant.h"
#include "obdh_globalvariables.h"
#include "obdh_schedule.h"

void Schedule_init() {
  memset(&schedule, 0, sizeof schedule);
  memset(&g_scheduleAdd, 0, sizeof g_scheduleAdd);
}

unsigned char Schedule_size() {
  unsigned char size;
  for (size = 0; size < SCHEDULE_SIZE_LIMIT; ++size)
    if (schedule[size].time == TAI_TIME_UNAVAILABLE)
      return size;
  return size; // only when scheduler is full
}

char Schedule_holdsEntry() {
  return schedule[0].time != TAI_TIME_UNAVAILABLE;
}

void Schedule_swap(unsigned char i1, unsigned char i2) {
  ScheduleItem temp;
  memcpy(&temp, &schedule[i1], sizeof(ScheduleItem)); // for faster computation memcpy is used instead of struct value assignment
  memcpy(&schedule[i1], &schedule[i2], sizeof(ScheduleItem));
  memcpy(&schedule[i2], &temp, sizeof(ScheduleItem));
}

void Schedule_bubbleSort() { // decent sorting algorithm for our purpose, Selection sort would also be an option
  char isSwapped;
  unsigned char size;
  unsigned char index;
  if (Schedule_holdsEntry()) // guarantees at least 1 entry in scheduler
    do {
      isSwapped = 0;
      size = Schedule_size(); // size is at least 1, because of Schedule_hasNext() condition
      for (index = 1; index < size; ++index)
        if (schedule[index - 1].time > schedule[index].time) {
          Schedule_swap(index - 1, index);
          isSwapped = 1;
        }
    } while (isSwapped);
}

void Schedule_remove(unsigned char beg, unsigned char end) { //start from 0, end in SCHEDULE_SIZE_LIMIT-1
  unsigned char index, num;
  unsigned char size;
  size = Schedule_size();
  if (beg < end && end <= size) { //It does not make sense to delete index that does not exist (>= schedule_size)
    num = end - beg;
    for (index = end; index < size; ++index) //For all the left-over element, shift them up
      memcpy(&schedule[index - num], &schedule[index], sizeof(ScheduleItem));
    for (index = size - num; index < size; ++index) //For all the left-over space, set them to zero
      memset(&schedule[index], 0, sizeof(ScheduleItem));
  }
}

char Schedule_handleAdd() {
  unsigned char size; // 0, 1, ..., SCHEDULE_SIZE_LIMIT-1
  char success = 0;
  if (g_scheduleAdd.id0 && g_scheduleAdd.id1 && g_scheduleAdd.id0 == g_scheduleAdd.id1) {
    g_scheduleAdd.id1 = 0;
    size = Schedule_size();
    success = size < SCHEDULE_SIZE_LIMIT && Uptime_isTaiTimeValid() && uptime.tai_time <= g_scheduleAdd.item.time; // scheduler still has vacant spots and new entry is not 'history'
    if (success)
      memcpy(&schedule[size], &g_scheduleAdd.item, sizeof(ScheduleItem)); // copy schedule item to global schedule
    // we deliberately do not memset addEntry to 0 so that gnd station has full flexibility
  }
  return success;
}

char Schedule_hasNext() { // this function is to be called periodically (ideally, once a second)!!
  if (Schedule_holdsEntry()) {
    Schedule_bubbleSort(); // the scheduler does not assume schedule is sorted, so that gnd station has the option to individually change the time
    return schedule[0].time <= uptime.tai_time;
  }
  return 0;
}
