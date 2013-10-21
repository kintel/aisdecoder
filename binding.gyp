{
  "targets": [  
    {                           
      "target_name": "aisdecoder",
      "sources": [ 
        "src/addon.cpp", 
        "src/aisdecoder.cpp", 
        "src/driver_ais.c",
        "src/bits.c", 
        "src/hex.c", 
        "src/aivdm_decode.c",
        "src/gpsd.c",
        "src/strl.c",
      ],
      "defines": [ "<@(strldefines)" ]
    }
  ],
  "variables": {
      "conditions": [
         ['OS=="linux" or OS=="win"', {"strldefines": [ ]}]
      ],
      "strldefines%": [ "HAVE_STRLCAT", "HAVE_STRLCPY" ]
  }
}
