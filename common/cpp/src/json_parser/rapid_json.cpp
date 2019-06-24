#include "rapid_json.h"
#include "rapidjson/stringbuffer.h"
#include <rapidjson/writer.h>

using namespace rapidjson;

#include <iostream>

namespace asapo {

RapidJson::RapidJson(const std::string& json, const std::unique_ptr<IO>* io) : io__{io}, json_{json} {

}

Error RapidJson::LazyInitialize() const noexcept {
    if (embedded_error_) {
        return TextError(embedded_error_->Explain());
    }

    if (initialized_)
        return nullptr;

    auto str = json_;
    if (io__) {
        Error err;
        str = (*io__)->ReadFileToString(json_, &err);
        if (err != nullptr) {
            return err;
        }
    }

    ParseResult ok = doc_.Parse(str.c_str());
    if (!ok || !doc_.IsObject()) {
        return TextError("Cannot parse document");
    }

    object_ = doc_.GetObject();

    object_p_ = &object_;
    initialized_ = true;
    return nullptr;
}

asapo::Error RapidJson::CheckValueType(const std::string& name, ValueType type, const Value* val) const {
    bool res = false;
    switch (type) {
    case ValueType::kObject:
        res = val->IsObject();
        break;
    case ValueType::kString:
        res = val->IsString();
        break;
    case ValueType::kUint64:
        res = val->IsUint64();
        break;
    case ValueType::kInt64:
        res = val->IsInt64();
        break;
    case ValueType::kBool:
        res = val->IsBool();
        break;
    case ValueType::kArray:
        res = val->IsArray();
        break;
    }
    if (!res) {
        return TextError("wrong type: " + name + " in: " + json_);
    }

    return nullptr;
}

asapo::Error RapidJson::GetValuePointer(const std::string& name, ValueType type, Value** val) const noexcept {
    if (Error err = LazyInitialize()) {
        return err;
    }

    auto iterator = object_p_->FindMember(name.c_str());
    if (iterator == object_p_->MemberEnd()) {
        return TextError("cannot find: " + name);
    }

    *val = &iterator->value;
    return CheckValueType(name, type, *val);
}

Error RapidJson::GetInt64(const std::string& name, int64_t* val) const noexcept {
    Value* json_val;
    if (Error err = GetValuePointer(name, ValueType::kInt64, &json_val)) {
        return err;
    }
    *val = json_val->GetInt64();
    return nullptr;
}

Error RapidJson::GetUInt64(const std::string& name, uint64_t* val) const noexcept {
    int64_t val_int64;

    Error err = GetInt64(name, &val_int64);
    if (!initialized_) {
        return err;
    }
    if (err == nullptr) {
        *val = static_cast<uint64_t>(val_int64);
        return nullptr;
    }

    Value* json_val;
    if (Error err = GetValuePointer(name, ValueType::kUint64, &json_val)) {
        return err;
    }
    *val = json_val->GetUint64();
    return nullptr;
}

Error RapidJson::GetBool(const std::string& name, bool* val) const noexcept {
    Value* json_val;
    if (Error err = GetValuePointer(name, ValueType::kBool, &json_val)) {
        return err;
    }
    *val = json_val->GetBool();
    return nullptr;
}

Error RapidJson::GetString(const std::string& name, std::string* val) const noexcept {
    Value* json_val;
    if (Error err = GetValuePointer(name, ValueType::kString, &json_val)) {
        return err;
    }
    *val = json_val->GetString();
    return nullptr;
}

Error RapidJson::GetArrayUInt64(const std::string& name, std::vector<uint64_t>* val) const noexcept {
    Value* json_val;
    if (Error err = GetValuePointer(name, ValueType::kArray, &json_val)) {
        return err;
    }

    val->clear();
    for (auto& v : json_val->GetArray()) {
        if (!v.IsUint64()) {
            return TextError("wrong type of array element: " + name);
        }
        val->push_back(v.GetUint64());
    }
    return nullptr;

}

Error RapidJson::GetArrayString(const std::string& name, std::vector<std::string>* val) const noexcept {
    Value* json_val;
    if (Error err = GetValuePointer(name, ValueType::kArray, &json_val)) {
        return err;
    }

    val->clear();
    for (auto& v : json_val->GetArray()) {
        if (!v.IsString()) {
            return TextError("wrong type of array element: " + name);
        }
        val->push_back(v.GetString());
    }
    return nullptr;

}

RapidJson::RapidJson(const RapidJson& parent, const std::string& subname) {
    auto err = parent.GetValuePointer(subname, ValueType::kObject, &object_p_);
    if (err) {
        embedded_error_ = std::move(err);
        return;
    }
    initialized_ = true;
}

Error RapidJson::GetRawString(std::string* val) const noexcept {
    if (Error err = LazyInitialize()) {
        return err;
    }

    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    object_p_->Accept(writer);
    val->assign(buffer.GetString());
    return nullptr;
}

Error RapidJson::GetArrayRawStrings(const std::string& name, std::vector<std::string>* val) const noexcept {
    if (Error err = LazyInitialize()) {
        return err;
    }

    Value* json_val;
    if (Error err = GetValuePointer(name, ValueType::kArray, &json_val)) {
        return err;
    }

    val->clear();
    for (auto& v : json_val->GetArray()) {
        if (!v.IsObject()) {
            return TextError("wrong type of array element: " + name);
        }
        StringBuffer buffer;
        Writer<StringBuffer> writer(buffer);
        v.Accept(writer);
        val->push_back(buffer.GetString());
    }

    return nullptr;

}

}