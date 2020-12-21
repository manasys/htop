/*
htop - TemperatureMeter.c
(C) 2013 Ralf Stemmer
(C) 2014 Blair Bonnett
(C) 2020 Maxim Kurnosenko
Released under the GNU GPL, see the COPYING file
in the source distribution for its full text.
*/

#include "TemperatureMeter.h"

#include "ProcessList.h"
#include "CRT.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

/*{
#include "Meter.h"
}*/

int TemperatureMeter_attributes[] = {
   TEMPERATURE_COOL,
   TEMPERATURE_MEDIUM,
   TEMPERATURE_HOT,
};

static void TemperatureMeter_setValues(Meter* this, char* buffer, int len) {
   ProcessList* pl = this->pl;
   this->total = pl->totalTasks;
   this->values[0] = pl->runningTasks;
   snprintf(buffer, len, "%d/%d", (int) this->values[0], (int) this->total);
}

static void TemperatureMeter_display(Object* cast, RichString* out) {
   (void) cast;
   FILE *p;
   p = popen("/usr/bin/cat /sys/class/thermal/thermal_zone0/temp", "r");
   if(p == NULL) return;

   int textColor   = CRT_colors[METER_TEXT];
   int coolColor   = CRT_colors[TEMPERATURE_COOL];
   int mediumColor = CRT_colors[TEMPERATURE_MEDIUM];
   int hotColor    = CRT_colors[TEMPERATURE_HOT];

   int read;
   size_t len;
   char *line = NULL;
   long  temperature;
   while ((read = getline(&line, &len, p)) != -1) {
      // convert the string into an integer, this is necessary for further steps
      temperature = strtol(line, NULL, 10);
      if (temperature == LONG_MAX || temperature == LONG_MIN) continue;
      temperature /= 1000;

      // choose the color for the temperature
      int tempColor;
      if      (temperature < 60) tempColor = coolColor;
      else if (temperature < 70) tempColor = mediumColor;
      else                       tempColor = hotColor;

      // output the temperature
      char buffer[30];
      sprintf(buffer, "%ld", temperature);
      RichString_append(out, tempColor, buffer);
      RichString_append(out, textColor, "°C ");
   }

   free(line);
   pclose(p);
}

MeterClass TemperatureMeter_class = {
   .super = {
      .extends = Class(Meter),
      .display = TemperatureMeter_display,
      .delete = Meter_delete,
   },
   .updateValues = TemperatureMeter_setValues,
   .defaultMode = TEXT_METERMODE,
   .maxItems = 1,
   .total = 100.0,
   .attributes = TemperatureMeter_attributes,
   .name = "Temperature",
   .uiName = "Temperature Sensors",
   .caption = "Temperature: "
};
