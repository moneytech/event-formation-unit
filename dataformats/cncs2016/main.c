#include <string.h>
#include <stdio.h>

#define THRESH 150    // Threshold adc value for detecting a signal (for double events)

class DetMultiGrid {

struct hdr {
  const int HDR{0x01};
  const int DAT{0x02};
  const int END{0x00};
}

typedef struct {
   unsigned int    n_words       : 12;
   unsigned int    adc_res       : 3;
   unsigned int    out_format : 1;
   unsigned int    module_id  : 8;
   unsigned int    sub_header : 6;
   unsigned int    header_sig : 2;
} EventHeader;

typedef struct {
   unsigned int    adc_data   : 14;
   unsigned int    overflow   : 1;
   unsigned int    nop        : 1;
   unsigned int    channel       : 5;
   unsigned int    fix       : 9;
   unsigned int    data_sig   : 2;
} DataWord;

typedef struct {
   unsigned int    trigger       : 30;
   unsigned int    footer_sig   : 2;
} Footer;

union {
  EventHeader eh;
  DataWord dw;
  Footer ef;
} data;

}:

int main(int argc, char * argv[]) {
  printf("file: %s\n", argv[1]);
  printf("Threshold value: %d\n", THRESH);

  DetMultiGrid det;

  struct stat_t {
    int rx;      // file stats - Rx bytes
    int errors;  // event stat - header errors
    int events;  // event stat - number of events
    int multi;   // event stat - number of multi events
  } stat;

  bzero(&stat, sizeof(stat));

  FILE *f = fopen(argv[1], "r");
  if (f == NULL) {
    printf("Cannot find file \'%s\'\n", argv[1]);
    return -1;
  }


  int rxdata[8];    // event data
  while (fread(&data, sizeof(data), 1, f) > 0){
    stat.rx += sizeof(data);
    // Read Header
    if (det.data.eh.header_sig == DetMultigrid::hdr.HDR) {
      //printf("\n%7d Header 0x%02x, subhdr: %3d, module id: %3d, words: %3d\n",
      //        i, data.eh.header_sig, data.eh.sub_header, data.eh.module_id, data.eh.n_words);

      // Read Data
      int nread = data.eh.n_words;
      for (int j = 1; j < nread; j++) {
        if (fread(&data, sizeof(data), 1, f) > 0) {
          stat.rx += sizeof(data);

          if (data.dw.data_sig != ST_DAT) {
            printf("Data Error x%02x\n", data.dw.data_sig);
            stat.errors++;
            continue;
          }
          rxdata[j] = data.dw.adc_data;
          //printf("%7d Data:  chan id: %3d, adc: %5d\n", i, data.dw.channel, data.dw.adc_data);
        }
      }

      // Read Footer
      if (fread(&data, sizeof(data), 1, f) > 0) {
        stat.rx += sizeof(data);

        if (data.ef.footer_sig != ST_END) {
          printf("Footer Error: x%02x (bytes read: %d)\n", data.ef.footer_sig, stat.rx);
          stat.errors++;
          continue;
        }
        //printf("%7d Footer: timestamp %d\n", i, data.ef.trigger);
        stat.events++;
        if ((rxdata[1] > THRESH) && (rxdata[2] > THRESH) && (rxdata[3] > THRESH) && (rxdata[4] > THRESH) ) {
          stat.multi++;
          printf("%7d: Double Neutron Event (bytes read %d)\n", stat.events, stat.rx);
        }

      }
    }
  }
  printf("Bytes read:    %d\n", stat.rx);
  printf("Events:        %d\n", stat.events);
  printf("Double events: %d\n", stat.multi);
  printf("Errors:        %d\n", stat.errors);
  return 0;
}
