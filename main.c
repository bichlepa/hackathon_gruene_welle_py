#include <stdio.h>
#include "stdint.h"
#include "math.h"

//Defines
#define ALARM_TRIGGER_DIST 45
#define ALARM_DISABLE_INCREASE 20
#define DEG_IN_RAD 0.0174533

typedef struct 
{
  float lat;
  float lon;
}ampel_pos_t;
ampel_pos_t ampel_pos[2] = 
{
  {
    48.4795867,
    7.921582
  },
  {
    48.4790187,
    7.921641
  }
};
uint32_t ampel_gpio[2] = {17, 27};


//Prototypes
void init_gpio(void);
float distance(float latDyn, float lonDyn, float latStat, float lonStat);
int32_t get_data(float *plat,float *plon);
void set_alarm(uint32_t ampel, uint32_t toggle);
void ampel_dist(void);

int main()
{  
  ampel_dist();

  init_gpio();
  float templat, templon, tempdist;
  float mindist[sizeof(ampel_pos) / sizeof(ampel_pos_t)];
  int32_t leaving[sizeof(ampel_pos) / sizeof(ampel_pos_t)];
  int32_t ret;

  for(uint32_t i=0; i < sizeof(ampel_pos) / sizeof(ampel_pos_t); i++)
  {
    mindist[i] = ALARM_TRIGGER_DIST;
    leaving[i] = 0;
  }

  while(1)
  {
    ret = get_data(&templat,&templon);
    printf("data ret: %i, lat: %f, lon: %f\n",ret, templat, templon);

    if(ret == 1)
    {
      for(uint32_t i=0; i < sizeof(ampel_pos) / sizeof(ampel_pos_t); i++)
      {
        //dist Ampel1
        tempdist = distance(templat, templon, ampel_pos[i].lat, ampel_pos[i].lon);
        printf("dist: %f mindist: %f\n", tempdist, mindist[i]);
        if(tempdist <= ALARM_TRIGGER_DIST * 0.9)
        {
          if (mindist[i] > tempdist)
          {
            mindist[i] = tempdist;
          }
          if ((tempdist > (mindist[i] + ALARM_DISABLE_INCREASE)) || leaving[i])
          {
            printf("leaving\n");
            leaving[i] = 1;
            set_alarm(i,0);
          }
          else
          {
            printf("alarm\n");
            set_alarm(i,1);
          }
          
        }
        else
        {
          mindist[i] = ALARM_TRIGGER_DIST;
          printf("outside\n");
          leaving[i] = 0;
          set_alarm(i,0);
        }
      }
    }
    else
    {
      system("sleep 1");
    }
  }
  return 0;
}

void ampel_dist(void)
{
  float dist_lat, dist_lon;
  dist_lat = ampel_pos[0].lat - ampel_pos[1].lat;
  dist_lon = ampel_pos[0].lon - ampel_pos[1].lon;
  printf("dist lat: %f\n", dist_lat);
  printf("dist lon: %f\n", dist_lon);
}

//Getting GPS Data from Dropbox
int32_t get_data(float *plat,float *plon)
{
  FILE *fp;
  int32_t ret;
  //-O gebe als nächstes Dateipfad an
  //-q gibt weniger auf Konsole aus
  system("wget -q -O /home/pi/position.txt https://www.dropbox.com/s/4vffhap8zs31b5t/position.txt?dl=1");
  fp = fopen("/home/pi/position.txt", "r");
  if(fp != 0)
  {
    ret = fscanf(fp, "%f,%f", plat, plon);
    fclose(fp);
    if(ret == 2)
    {
      ret = 1;
    }
    else
    {
      ret = 0;
    }
  }
  else
  {
    ret = 0;
  }
  return ret;
}


//Calculate distance between two points
float distance (float latDyn, float lonDyn, float latStat, float lonStat)
{
  float dist = 0.0;
  //dist in kilometers!
  latDyn *= DEG_IN_RAD;
  lonDyn *= DEG_IN_RAD;
  latStat *= DEG_IN_RAD;
  lonStat *= DEG_IN_RAD;
  dist = 6378.388 * acos( sin(latDyn) * sin(latStat) + cos(latDyn) * cos(latStat) * cos(lonStat - lonDyn) );
  dist *= 1000.0;
  return dist;
}


//Setting GPIO PINs
void set_alarm(uint32_t ampel, uint32_t toggle)
{
  char str[50];
  sprintf(str, "echo %i > /sys/class/gpio/gpio%i/value", toggle, ampel_gpio[ampel]);
  system(str);
}









void init_gpio(void)
{
  //Export Pins
  system("sudo echo 17 > /sys/class/gpio/export");
  system("sudo echo 27 > /sys/class/gpio/export");
  //Setting root rights
  system("sudo chmod 660 /sys/class/gpio/gpio17/direction");
  system("sudo chmod 660 /sys/class/gpio/gpio17/value");
  system("sudo chmod 660 /sys/class/gpio/gpio27/direction");
  system("sudo chmod 660 /sys/class/gpio/gpio27/value");
  //Directions
  system("echo out > /sys/class/gpio/gpio17/direction"); 
  system("echo out > /sys/class/gpio/gpio27/direction"); 
  //Initvalues
  system("echo 0 > /sys/class/gpio/gpio17/value");
  system("echo 0 > /sys/class/gpio/gpio27/value");

  system("echo start...");
}


