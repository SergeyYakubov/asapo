#include "rapid_json.h"
#include "rapid_json.h"

using namespace rapidjson;

#include "io/io_factory.h"

namespace hidra2 {

RapidJson::RapidJson(const std::string& json, bool read_from_file): io__{GenerateDefaultIO()}, json_{json},
    read_from_file_{read_from_file} {

}

Error RapidJson::LazyInitialize()const noexcept {
    if (embedded_error_) {
        return TextError(embedded_error_->Explain());
    }

    if (initialized_)
        return nullptr;

    auto str = json_;
    if (read_from_file_) {
        Error err;
        str = io__->ReadFileToString(json_, &err);
        if (err != nullptr) {
            return err;
        }
    }

    if ( doc_.Parse(str.c_str()).HasParseError()) {
        return TextError("Cannot parse document");
    }

    object_ = doc_.GetObject();

    return nullptr;
}

hidra2::Error CheckValueType(const std::string& name, ValueType type, const Value& val) {
    bool res = false;
    switch (type) {
    case ValueType::kObject:
        res = val.IsObject();
        break;
    case ValueType::kString:
        res = val.IsString();
        break;
    case ValueType::kUint64:
        res = val.IsInt64();
        break;
    case ValueType::kArray:
        res = val.IsArray();
        break;
    }
    if (!res) {
        return TextError("wrong type: " + name);
    }

    return nullptr;
}


hidra2::Error RapidJson::GetValue(const std::string& name, ValueType type, Value* val)const noexcept {
    if (Error err = LazyInitialize()) {
        return err;
    }

    auto iterator = object_.FindMember(name.c_str());
    if (iterator == object_.MemberEnd()) {
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


Error RapidJson::GetArrayUInt64(const std::string& name, std::vector<uint64_t>* val) const noexcept {
    Value json_val;
    if (Error err = GetValue(name, ValueType::kArray, &json_val)) {
        return err;
    }

    for (auto& v : json_val.GetArray()) {
        if (!v.IsInt64()) {
            return TextError("wrong type of array element: " + name);
        }
        val->push_back(v.GetInt());
    }
    return nullptr;

}

Error RapidJson::GetArrayString(const std::string& name, std::vector<std::string>* val) const noexcept {
    Value json_val;
    if (Error err = GetValue(name, ValueType::kArray, &json_val)) {
        return err;
    }

    for (auto& v : json_val.GetArray()) {
        if (!v.IsString()) {
            return TextError("wrong type of array element: " + name);
        }
        val->push_back(v.GetString());
    }
    return nullptr;

}


RapidJson::RapidJson(const RapidJson& parent, const std::string& subname) {
    auto err = parent.GetValue(subname, ValueType::kObject, &object_);
    if (err) {
        embedded_error_ = std::move(err);
        return;
    }
    initialized_ = true;
}


}