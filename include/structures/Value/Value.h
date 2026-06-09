#pragma once

#include "Common/Common.h"

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

struct NIL {
    auto operator<=>(const NIL&) const = default;
};

struct Value;
using Array = std::vector<Value>;

struct Value {
    using Type = std::variant<double, bool, char, std::string, NIL, std::shared_ptr<Array>>;
    Type as;

    explicit Value() : as(NIL{  }) {  }

    template<typename T>
    explicit Value(T&& value) : as(std::forward<T>(value)) {  }

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

    bool operator==(const Value &other) const {
        return as == other.as;
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
            [](const char &val) {
                return std::string(1, val);
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
            }
        }, as);
    }
};

namespace std {
    template<>
    struct hash<NIL> {
        std::size_t operator()(const NIL&) const noexcept {
            return 0;
        }
    };

    template<>
    struct hash<Value> {
        std::size_t operator()(const Value &value) const noexcept {
            return std::hash<Value::Type>{}(value.as);
        }
    };
}
