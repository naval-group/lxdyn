#ifndef __EXCEPTIONHANDLING__
#define __EXCEPTIONHANDLING__

#include <string>
#include <exception>
#include <sstream>

namespace ssc
{
    namespace exception_handling
    {
        class Exception : public std::exception
        {
            public:
                Exception(const std::string& message, const std::string& file, const std::string& function, const unsigned int line);
                Exception(const Exception& rhs);

                ~Exception () throw ();

                virtual const char* what() const throw();
                std::string get_message() const;

            private:
                Exception();
                Exception& operator=(const Exception& rhs);
                std::string full_message;
                std::string short_message;
        };
    }
}

#define QUOTEME_(x) #x
#define QUOTEME(x) QUOTEME_(x)
#define THROW(function,exception, message)\
    std::stringstream __ss;\
    __ss << message;\
    exception exc(__ss.str(), __FILE__, function, __LINE__);\
    throw exc;
#endif

