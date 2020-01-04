#include "Json.h"

static void skip_whitespace(const std::string &data,size_t &i){
    for(char c=data[i];i<data.size()&&((c=data[i]),(c==' '||c=='\t'||c=='\n'||c=='#'));i++){
        if(c=='#')while(i<data.size()&&data[i]!='\n')i++;//comment
    }
}

static inline constexpr char unescape(char c){
    switch(c) {
    case 'a':
        return '\a';
    case 'b':
        return '\b';
    case 't':
        return '\t';
    case 'n':
        return '\n';
    case 'v':
        return '\v';
    case 'f':
        return '\f';
    case 'r':
        return '\r';
    case '\\':
        return '\\';
    case '"':
        return '\"';
    default:
        return c;
    }
}

static std::string readString(const std::string &data,size_t &i){
    if(skip_whitespace(data,i),data[i]!='"'){
        throw std::runtime_error("Trying to read non-string as string");
    }
    i++;
    size_t start=i;
    for(;i<data.size()&&(data[i]!='"'||data[i-1]=='\\');i++);
    size_t end=i;
    std::string temp;
    bool reading_escape=false;
    for(size_t j=start;j<end;j++){
        if(reading_escape){
            temp+=unescape(data[j]);
            start=j+1;
            reading_escape=false;
        }else if(data[j]=='\\'){
            temp+=data.substr(start,j-start);
            reading_escape=true;
        }
    }
    if(start!=end){
        temp+=data.substr(start,end-start);
    }
    i++;
    return temp;
}

static std::shared_ptr<Json::JsonElement> getElement(const std::string &data,size_t &i);

static std::shared_ptr<Json::JsonObject> getObject(const std::string &data,size_t &i){
    if(skip_whitespace(data,i),data[i]!='{'){
        return nullptr;
    }
    std::map<std::string,std::shared_ptr<Json::JsonElement>> obj;
    i++;
    while(skip_whitespace(data,i),data[i]!='}'){
        std::string key=readString(data,i);
        skip_whitespace(data,i);
        char c=data[i];
        if(c!=':'){
            throw Json::JsonParseError("Expected ':' got '"+std::string(1,data[i])+"'");
        }
        i++;
        obj[key]=getElement(data,i);
        if(skip_whitespace(data,i),data[i]!=','){
            if(data[i]=='}')break;
            throw Json::JsonParseError("Expected ',' got '"+std::string(1,data[i])+"'");
        }
        i++;
    }
    i++;
    return std::make_shared<Json::JsonObject>(obj);
}

static std::shared_ptr<Json::JsonArray> getArray(const std::string &data,size_t &i){
    if(skip_whitespace(data,i),data[i]!='['){
        return nullptr;
    }
    std::vector<std::shared_ptr<Json::JsonElement>> arr;
    i++;
    while(skip_whitespace(data,i),data[i]!=']'){
        arr.push_back(getElement(data,i));
        if(skip_whitespace(data,i),data[i]!=','){
            if(data[i]==']')break;
            throw Json::JsonParseError("Expected ',' got '"+std::string(1,data[i])+"'");
        }
        i++;
    }
    return std::make_shared<Json::JsonArray>(arr);
}

static inline unsigned int ipow(unsigned int base,unsigned int exp){
    unsigned int val=1;
    while(exp-->0){
        val*=base;
    }
    return val;
}

static std::shared_ptr<Json::JsonElement> getLiteral(const std::string &data,size_t &i){
    if(skip_whitespace(data,i),data[i]=='"'){//read string
        return std::make_shared<Json::JsonString>(readString(data,i));
    }else{
        if((data[i]>='0'&&data[i]<='9')||data[i]=='.'||data[i]=='-'){
            int num=0;
            int dec=0;
            int dec_len=0;
            bool is_float=false;
            bool is_negative=false;
            if(data[i]=='-'){
                is_negative=true;
                i++;
            }
            while(i<data.size()){
                if(data[i]>='0'&&data[i]<='9'){
                    if(is_float){
                        dec_len++;
                        dec*=10;
                        dec+=data[i]-'0';
                    }else{
                        num*=10;
                        num+=data[i]-'0';
                    }
                }else if(!is_float&&data[i]=='.'){
                    is_float=true;
                }else{
                    break;
                }
                i++;
            }
            if(is_float){
                double d=num+(double(dec)/ipow(10,dec_len));
                return std::make_shared<Json::JsonFloat>(is_negative?-d:d);
            }else{
                return std::make_shared<Json::JsonInteger>(is_negative?-num:num);
            }
        }else if(data[i]=='n'&&data[i+1]=='u'&&data[i+2]=='l'&&data[i+3]=='l'){
            i+=4;
            return std::make_shared<Json::JsonNull>();
        }else if(data[i]=='t'&&data[i+1]=='r'&&data[i+2]=='u'&&data[i+3]=='e'){
            i+=4;
            return std::make_shared<Json::JsonTrue>();
        }else if(data[i]=='f'&&data[i+1]=='a'&&data[i+2]=='l'&&data[i+3]=='s'&&data[i+4]=='e'){
            i+=5;
            return std::make_shared<Json::JsonFalse>();
        }
    }
    return nullptr;
}

static std::shared_ptr<Json::JsonElement> getElement(const std::string &data,size_t &i){
    std::shared_ptr<Json::JsonElement> e=std::static_pointer_cast<Json::JsonElement>(getObject(data,i));
    if(!e){
        e=std::static_pointer_cast<Json::JsonElement>(getArray(data,i));
        if(!e){
            e=getLiteral(data,i);
            if(!e){
                throw Json::JsonParseError("Invalid Json");
            }
        }
    }
    return e;
}

Json::JsonParseError::JsonParseError(const std::string& error):runtime_error("Error while parsing Json: "+error){}

std::shared_ptr<Json::JsonElement> Json::parse(const std::string &data){
    size_t i=0;
    return getElement(data,i);
}

Json::JsonObject::JsonObject(const std::map<std::string,std::shared_ptr<JsonElement>> &obj):map(obj){
}

bool Json::JsonObject::has_key(std::string key){
    return find(key)!=end();
}

Json::JsonElementType Json::JsonObject::getType(){
    return JsonElementType::OBJECT;
}

Json::JsonArray::JsonArray(const std::vector<std::shared_ptr<JsonElement>> &arr):vector(arr){
}

Json::JsonElementType Json::JsonArray::getType(){
    return JsonElementType::ARRAY;
}

Json::JsonInteger::JsonInteger(int i2):i(i2){
}

Json::JsonElementType Json::JsonInteger::getType(){
    return JsonElementType::INTEGER;
}

Json::JsonFloat::JsonFloat(double f2):f(f2){
}

Json::JsonElementType Json::JsonFloat::getType(){
    return JsonElementType::FLOAT;
}

Json::JsonString::JsonString(const std::string &s2):s(s2){
}

Json::JsonElementType Json::JsonString::getType(){
    return JsonElementType::STRING;
}

Json::JsonElementType Json::JsonTrue::getType(){
    return JsonElementType::JSON_TRUE;
}

Json::JsonElementType Json::JsonFalse::getType(){
    return JsonElementType::JSON_FALSE;
}

Json::JsonElementType Json::JsonNull::getType(){
    return JsonElementType::JSON_NULL;
}



