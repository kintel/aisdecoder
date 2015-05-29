#include <node.h>
#include <node_object_wrap.h>
#include <v8.h>


#include "aisdecoder.h"

using namespace v8;
using namespace node;

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
  Isolate* isolate = Isolate::GetCurrent();
  Handle<Object> aisobj = Object::New(isolate);
  aisobj->Set(String::NewFromUtf8(isolate, "type"), String::NewFromUtf8(isolate, ais_typestring(ais->type)));
  aisobj->Set(String::NewFromUtf8(isolate, "mmsi"), Number::New(isolate, ais->mmsi));
  aisobj->Set(String::NewFromUtf8(isolate, "repeat"), Number::New(isolate, ais->repeat));
  switch (ais->type) {
  case 1:			/* Position Report */
  case 2:
  case 3:
    aisobj->Set(String::NewFromUtf8(isolate, "status"), Number::New(isolate, ais->type1.status));
    aisobj->Set(String::NewFromUtf8(isolate, "lon"), Number::New(isolate, ais->type1.lon / AIS_LATLON_DIV));
    aisobj->Set(String::NewFromUtf8(isolate, "lat"), Number::New(isolate, ais->type1.lat / AIS_LATLON_DIV));
    aisobj->Set(String::NewFromUtf8(isolate, "course"), Number::New(isolate, ais->type1.course / 10.0));
    aisobj->Set(String::NewFromUtf8(isolate, "heading"), Number::New(isolate, ais->type1.heading));
    aisobj->Set(String::NewFromUtf8(isolate, "accuracy"), Boolean::New(isolate, ais->type1.accuracy));
    aisobj->Set(String::NewFromUtf8(isolate, "second"), Number::New(isolate, ais->type1.second));
    aisobj->Set(String::NewFromUtf8(isolate, "maneuver"), Number::New(isolate, ais->type1.maneuver));
    aisobj->Set(String::NewFromUtf8(isolate, "raim"), Boolean::New(isolate, ais->type1.raim));
    aisobj->Set(String::NewFromUtf8(isolate, "radio"), Number::New(isolate, ais->type1.radio));

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
        speedval = String::NewFromUtf8(isolate, "nan");
        break;
      case AIS_SPEED_FAST_MOVER:
        speedval = String::NewFromUtf8(isolate, "fast");
        break;
      default:
        speedval = Number::New(isolate, ais->type1.speed / 10.0);
        break;
      }
      aisobj->Set(String::NewFromUtf8(isolate, "speed"), speedval);
    }

    /*
     * Express turn as nan if not available,
     * "fastleft"/"fastright" for fast turns.
     */
    {
      Handle<Value> turnval;
      switch (ais->type1.turn) {
      case -128:
        turnval = String::NewFromUtf8(isolate, "nan");
        break;
      case -127:
        turnval = String::NewFromUtf8(isolate, "fastleft");
        break;
      case 127:
        turnval = String::NewFromUtf8(isolate, "fastright");
        break;
      default:
        double rot1 = ais->type1.turn / 4.733;
        turnval = Number::New(isolate, rot1 * rot1);
      }
      aisobj->Set(String::NewFromUtf8(isolate, "turn"), turnval);
    }

    break;
  case 4:			/* Base Station Report */
  case 11:			/* UTC/Date Response */
    /* some fields have been merged to an ISO8601 date */
    aisobj->Set(String::NewFromUtf8(isolate, "lon"), Number::New(isolate, ais->type4.lon / AIS_LATLON_DIV));
    aisobj->Set(String::NewFromUtf8(isolate, "lat"), Number::New(isolate, ais->type4.lat / AIS_LATLON_DIV));
    /*
      aisobj->Set(String::NewFromUtf8(isolate, "timestamp"), );
      ais->type4.year,
      ais->type4.month,
      ais->type4.day,
      ais->type4.hour,
      ais->type4.minute,
      ais->type4.second,
    */
    aisobj->Set(String::NewFromUtf8(isolate, "accuracy"), Boolean::New(isolate, ais->type4.accuracy));
    aisobj->Set(String::NewFromUtf8(isolate, "raim"), Boolean::New(isolate, ais->type4.raim));
    aisobj->Set(String::NewFromUtf8(isolate, "radio"), Number::New(isolate, ais->type4.radio));
    aisobj->Set(String::NewFromUtf8(isolate, "epfd"), Number::New(isolate, ais->type4.epfd));
    /*
      \"epfd_text\":\"%s\","
      EPFD_DISPLAY(ais->type4.epfd),
    *
    break;
  case 5:			// Ship static and voyage related data */
    /* some fields have been merged to an ISO8601 partial date */
    aisobj->Set(String::NewFromUtf8(isolate, "imo"), Number::New(isolate, ais->type5.imo));
    aisobj->Set(String::NewFromUtf8(isolate, "callsign"), String::NewFromUtf8(isolate, ais->type5.callsign));
    aisobj->Set(String::NewFromUtf8(isolate, "shipname"), String::NewFromUtf8(isolate, ais->type5.shipname));
    aisobj->Set(String::NewFromUtf8(isolate, "shiptype"), Number::New(isolate, ais->type5.shiptype));
    aisobj->Set(String::NewFromUtf8(isolate, "destination"), String::NewFromUtf8(isolate, ais->type5.destination));

    aisobj->Set(String::NewFromUtf8(isolate, "ais_version"), Number::New(isolate, ais->type5.ais_version));
    aisobj->Set(String::NewFromUtf8(isolate, "to_bow"), Number::New(isolate, ais->type5.to_bow));
    aisobj->Set(String::NewFromUtf8(isolate, "to_stern"), Number::New(isolate, ais->type5.to_stern));
    aisobj->Set(String::NewFromUtf8(isolate, "to_port"), Number::New(isolate, ais->type5.to_port));
    aisobj->Set(String::NewFromUtf8(isolate, "to_starboard"), Number::New(isolate, ais->type5.to_starboard));
    aisobj->Set(String::NewFromUtf8(isolate, "epfd"), Number::New(isolate, ais->type5.epfd));
    aisobj->Set(String::NewFromUtf8(isolate, "draught"), Number::New(isolate, ais->type5.draught / 10.0));
    aisobj->Set(String::NewFromUtf8(isolate, "dte"), Number::New(isolate, ais->type5.dte));

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
    aisobj->Set(String::NewFromUtf8(isolate, "lon"), Number::New(isolate, ais->type18.lon / AIS_LATLON_DIV));
    aisobj->Set(String::NewFromUtf8(isolate, "lat"), Number::New(isolate, ais->type18.lat / AIS_LATLON_DIV));
    aisobj->Set(String::NewFromUtf8(isolate, "course"), Number::New(isolate, ais->type18.course / 10.0));
    if (ais->type18.heading != 511) {
      aisobj->Set(String::NewFromUtf8(isolate, "heading"), Number::New(isolate, ais->type18.heading));
    }
    aisobj->Set(String::NewFromUtf8(isolate, "speed"), Number::New(isolate, ais->type18.speed / 10.0));
    aisobj->Set(String::NewFromUtf8(isolate, "accuracy"), Boolean::New(isolate, ais->type18.accuracy));
    //    aisobj->Set(String::NewFromUtf8(isolate, "reserved"), Number::New(isolate, ais->type18.reserved));
    aisobj->Set(String::NewFromUtf8(isolate, "regional"), Number::New(isolate, ais->type18.regional));
    aisobj->Set(String::NewFromUtf8(isolate, "cs"), Boolean::New(isolate, ais->type18.cs));
    aisobj->Set(String::NewFromUtf8(isolate, "display"), Boolean::New(isolate, ais->type18.display));
    aisobj->Set(String::NewFromUtf8(isolate, "dsc"), Boolean::New(isolate, ais->type18.dsc));
    aisobj->Set(String::NewFromUtf8(isolate, "band"), Boolean::New(isolate, ais->type18.band));
    aisobj->Set(String::NewFromUtf8(isolate, "msg22"), Boolean::New(isolate, ais->type18.msg22));
    aisobj->Set(String::NewFromUtf8(isolate, "raim"), Boolean::New(isolate, ais->type18.raim));
    aisobj->Set(String::NewFromUtf8(isolate, "radio"), Number::New(isolate, ais->type18.radio));
    aisobj->Set(String::NewFromUtf8(isolate, "assigned"), Number::New(isolate, ais->type18.assigned));
    aisobj->Set(String::NewFromUtf8(isolate, "second"), Number::New(isolate, ais->type18.second));
    break;
    case 19:
    aisobj->Set(String::NewFromUtf8(isolate, "lon"), Number::New(isolate, ais->type19.lon / AIS_LATLON_DIV));
    aisobj->Set(String::NewFromUtf8(isolate, "lat"), Number::New(isolate, ais->type19.lat / AIS_LATLON_DIV));
    aisobj->Set(String::NewFromUtf8(isolate, "course"), Number::New(isolate, ais->type19.course / 10.0));
    if (ais->type19.heading != 511) {
      aisobj->Set(String::NewFromUtf8(isolate, "heading"), Number::New(isolate, ais->type19.heading));
    }
    aisobj->Set(String::NewFromUtf8(isolate, "speed"), Number::New(isolate, ais->type19.speed / 10.0));
    aisobj->Set(String::NewFromUtf8(isolate, "accuracy"), Boolean::New(isolate, ais->type19.accuracy));
    //    aisobj->Set(String::NewFromUtf8(isolate, "reserved"), Number::New(isolate, ais->type19.reserved));
    aisobj->Set(String::NewFromUtf8(isolate, "regional"), Number::New(isolate, ais->type19.regional));
    aisobj->Set(String::NewFromUtf8(isolate, "second"), Number::New(isolate, ais->type19.second));
    aisobj->Set(String::NewFromUtf8(isolate, "shipname"), String::NewFromUtf8(isolate, ais->type19.shipname));
    aisobj->Set(String::NewFromUtf8(isolate, "shiptype"), Number::New(isolate, ais->type19.shiptype));
    aisobj->Set(String::NewFromUtf8(isolate, "to_bow"), Number::New(isolate, ais->type19.to_bow));
    aisobj->Set(String::NewFromUtf8(isolate, "to_stern"), Number::New(isolate, ais->type19.to_stern));
    aisobj->Set(String::NewFromUtf8(isolate, "to_port"), Number::New(isolate, ais->type19.to_port));
    aisobj->Set(String::NewFromUtf8(isolate, "to_starboard"), Number::New(isolate, ais->type19.to_starboard));
    aisobj->Set(String::NewFromUtf8(isolate, "epfd"), Number::New(isolate, ais->type19.epfd));
    aisobj->Set(String::NewFromUtf8(isolate, "raim"), Boolean::New(isolate, ais->type19.raim));
    aisobj->Set(String::NewFromUtf8(isolate, "dte"), Number::New(isolate, ais->type19.dte));
    aisobj->Set(String::NewFromUtf8(isolate, "assigned"), Boolean::New(isolate, ais->type19.assigned));

    /*\"shiptype_text\":\"%s\","
\"epfd_text\":\"%s\","
			   SHIPTYPE_DISPLAY(ais->type19.shiptype),
			   EPFD_DISPLAY(ais->type19.epfd),
    */
    break;
  case 27:
    aisobj->Set(String::NewFromUtf8(isolate, "lon"), Number::New(isolate, ais->type27.lon / AIS_LONGRANGE_LATLON_DIV));
    aisobj->Set(String::NewFromUtf8(isolate, "lat"), Number::New(isolate, ais->type27.lat / AIS_LONGRANGE_LATLON_DIV));
    aisobj->Set(String::NewFromUtf8(isolate, "course"), Number::New(isolate, ais->type27.course));
    aisobj->Set(String::NewFromUtf8(isolate, "speed"), Number::New(isolate, ais->type27.speed));
    aisobj->Set(String::NewFromUtf8(isolate, "accuracy"), Boolean::New(isolate, ais->type27.accuracy));
    aisobj->Set(String::NewFromUtf8(isolate, "raim"), Boolean::New(isolate, ais->type27.raim));
    aisobj->Set(String::NewFromUtf8(isolate, "gnss"), Boolean::New(isolate, ais->type27.gnss));
    aisobj->Set(String::NewFromUtf8(isolate, "status"), Number::New(isolate, ais->type27.status));
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
    Isolate* isolate = Isolate::GetCurrent();
    // Prepare constructor template
    Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
    tpl->SetClassName(String::NewFromUtf8(isolate, "AisDecoder"));
    tpl->InstanceTemplate()->SetInternalFieldCount(1);
    // Prototype
    NODE_SET_PROTOTYPE_METHOD(tpl, "decode", decode);
    exports->Set(String::NewFromUtf8(isolate, "AisDecoder"),
               tpl->GetFunction());
  }
private:
  ais_handle_t *ais_handle;

  AisDecoder() {
    this->ais_handle = ais_create_handle();
  }
  ~AisDecoder() {
    ais_destroy_handle(this->ais_handle);
  }


  static void New(const FunctionCallbackInfo<Value>& info) {
      AisDecoder *decoder = new AisDecoder();
      decoder->Wrap(info.This());
      info.GetReturnValue().Set(info.This());
  }

  static void decode(const FunctionCallbackInfo<Value>& info) {    
    AisDecoder *thisp = ObjectWrap::Unwrap<AisDecoder>(info.This());
    Local<String> op = info[0]->ToString();
    const int length = op->Utf8Length() + 1; 
    uint8_t* buffer = new uint8_t[length];
    op->WriteOneByte(buffer, 0, length);
    ais_t ais;

    bool ret = ais_decode(thisp->ais_handle, 
                          (const char *) buffer, length,
                          &ais,
                          false, LOG_ERROR);
    Handle<Value> aisobj;
    if (ret) {
      aisobj = convertToJS(&ais);
    }
    else {
      aisobj = Undefined(info.GetIsolate());
    }

    info.GetReturnValue().Set(aisobj);
  }
};

void InitAll(Handle<Object> exports) {
  AisDecoder::Init(exports);
}

NODE_MODULE(aisdecoder, InitAll)
