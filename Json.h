#ifndef JSON_H
#define JSON_H

#include <map>
#include <vector>
#include <memory>
#include <string>
#include <stdexcept>

namespace Json {
    class JsonParseError : public std::runtime_error {
            public:
                explicit JsonParseError(const std::string& error);
    };
    enum class JsonElementType {
        OBJECT,
        ARRAY,
        INTEGER,
        FLOAT,
        STRING,
        JSON_TRUE,
        JSON_FALSE,
        JSON_NULL,
    };
    class JsonElement{
    public:
        virtual JsonElementType getType()=0;
    };
    class JsonObject : public JsonElement, protected std::map<std::string,std::shared_ptr<JsonElement>> {
    public:
        JsonObject(const std::map<std::string,std::shared_ptr<JsonElement>> &);
        bool has_key(std::string key);
        using map::at;
        virtual JsonElementType getType() override;
    };
    class JsonArray : public JsonElement, protected std::vector<std::shared_ptr<JsonElement>> {
    public:
        JsonArray(const std::vector<std::shared_ptr<JsonElement>> &);
        using vector::size;
        using vector::at;
        virtual JsonElementType getType() override;
    };
    class JsonInteger : public JsonElement {
    public:
        JsonInteger(int);
        int i;
        virtual JsonElementType getType() override;
    };
    class JsonFloat : public JsonElement {
    public:
        JsonFloat(double);
        double f;
        virtual JsonElementType getType() override;
    };
    class JsonString : public JsonElement {
    public:
        JsonString(const std::string &);
        std::string s;
        virtual JsonElementType getType() override;
    };
    class JsonTrue : public JsonElement {
        virtual JsonElementType getType() override;
    };
    class JsonFalse : public JsonElement {
        virtual JsonElementType getType() override;
    };
    class JsonNull : public JsonElement {
        virtual JsonElementType getType() override;
    };
    std::shared_ptr<JsonElement> parse(const std::string &data);
};

#endif // JSON_H
