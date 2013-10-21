aisdecoder
==========
[![Build Status](https://travis-ci.org/kintel/aisdecoder.png)](https://travis-ci.org/kintel/aisdecoder)

AIS decoder module for node.js


This project is a node.js wrapper for the AIS decoder from the [gpsd project](http://www.catb.org/gpsd).

# Install

```npm install aisdecoder```

# Usage

````javascript
var AisDecoder = require ('aisdecoder').AisDecoder;
var decoder = new AisDecoder;
var aisobject = decoder.decode('!AIVDM,1,1,,B,177KQJ5000G?tO`K>RA1wUbN0TKH,0*5C');
````


