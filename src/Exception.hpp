#ifndef __MY_EXCEPTION__
#define __MY_EXCEPTION__
#define GLM_FORCE_SIMD_AVX2
#include <exception>
#include <string>

struct ERRNOFILE : public std::exception{
    const char* what() const throw(){
        return "No such file";
    }
};
struct ERRNOCURVE : public std::exception{
    const char* what() const throw(){
        return "Not supported curve type";
    }
};
struct ERRPARSE : public std::exception{
    const char* what() const throw(){
        return "Error on parsing";
    }
};


struct ERRWRNGOBJ : public std::exception{
    const char* what() const throw(){
        return "OBJ file has problem";
    }
};

class ERRWRNGIMPLICIT : public std::exception{
    std::string msg;
public:
    ERRWRNGIMPLICIT(const std::string& type):std::exception(){
        this->msg = std::string("No such type of implicit surface : ")+type;
    }
    const char* what() const throw(){
        return this->msg.c_str();
    }
};

#endif