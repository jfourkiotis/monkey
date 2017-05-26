#ifndef MONKEY_OBJECT_H_INCLUDED
#define MONKEY_OBJECT_H_INCLUDED

#include <string>
#include <memory>

enum class ObjectType {
    INTEGER_OBJ,
    BOOLEAN_OBJ,
    NULL_OBJ,
    RETURN_VALUE_OBJ,
};

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
};
#endif // MONKEY_OBJECT_H_INCLUDED

