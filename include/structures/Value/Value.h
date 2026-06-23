#pragma once

#include "Common/Common.h"

template<class... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};

template<class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

struct NIL {
    auto operator<=>(const NIL &) const = default;
};

template<>
struct std::hash<NIL> {
    std::size_t operator()(NIL) const noexcept {
        return 0;
    }
};

struct Value;

struct ValueHasher {
    std::size_t operator()(const Value &v) const noexcept;
};

struct FileObject {
    std::filesystem::path path;
    bool is_file_closed{false};

    explicit FileObject(std::filesystem::path path)
        : path(std::move(path)) {
    }
};

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

using Character  = std::shared_ptr<char>;
using String     = std::shared_ptr<std::string>;
using Array      = std::shared_ptr<std::vector<Value> >;
using Set        = std::shared_ptr<std::unordered_set<Value, ValueHasher> >;
using Dictionary = std::shared_ptr<std::unordered_map<Value, Value, ValueHasher> >;
using File       = std::shared_ptr<FileObject>;

struct Value {
    using Type = std::variant<
        double,
        bool,
        Character,
        String,
        NIL,
        Array,
        Set,
        Dictionary,
        File,
        std::shared_ptr<Function>,
        std::shared_ptr<NativeFunction>,
        std::shared_ptr<BoundNativeMethod>
    >;

    Type as{NIL{}};

    Value() = default;

    template<typename T>
        requires std::constructible_from<Type, T> && (!std::same_as<std::decay_t<T>, Value>)
    Value(T &&val) : as(std::forward<T>(val)) {
    }

    template<typename T>
    [[nodiscard]] bool is() const {
        return std::holds_alternative<T>(as);
    }

    template<typename T>
    [[nodiscard]] T &get() {
        return std::get<T>(as);
    }

    template<typename T>
    [[nodiscard]] const T &get() const {
        return std::get<T>(as);
    }

    auto operator<=>(const Value &other) const = default;

    [[nodiscard]] std::string type() const;
    [[nodiscard]] std::string str() const;
};

struct BoundNativeMethod {
    Value receiver;
    std::shared_ptr<NativeFunction> method;

    auto operator<=>(const BoundNativeMethod &) const = default;
};

inline std::size_t ValueHasher::operator()(const Value &v) const noexcept {
    return std::hash<Value::Type>{}(v.as);
}

inline std::string Value::type() const {
    return std::visit(overloaded{
                          [](double) -> std::string { return "Number"; },
                          [](bool) -> std::string { return "Boolean"; },
                          [](const Character &) -> std::string { return "Character"; },
                          [](const String &) -> std::string { return "String"; },
                          [](NIL) -> std::string { return "Null"; },
                          [](const Array &) -> std::string { return "Array"; },
                          [](const Set &) -> std::string { return "Set"; },
                          [](const Dictionary &) -> std::string { return "Dictionary"; },
                          [](const File &) -> std::string { return "FileObject"; },
                          [](const std::shared_ptr<Function> &val) -> std::string {
                              return std::format("<function({} arguments)", val->arity);
                          },
                          [](const std::shared_ptr<NativeFunction> &val) -> std::string {
                              return std::format("<native_function({} arguments)", val->arity);
                          },
                          [](const std::shared_ptr<BoundNativeMethod> &val) -> std::string {
                              return std::format("<bound_native_method({} arguments)", val->method->arity);
                          }
                      }, as);
}

inline std::string Value::str() const {
    return std::visit(overloaded{
                          [](double value) { return std::format("{}", value); },
                          [](bool value) { return std::string(value ? "True" : "False"); },
                          [](const Character &value) { return std::format("'{}'", *value); },
                          [](const String &value) { return std::format("{}", *value); },
                          [](NIL) { return std::format("null"); },
                          [](const Array &value) -> std::string {
                              if (!value || value->empty()) return "[]";

                              std::string resulting_array_output = "[";

                              for (auto i = 0; i < value->size(); ++i) {
                                  resulting_array_output += (*value)[i].str();

                                  if (i < value->size() - 1) {
                                      resulting_array_output += ", ";
                                  }
                              }

                              return resulting_array_output + "]";
                          },
                          [](const Set &value) -> std::string {
                              if (!value || value->empty()) return "{}";

                              std::string resulting_set_output = "{";

                              bool is_first = true;
                              for (const auto &elem: *value) {
                                  if (!is_first) resulting_set_output += ", ";

                                  resulting_set_output += elem.str();
                                  is_first = false;
                              }

                              resulting_set_output += "}";
                              return resulting_set_output;
                          },
                          [](const Dictionary &value) -> std::string {
                              if (!value || value->empty()) return "{}";

                              std::string resulting_dictionary_output = "{";

                              bool is_first = true;
                              for (const auto &[first, second]: *value) {
                                  if (!is_first) resulting_dictionary_output += ", ";

                                  resulting_dictionary_output += first.str() + ": " + second.str();
                                  is_first = false;
                              }

                              resulting_dictionary_output += "}";
                              return resulting_dictionary_output;
                          },
                          [](const File &value) -> std::string {
                              if (!value) return "<uninitialized file>";

                              return std::format("<file `{}`>", value->path.string());
                          },
                          [](const std::shared_ptr<Function> &value) -> std::string {
                              return std::format("<func `{}` with {} arguments>", value ? value->name : "anonymous",
                                                 value->arity);
                          },
                          [](const std::shared_ptr<NativeFunction> &value) -> std::string {
                              return std::format("<native_function `{}`>", value ? value->name : "anonymous");
                          },
                          [](const std::shared_ptr<BoundNativeMethod> &value) -> std::string {
                              return std::format("<bound_native_function `{}`>",
                                                 value ? value->method->name : "anonymous");
                          }
                      }, as);
}
