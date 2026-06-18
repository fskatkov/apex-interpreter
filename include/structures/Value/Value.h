#pragma once

#include "Common/Common.h"

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

struct NIL {
    NIL() = default;
    auto operator<=>(const NIL&) const = default;
};

namespace std {
    template<>
    struct hash<NIL> {
        std::size_t operator()(const NIL&) const noexcept {
            return 0;
        }
    };
}

struct Value;

struct ValueHasher {
    std::size_t operator()(const Value& v) const noexcept;
};

using Array      = std::vector<Value>;
using Set        = std::unordered_set<Value, ValueHasher>;
using Dictionary = std::unordered_map<Value, Value, ValueHasher>;

struct Function {
    std::string name;
    int arity;
    int startingAddress;
};

struct BoundNativeMethod;

struct NativeFunction {
    std::string name;
    int arity;
    std::function<Value(Value receiver, const std::vector<Value> &)> callable;
};

struct Value {
    using Type = std::variant<double, bool, char, std::string, NIL, std::shared_ptr<Array>, std::shared_ptr<Set>,
        std::shared_ptr<Dictionary>, std::shared_ptr<Function>, std::shared_ptr<NativeFunction>,
        std::shared_ptr<BoundNativeMethod> >;
    Type as;

    explicit Value()                              : as(NIL{  }) {  }
    Value(double val)                             : as(val) {  }
    Value(bool val)                               : as(val) {  }
    Value(char val)                               : as(val) {  }
    Value(const char* val)                        : as(std::string(val)) {  }
    Value(std::string val)                        : as(std::move(val)) {  }
    Value(NIL val)                                : as(val) {  }
    Value(std::shared_ptr<Array> val)             : as(std::move(val)) {  }
    Value(std::shared_ptr<Set> val)               : as(std::move(val)) {  }
    Value(std::shared_ptr<Dictionary> val)        : as(std::move(val)) {  }
    Value(std::shared_ptr<Function> val)          : as(std::move(val)) {  }
    Value(std::shared_ptr<NativeFunction> val)    : as(std::move(val)) {  }
    Value(std::shared_ptr<BoundNativeMethod> val)    : as(std::move(val)) {  }

    template<typename T>
    [[nodiscard]] bool is() const {
        return std::holds_alternative<T>(as);
    }

    template<typename T>
    T& get() {
        return std::get<T>(as);
    }

    template<typename T>
    const T& get() const {
        return std::get<T>(as);
    }

    auto operator<=>(const Value &other) const = default;

    [[nodiscard]] std::string type() const {
        return std::visit(overloaded {
            [](const double &val)  -> std::string {
                return "Number";
            },
            [](const bool &val)  -> std::string {
                return "Boolean";
            },
            [](const char &val)  -> std::string {
                return "Character";
            },
            [](const std::string &val)  -> std::string {
                return "String";
            },
            [](NIL)  -> std::string {
                return "Null";
            },
            [](const std::shared_ptr<Array> &val) -> std::string {
                return "Array";
            },
            [](const std::shared_ptr<Set> &val) -> std::string {
                return "Set";
            },
            [](const std::shared_ptr<Dictionary> &val) -> std::string {
                return "Dictionary";
            },
            [](const std::shared_ptr<Function> &val) -> std::string {
                return "Function(" + std::to_string(val->arity) + " arguments)";
            },
            [](const std::shared_ptr<NativeFunction> &val) -> std::string {
                return "Native_Function(" + std::to_string(val->arity) + " arguments)";
            },
            [](const std::shared_ptr<BoundNativeMethod> &val) -> std::string {
                return "Bound_Method()";
            }
        }, as);
    }

    [[nodiscard]] std::string str() const {
        return std::visit(overloaded {
            [](const double &val) {
                std::ostringstream output;
                output << val;
                return output.str();
            },
            [](const bool &val) {
                return std::string(val ? "True" : "False");
            },
            [](const char &val) {;
                return "\'" + std::string(1, val) + "\'";
            },
            [](const std::string &val) {
                return "\"" + val + "\"";
            },
            [](NIL) {
                return std::string("null");
            },
            [](const std::shared_ptr<Array> &val) -> std::string {
                if (!val) {
                    return "[]";
                }

                std::string result = "[";

                for (auto i = 0; i < val->size(); ++i) {
                    result += (*val)[i].str();

                    if (i < val->size() - 1) {
                        result += ", ";
                    }
                }

                result += "]";
                return result;
            },
            [](const std::shared_ptr<Set> &val) -> std::string {
                if (!val) {
                    return "{}";
                }

                std::string result = "{";

                bool isFirst = true;
                for (const auto& elem : *val) {
                    if (!isFirst) {
                        result += ", ";
                    }

                    result += elem.str();
                    isFirst = false;
                }

                result += "}";
                return result;
            },
            [](const std::shared_ptr<Dictionary> &val) -> std::string {
                if (!val) {
                    return "{}";
                }

                std::string result = "{";

                bool isFirst = true;
                for (const auto&[first, second] : *val) {
                    if (!isFirst) {
                        result += ", ";
                    }

                    result += first.str() + ": " + second.str();
                    isFirst = false;
                }

                result += "}";
                return result;
            },
            [](const std::shared_ptr<Function> &val) -> std::string {
                return "<func `" + (val ? val->name : "anonymous") + ">` with " + std::to_string(val->arity) + " arguments at "
                       + std::to_string(val->startingAddress) + " in memory";
            },
            [](const std::shared_ptr<NativeFunction> &val) -> std::string {
                return "<native function `" + (val ? val->name : "anonymous") + "`>";
            },
            [](const std::shared_ptr<BoundNativeMethod> &val) -> std::string {
                return "Bound_Method with zero or some arguments";
            }
        }, as);
    }
};

inline std::size_t ValueHasher::operator()(const Value& v) const noexcept {
    return std::hash<Value::Type>{}(v.as);
}

struct BoundNativeMethod {
    Value receiver;
    std::shared_ptr<NativeFunction> method;
};
