#include "rapid_json.h"

using namespace rapidjson;

namespace hidra2 {

RapidJson::RapidJson(const std::string& json): json_{json} {

}

bool RapidJson::LazyInitialize()const noexcept {
    if (initialized_)
        return true;

    if ( doc_.Parse(json_.c_str()).HasParseError()) {
        return false;
    }

    return true;

}

hidra2::Error CheckValueType(const std::string& name, ValueType type, const Value& val) {
    bool res = false;
    switch (type) {
    case ValueType::kString:
        res = val.IsString();
        break;
    case ValueType::kUint64:
        res = val.IsInt64();
        break;
    }
    if (!res) {
        return TextError("wrong type: " + name);
    }

    return nullptr;
}


hidra2::Error RapidJson::GetValue(const std::string& name, ValueType type, Value* val)const noexcept {
    if (!LazyInitialize()) {
        return TextError("cannot parse document");
    }

    auto iterator = doc_.FindMember(name.c_str());
    if (iterator == doc_.MemberEnd()) {
        return  TextError("cannot find: " + name);
    }

    *val =  iterator->value;
    return CheckValueType(name, type, *val);
}


Error RapidJson::GetUInt64(const std::string& name, uint64_t* val) const noexcept {
    Value json_val;
    if (Error err = GetValue(name, ValueType::kUint64, &json_val)) {
        return err;
    }
    *val = json_val.GetInt64();
    return nullptr;
}




Error RapidJson::GetString(const std::string& name, std::string* val) const noexcept {
    Value json_val;
    if (Error err = GetValue(name, ValueType::kString, &json_val)) {
        return err;
    }
    *val = json_val.GetString();
    return nullptr;
}


}