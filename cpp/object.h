#ifndef MONKEY_OBJECT_H_INCLUDED
#define MONKEY_OBJECT_H_INCLUDED

#include <string>
#include <memory>
#include <cassert>


#define ALL_OBJECT_TYPES \
    PROCESS(INTEGER) \
    PROCESS(BOOLEAN) \
    PROCESS(NULL) \
    PROCESS(RETURN_VALUE) \
    PROCESS(ERROR) \

#define PROCESS(a) a##_OBJ,
enum class ObjectType {
    ALL_OBJECT_TYPES
};
#undef PROCESS


#define PROCESS(a) #a,
inline const char *GetObjectTypeName(ObjectType type)
{
    static const char* names[] = { ALL_OBJECT_TYPES nullptr };
    return names[static_cast<size_t>(type)];
}
#undef PROCESS

class MObject {
public:
    explicit MObject(ObjectType type) : type_(type) {}
    virtual ~MObject() {}

    ObjectType Type() const { return type_; }
    virtual std::string Inspect() const = 0;
private:
    ObjectType type_;
};//~ MObject

template<typename T, ObjectType O>
class MValue : public MObject {
public:
    MValue(T value) : MObject(O), value_(value) {}
    std::string Inspect() const override { return std::to_string(value_); }
    T Value() const { return value_; }
private:
    T value_;
};//~ MValue

using MInteger = MValue<int64_t, ObjectType::INTEGER_OBJ>;
using MBoolean = MValue<bool   , ObjectType::BOOLEAN_OBJ>;

class MNull : public MObject {
public:
    MNull() : MObject(ObjectType::NULL_OBJ) {}
    std::string Inspect() const override { return "null"; }
};//~ MNull

class MReturn : public MObject {
public:
    explicit MReturn(const std::shared_ptr<MObject>& val)
    : MObject(ObjectType::RETURN_VALUE_OBJ), value_(val) {}

    std::string Inspect() const override { return value_->Inspect(); }
    std::shared_ptr<MObject> Value() { return value_; }
private:
    std::shared_ptr<MObject> value_;
};//~ MReturn

class MError : public MObject {
public:
    explicit MError(const std::string& message)
    : MObject(ObjectType::ERROR_OBJ), message_(message) {}
    
    std::string Inspect() const override { return message_; }
private:
    std::string message_;
};// MError
#endif // MONKEY_OBJECT_H_INCLUDED

