var aisdecoder = require('../');
var AisDecoder = aisdecoder.AisDecoder;
var should = require('should');

isea = {};

describe('aisdecoder', function() {

  var decoder = new AisDecoder();

  function decoderWorks(input, output) {
    var inputarray = [].concat(input); // support both string and array of strings
    for (var i=0;i<inputarray.length;i++) {
      var res = decoder.decode(inputarray[i]);
    }
    if (!output) {
      should.not.exist(res);
    }
    else {
      res.should.eql(output);
    }
  }

  function decoderFails(input, errstr) {
    try {
      decoder.decode(input);
    }
    catch (err) {
      err.toString().should.match(new RegExp(errstr));
    }
  }

  describe('constructor', function() {
    it('should return an aisdecoder instance', function() {
      var s = new aisdecoder.AisDecoder();
      s.should.be.an.instanceOf(aisdecoder.AisDecoder);
    });
  });
  describe('decoding type 1 message', function() {
    it('works', function() {
      decoderWorks('!AIVDM,1,1,,B,177KQJ5000G?tO`K>RA1wUbN0TKH,0*5C',
                  {
                   type: 'PositionReportClassA',
                   repeat: 0,
                   mmsi: 477553000,
                   status: 5,
                   turn: 0.0,
                   speed: 0.0,
                   accuracy: false,
                   lon: -122.34583333333333,
                   lat: 47.58283333333333,
                   course: 51.0,
                   heading: 181.0,
                   second: 15, 
                   maneuver: 0,
                   raim: false,
                   radio: 149208
                  });
    });
  });
  describe('decoding type 4 message', function() {
    it('works', function() {
      decoderWorks('!AIVDM,1,1,,A,402M43Aug9g@o0frsPTBHl7000S:,0*23',
                  {
                    type: 'BaseStationReport',
/* FIXME: Support timestamp
                    minute: 16,
                    second: 55,
                    hour: 15,
                    day: 19,
                    month: 12,
                    year: 2011, */
                    radio: 2250,
                    raim: false,
                    epfd: 7,
                    repeat: 0,
                    accuracy: false,
                    lon: 10.25,
                    lat: 63.416666666666664,
                    mmsi: 2573325
                  });
    });
  });
  describe('decoding type 5 message', function() {
    it('works', function() {
      decoderWorks(['!AIVDM,2,1,1,A,55?MbV02;H;s<HtKR20EHE:0@T4@Dn2222222216L961O5Gf0NSQEp6ClRp8,0*1C',
                   '!AIVDM,2,2,1,A,88888888880,2*25'],
                  {
                    type: 'StaticAndVoyageRelatedData',
                    to_stern: 70,
                    to_port: 1,
                    imo: 9134270,
                    to_starboard: 31,
                    callsign: "3FOF8",
                    to_bow: 225,
                    epfd: 1,
                    dte: 0,
                    repeat: 0,
                    draught: 12.2,
/* FIXME: timestamp                    day: 15,
                    month: 5,
                    hour: 14,
                    minute: 0, */
                    ais_version: 0,
                    shipname: "EVER DIADEM",
                    shiptype: 70,
                    destination: "NEW YORK",
                    mmsi: 351759000
                  });
    });
  });
  describe('decoding type 18 message', function() {
    it('works', function() {
      decoderWorks('!AIVDM,1,1,,B,B69>7mh0?J<:>05B0`0e;wq2PHI8,0*3D',
                  {
                    type: 'ClassBCSPositionReport',
                    course: 72.2,
                    speed: 6.1,
                    dsc: false,
                    radio: 99912,
                    raim: false,
                    msg22: false,
                    cs: false,
                    band: true,
                    repeat: false,
                    assigned: false,
                    accuracy: true,
                    lon: 122.47338666666667,
                    regional: 0,
                    lat: 36.91968,
                    display: true,
                    second: 50,
                    mmsi: 412321751
                  });
    });
  });
  describe('decoding type 19 message', function() {
    it('works', function() {
      decoderWorks('!AIVDM,1,1,,B,C69>7mh0>r<9vD5Auh;PcwVPHc0TNL?0jc1WQkR00000?1@5222P,0*52',
                  {
                    type: 'ExtendedClassBCSPositionReport',
                    to_port: 4,
                    to_stern: 10,
                    course: 359.4,
                    to_starboard: 4,
                    speed: 5.9,
                    raim: false,
                    epfd: 1,
                    to_bow: 20,
                    dte: 1,
                    repeat: false,
                    assigned: false,
                    accuracy: true,
                    lon: 122.47004,
                    regional: 0,
                    lat: 36.91477666666667,
                    second: 13,
                    shipname: "LU RONG YU 3091",
                    shiptype: 30,
                    mmsi: 412321751
                  });
    });
  });
  describe('decoding type 27 message',
 function() {
    it('works', function() {
      decoderWorks('!AIVDM,1,1,,B,KC5E2b@U19PFdLbMuc5=ROv62<7m,0*16',
                  {
                    gnss: false,
                    type: 'LongRangeBroadcastMessage',
                    course: 167.0,
                    speed: 57.0,
                    raim: false,
                    status: 2,
                    accuracy: false,
                    repeat: 1,
                    lon: 137.02333333333334,
                    lat: 4.84,
                    mmsi: 206914217
                  });
    });
  });
  describe('decoding type 23 message',
 function() {
    it('is unsupported', function() {
      decoderFails('!AIVDM,1,1,,A,G02bBUP<3HluhGja`UV00000900,2*54',
                   'Unsupported message type 23');
    });
  });
  describe('decoding type 24 message',
 function() {
    it('is not implemented', function() {
      decoderFails('!AIVDM,1,1,,B,91b55wPVAOOTnPLQ?OFrcPP206I`,0*32',
                   'Not implemented: Static Data Report');
    });
  });
});
