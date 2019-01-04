
#pragma once

#include <string>

// \todo get rid of this file; use binary reference data instead

std::string vmm3json = R"(
{
  "builder_type" : "VMM3",

  "time_config" :
  {
    "tac_slope" : 60,
    "bc_clock" : 20,
    "trigger_resolution" : 3.125,
    "target_resolution" : 0.5,
    "acquisition_window" : 4000
  },

  "srs_mappings" :
  [
    {"fecID":1, "vmmID":0, "planeID":0, "strip_offset":0},
    {"fecID":1, "vmmID":1, "planeID":0, "strip_offset":64},
    {"fecID":1, "vmmID":6, "planeID":0, "strip_offset":128},
    {"fecID":1, "vmmID":7, "planeID":0, "strip_offset":192},
    {"fecID":1, "vmmID":10, "planeID":1, "strip_offset":0},
    {"fecID":1, "vmmID":11, "planeID":1, "strip_offset":64},
    {"fecID":1, "vmmID":14, "planeID":1, "strip_offset":128},
    {"fecID":1, "vmmID":15, "planeID":1, "strip_offset":192}
  ],

  "clusterer x" :
  {
    "hit_adc_threshold" : 0,
    "max_strip_gap" : 2,
    "max_time_gap" : 200,
    "min_cluster_size" : 3
  },

  "clusterer y" :
  {
    "hit_adc_threshold" : 0,
    "max_strip_gap" : 2,
    "max_time_gap" : 200,
    "min_cluster_size" : 3
  },

  "matcher_max_delta_time" : 200,

  "analyze_weighted" : true,
  "analyze_max_timebins" : 3,
  "analyze_max_timedif" : 7,

  "filters" :
  {
    "enforce_lower_uncertainty_limit" : false,
    "lower_uncertainty_limit" : 6,
    "enforce_minimum_hits" : false,
    "minimum_hits" : 6
  },

  "hit_histograms" : true,
  "cluster_adc_downshift" : 6,
  "send_tracks" : true,
  "track_sample_minhits" : 6,

  "geometry_x" : 256,
  "geometry_y" : 256,

  "dump_csv" : true,
  "dump_h5" : true,
  "dump_directory" : ""
}
)";

// Truncated data from packet 44 in high_datarates.pcapng
std::vector<uint8_t> pkt44 = {
            0x00, 0x32, 0x8d, 0xde, 0x56, 0x4d, /* ...2..VM */
0x32, 0x00, 0x78, 0xc6, 0x00, 0x00, 0x00, 0x00, /* 2.x..... */
0x00, 0x00, 0x28, 0x4e, 0xee, 0xbd, 0xc6, 0xac, /* ..(N.... */
0x28, 0x0a, 0x4f, 0x03, 0xc5, 0x5d, 0x2b, 0x85, /* (.O..]+. */
0x8a, 0x0b, 0xe5, 0x4b, 0x29, 0xd7, 0x8a, 0x6f, /* ...K)..o */
0xf5, 0x76, 0x2b, 0xd1, 0x6e, 0xb1, 0xcf, 0x51, /* .v+.n..Q */
0x28, 0x4d, 0xbe, 0xbd, 0xc7, 0x90, 0x28, 0x19, /* (M....(. */
0xde, 0x8f, 0xc6, 0x70, 0x2b, 0x87, 0x6a, 0x0b, /* ...p+.j. */
0xe6, 0x3f, 0x29, 0xce, 0x4a, 0x6f, 0xf6, 0x8f, /* .?).Jo.. */
0x2b, 0xcc, 0x1e, 0xb0, 0xd0, 0xa9, 0x28, 0x4b, /* +.....(K */
0x0e, 0xbc, 0xc9, 0x83, 0x28, 0x1e, 0x1e, 0x8e, /* ....(... */
0xc7, 0xcf, 0x2b, 0x8e, 0x5a, 0x09, 0xe7, 0x8b, /* ..+.Z... */
0x2b, 0xcf, 0x5e, 0x90, 0xd1, 0x59, 0x29, 0x0a, /* +.^..Y). */
0xba, 0x47, 0xe9, 0x81, 0x29, 0xc6, 0x7a, 0x46, /* .G..).zF */
0xed, 0x61, 0x28, 0x51, 0x1e, 0xbc, 0xca, 0x80, /* .a(Q.... */
0x28, 0x0a, 0x3d, 0x12, 0xc8, 0x70, 0x2b, 0x8a, /* (.=..p+. */
0xba, 0x09, 0xe8, 0x7d, 0x2b, 0xcb, 0x2e, 0xb0, /* ...}+... */
0xd2, 0x98, 0x29, 0x08, 0xba, 0x42, 0xea, 0xc8, /* ..)..B.. */
0x29, 0xc8, 0x1a, 0x46, 0xee, 0x7f, 0x28, 0x51, /* )..F..(Q */
0x3e, 0xbc, 0xcb, 0xa1, 0x28, 0x0c, 0x0f, 0xf2, /* >...(... */
0xcf, 0xc3, 0x2b, 0x86, 0xaa, 0x6f, 0xf6, 0x66, /* ..+..o.f */
0x2b, 0xc9, 0x3e, 0xb0, 0xd3, 0x8c, 0x29, 0x0b, /* +.>...). */
0x0a, 0x46, 0xeb, 0xc1, 0x29, 0xce, 0x6a, 0x6e, /* .F..).jn */
0xf0, 0x5d, 0x28, 0x59, 0x6e, 0xb4, 0xcc, 0x68, /* .](Yn..h */
0x28, 0x12, 0x0a, 0x6e, 0xd0, 0x7d, 0x2b, 0x94, /* (..n.}+. */
0xea, 0x6f, 0xf7, 0x71, 0x2b, 0xc9, 0x6e, 0xb0, /* .o.q+.n. */
0xd4, 0x55, 0x29, 0x09, 0xba, 0x46, 0xec, 0x60, /* .U)..F.` */
0x29, 0xda, 0xba, 0x6e, 0xf1, 0x78, 0x28, 0x4d, /* )..n.x(M */
0xae, 0x6a, 0xd4, 0xb0, 0x28, 0x0b, 0x5f, 0xf2, /* .j..(._. */
0xd1, 0xab, 0x2b, 0xad, 0x0a, 0x6f, 0xf8, 0x51, /* ..+..o.Q */
0x2b, 0xc9, 0xfe, 0xb0, 0xd5, 0x76, 0x29, 0x0d, /* +....v). */
0x0a, 0x46, 0xee, 0x6b, 0x29, 0xd9, 0x8a, 0x6f, /* .F.k)..o */
0xf2, 0x58, 0x28, 0x52, 0xde, 0x6a, 0xd5, 0x88, /* .X(R.j.. */
0x28, 0x0b, 0xcf, 0xf2, 0xd2, 0x91, 0x2b, 0xa5, /* (.....+. */
0x7a, 0x6f, 0xf9, 0x5b, 0x2b, 0xcb, 0xde, 0xb0, /* zo.[+... */
0xd6, 0x60, 0x29, 0x12, 0x0a, 0x42, 0xef, 0xa1, /* .`)..B.. */
0x29, 0xc7, 0xca, 0x45, 0xf4, 0x57, 0x28, 0x4c, /* )..E.W(L */
0x4e, 0x6a, 0xd6, 0x60, 0x28, 0x0b, 0xdf, 0xf2, /* Nj.`(... */
0xd3, 0x68, 0x2b, 0x92, 0x6a, 0x6f, 0xfa, 0x70, /* .h+.jo.p */
0x2b, 0xca, 0x7e, 0xb0, 0xd7, 0x68, 0x29, 0x19, /* +.~..h). */
0x0a, 0x42, 0xf0, 0x83, 0x29, 0xc6, 0xba, 0x47, /* .B..)..G */
0xf5, 0x75, 0x28, 0x4a, 0x9e, 0x6e, 0xd7, 0xb6  /* .u(J.n.. */
};