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

                ~Exception () throw () override;

                const char* what() const throw() override;
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
#define THROW(function, exception, message)\
    do {\
        std::stringstream ssc_throw_message;\
        ssc_throw_message << message;\
        throw exception(ssc_throw_message.str(), __FILE__, function, __LINE__);\
    } while (false)
#endif

