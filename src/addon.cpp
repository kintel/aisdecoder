#include <node.h>
#include <v8.h>


#include "aisdecoder.h"

using namespace v8;

static const char *ais_typestring(unsigned int type)
{
  switch (type) {
  case 1: case 2: case 3: return "PositionReportClassA";
  case 4:  return "BaseStationReport";
  case 11: return "UTCAndDateResponse";
  case 5:  return "StaticAndVoyageRelatedData";
  case 18: return "ClassBCSPositionReport";
  case 19: return "ExtendedClassBCSPositionReport";
  case 27: return "LongRangeBroadcastMessage";
  case 6: // Ignore Binary Addressed Message
  case 8:  // Ignore Binary Broadcast Message
  case 10: // Ignore UTC/Date Inquiry
  case 12: // Ignore Addressed Safety-Related Message
  case 16: // Ignore Assignment Mode Command
  case 17: // Ignore differential GPS messages (DGNSS Binary Broadcast Message)
  case 20: // Ignore Data Link Management messages
  case 21: // Ignore Aid-to-Navigation Report. FIXME: ISEA-222
  case 22: // Ignore Channel Management
  case 25: // Ignore Single Slot Binary Message
  case 24: // Ignore unimplemented S&R aircraft message
    return "UnknownMessageType";
  case 9:
    return "Not implemented: Static Data Report";
  default:
    return "Unsupported message type"; // FIXME: " + msgType + ": " + payload)
  }
}

Handle<Object> convertToJS(ais_t *ais)
{
  // Create AIS object
  Handle<Object> aisobj = Object::New();
  aisobj->Set(String::New("type"), String::New(ais_typestring(ais->type)));
  aisobj->Set(String::New("mmsi"), Number::New(ais->mmsi));
  aisobj->Set(String::New("repeat"), Number::New(ais->repeat));
  switch (ais->type) {
  case 1:			/* Position Report */
  case 2:
  case 3:
    aisobj->Set(String::New("status"), Number::New(ais->type1.status));
    aisobj->Set(String::New("lon"), Number::New(ais->type1.lon / AIS_LATLON_DIV));
    aisobj->Set(String::New("lat"), Number::New(ais->type1.lat / AIS_LATLON_DIV));
    aisobj->Set(String::New("course"), Number::New(ais->type1.course / 10.0));
    aisobj->Set(String::New("heading"), Number::New(ais->type1.heading));
    aisobj->Set(String::New("accuracy"), Boolean::New(ais->type1.accuracy));
    aisobj->Set(String::New("second"), Number::New(ais->type1.second));
    aisobj->Set(String::New("maneuver"), Number::New(ais->type1.maneuver));
    aisobj->Set(String::New("raim"), Boolean::New(ais->type1.raim));
    aisobj->Set(String::New("radio"), Number::New(ais->type1.radio));

    /*
      \"status_text\":\"%s\","
      nav_legends[ais->type1.status],
    */

    /*
     * Express speed as nan if not available,
     * "fast" for fast movers.
     */
    {
      Handle<Value> speedval;
      switch (ais->type1.speed) {
      case AIS_SPEED_NOT_AVAILABLE:
        speedval = String::New("nan");
        break;
      case AIS_SPEED_FAST_MOVER:
        speedval = String::New("fast");
        break;
      default:
        speedval = Number::New(ais->type1.speed / 10.0);
        break;
      }
      aisobj->Set(String::New("speed"), speedval);
    }

    /*
     * Express turn as nan if not available,
     * "fastleft"/"fastright" for fast turns.
     */
    {
      Handle<Value> turnval;
      switch (ais->type1.turn) {
      case -128:
        turnval = String::New("nan");
        break;
      case -127:
        turnval = String::New("fastleft");
        break;
      case 127:
        turnval = String::New("fastright");
        break;
      default:
        double rot1 = ais->type1.turn / 4.733;
        turnval = Number::New(rot1 * rot1);
      }
      aisobj->Set(String::New("turn"), turnval);
    }

    break;
  case 4:			/* Base Station Report */
  case 11:			/* UTC/Date Response */
    /* some fields have been merged to an ISO8601 date */
    aisobj->Set(String::New("lon"), Number::New(ais->type4.lon / AIS_LATLON_DIV));
    aisobj->Set(String::New("lat"), Number::New(ais->type4.lat / AIS_LATLON_DIV));
    /*
      aisobj->Set(String::New("timestamp"), );
      ais->type4.year,
      ais->type4.month,
      ais->type4.day,
      ais->type4.hour,
      ais->type4.minute,
      ais->type4.second,
    */
    aisobj->Set(String::New("accuracy"), Boolean::New(ais->type4.accuracy));
    aisobj->Set(String::New("raim"), Boolean::New(ais->type4.raim));
    aisobj->Set(String::New("radio"), Number::New(ais->type4.radio));
    aisobj->Set(String::New("epfd"), Number::New(ais->type4.epfd));
    /*
      \"epfd_text\":\"%s\","
      EPFD_DISPLAY(ais->type4.epfd),
    */
    break;
  case 5:			/* Ship static and voyage related data */
    /* some fields have been merged to an ISO8601 partial date */
    aisobj->Set(String::New("imo"), Number::New(ais->type5.imo));
    aisobj->Set(String::New("callsign"), String::New(ais->type5.callsign));
    aisobj->Set(String::New("shipname"), String::New(ais->type5.shipname));
    aisobj->Set(String::New("shiptype"), Number::New(ais->type5.shiptype));
    aisobj->Set(String::New("destination"), String::New(ais->type5.destination));

    aisobj->Set(String::New("ais_version"), Number::New(ais->type5.ais_version));
    aisobj->Set(String::New("to_bow"), Number::New(ais->type5.to_bow));
    aisobj->Set(String::New("to_stern"), Number::New(ais->type5.to_stern));
    aisobj->Set(String::New("to_port"), Number::New(ais->type5.to_port));
    aisobj->Set(String::New("to_starboard"), Number::New(ais->type5.to_starboard));
    aisobj->Set(String::New("epfd"), Number::New(ais->type5.epfd));
    aisobj->Set(String::New("draught"), Number::New(ais->type5.draught / 10.0));
    aisobj->Set(String::New("dte"), Number::New(ais->type5.dte));

    /*
      \"shiptype_text\":\"%s\","
\"epfd_text\":\"%s\","
      "\"eta\":\"%02u-%02uT%02u:%02uZ\","

      SHIPTYPE_DISPLAY(ais->type5.shiptype),
      EPFD_DISPLAY(ais->type5.epfd),
      ais->type5.month,
      ais->type5.day,
      ais->type5.hour, ais->type5.minute,
    */
    break;
    case 18:
    aisobj->Set(String::New("lon"), Number::New(ais->type18.lon / AIS_LATLON_DIV));
    aisobj->Set(String::New("lat"), Number::New(ais->type18.lat / AIS_LATLON_DIV));
    aisobj->Set(String::New("course"), Number::New(ais->type18.course / 10.0));
    if (ais->type18.heading != 511) {
      aisobj->Set(String::New("heading"), Number::New(ais->type18.heading));
    }
    aisobj->Set(String::New("speed"), Number::New(ais->type18.speed / 10.0));
    aisobj->Set(String::New("accuracy"), Boolean::New(ais->type18.accuracy));
    //    aisobj->Set(String::New("reserved"), Number::New(ais->type18.reserved));
    aisobj->Set(String::New("regional"), Number::New(ais->type18.regional));
    aisobj->Set(String::New("cs"), Boolean::New(ais->type18.cs));
    aisobj->Set(String::New("display"), Boolean::New(ais->type18.display));
    aisobj->Set(String::New("dsc"), Boolean::New(ais->type18.dsc));
    aisobj->Set(String::New("band"), Boolean::New(ais->type18.band));
    aisobj->Set(String::New("msg22"), Boolean::New(ais->type18.msg22));
    aisobj->Set(String::New("raim"), Boolean::New(ais->type18.raim));
    aisobj->Set(String::New("radio"), Number::New(ais->type18.radio));
    aisobj->Set(String::New("assigned"), Number::New(ais->type18.assigned));
    aisobj->Set(String::New("second"), Number::New(ais->type18.second));
    break;
    case 19:
    aisobj->Set(String::New("lon"), Number::New(ais->type19.lon / AIS_LATLON_DIV));
    aisobj->Set(String::New("lat"), Number::New(ais->type19.lat / AIS_LATLON_DIV));
    aisobj->Set(String::New("course"), Number::New(ais->type19.course / 10.0));
    if (ais->type19.heading != 511) {
      aisobj->Set(String::New("heading"), Number::New(ais->type19.heading));
    }
    aisobj->Set(String::New("speed"), Number::New(ais->type19.speed / 10.0));
    aisobj->Set(String::New("accuracy"), Boolean::New(ais->type19.accuracy));
    //    aisobj->Set(String::New("reserved"), Number::New(ais->type19.reserved));
    aisobj->Set(String::New("regional"), Number::New(ais->type19.regional));
    aisobj->Set(String::New("second"), Number::New(ais->type19.second));
    aisobj->Set(String::New("shipname"), String::New(ais->type19.shipname));
    aisobj->Set(String::New("shiptype"), Number::New(ais->type19.shiptype));
    aisobj->Set(String::New("to_bow"), Number::New(ais->type19.to_bow));
    aisobj->Set(String::New("to_stern"), Number::New(ais->type19.to_stern));
    aisobj->Set(String::New("to_port"), Number::New(ais->type19.to_port));
    aisobj->Set(String::New("to_starboard"), Number::New(ais->type19.to_starboard));
    aisobj->Set(String::New("epfd"), Number::New(ais->type19.epfd));
    aisobj->Set(String::New("raim"), Boolean::New(ais->type19.raim));
    aisobj->Set(String::New("dte"), Number::New(ais->type19.dte));
    aisobj->Set(String::New("assigned"), Boolean::New(ais->type19.assigned));

    /*\"shiptype_text\":\"%s\","
\"epfd_text\":\"%s\","
			   SHIPTYPE_DISPLAY(ais->type19.shiptype),
			   EPFD_DISPLAY(ais->type19.epfd),
    */
    break;
  case 27:
    aisobj->Set(String::New("lon"), Number::New(ais->type27.lon / AIS_LONGRANGE_LATLON_DIV));
    aisobj->Set(String::New("lat"), Number::New(ais->type27.lat / AIS_LONGRANGE_LATLON_DIV));
    aisobj->Set(String::New("course"), Number::New(ais->type27.course));
    aisobj->Set(String::New("speed"), Number::New(ais->type27.speed));
    aisobj->Set(String::New("accuracy"), Boolean::New(ais->type27.accuracy));
    aisobj->Set(String::New("raim"), Boolean::New(ais->type27.raim));
    aisobj->Set(String::New("gnss"), Boolean::New(ais->type27.gnss));
    aisobj->Set(String::New("status"), Number::New(ais->type27.status));
    /*
			   "\"status\":\"%s\","
			   nav_legends[ais->type27.status],
    */
    break;
  default:
    break;
  }

  return aisobj;
}

class AisDecoder : public node::ObjectWrap
{
public:
  static void Init(Handle<Object> exports) { 
    // Prepare constructor template
    Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
    tpl->SetClassName(String::NewSymbol("AisDecoder"));
    tpl->InstanceTemplate()->SetInternalFieldCount(1);
    // Prototype
    tpl->PrototypeTemplate()->Set(String::NewSymbol("decode"),
                                  FunctionTemplate::New(decode)->GetFunction());

    Persistent<Function> constructor = Persistent<Function>::New(tpl->GetFunction());

    // FIXME: Return the constructor directly to exports instead
    exports->Set(String::NewSymbol("AisDecoder"), constructor);   
  }
private:
  ais_handle_t *ais_handle;

  AisDecoder() {
    this->ais_handle = ais_create_handle();
  }
  ~AisDecoder() {
    ais_destroy_handle(this->ais_handle);
  }

  static Handle<Value> New(const Arguments& args) {
    HandleScope scope;
    
    AisDecoder *decoder = new AisDecoder();
    decoder->Wrap(args.This());
    
    return args.This();
  }

  static Handle<Value> decode(const Arguments& args) {
    HandleScope scope;
    
    AisDecoder *thisp = ObjectWrap::Unwrap<AisDecoder>(args.This());

    String::AsciiValue ascii(args[0]->ToString());
    ais_t ais;
    bool ret = ais_decode(thisp->ais_handle, 
                          *ascii, ascii.length(),
                          &ais,
                          false, LOG_ERROR);

    Handle<Value> aisobj;
    if (ret) {
      aisobj = convertToJS(&ais);
    }
    else {
      aisobj = Undefined();
    }

    return scope.Close(aisobj);
  }
};

void InitAll(Handle<Object> exports) {
  AisDecoder::Init(exports);
}

NODE_MODULE(aisdecoder, InitAll)
