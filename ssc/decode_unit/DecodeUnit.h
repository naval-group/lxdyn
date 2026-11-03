//
// 2005/12/06 @NC SIREHNA
//
// A class and a function for decoding units (transform into a scale factor).
//
// The main function is :
//      DecodeUnits::decodeUnit( std::string unit );
// Example:
//      double scal=DecodeUnits::decodeUnit( std::string("km") );
// would answer:
//      1000.0
//
// In order to convert the value into USI, it must be multiplied with obtained scale factor.
// If conversion fails, then 0 is answered. This should be tested.
//
// Two syntaxes are possible :
//    - implicit operations : for example : kg m3 s-2
//    - explicit operations : for example : kg*m^3/s^2
// It is not possible to mix the two syntaxes : kg*m2 would fail
// Parentheses can be used : ((tour/min) / (mph))
// mult. and divide are evaluated left to right with same precedence.
// for example, kg/m/s is same as (kg/m)/s or also kg/(m*s)
// and kg/m*s is same as (kg/m)*s or also (kg*s)/m
//
// cubic and square can prefix a unit. Example : cubic feet, square yard.
// cube and carre can postfix a unit. Example : metre carre, cm cube
//
// Units coming from names shall begin with an upper case letter.
// Example: Ampere, Joule, Watt, Weber, etc...
//
// This is also true for abbreviated form.
// Example: A, J, W, Wb, etc...
//
// Unit coming from names shall not take a final s : Amperes is wrong
// Other common names can take a s : metres and seconds are valid
// French and English names should be accepted.
//
// Units of metric system can be prefixed with standard prefixes
// Example long form: nanometre
// Example short form: nm
//
// Limitations:
// -----------
// So far, only ASCII characters are handled... no �, no �, no accent.
// No way to convert absolute temperatures.
// No way to convert rad/s into Hz nor into second
// (could be used for specifying a filter pulsation/frequency/period).
// No way to check that units have right dimensions
// (providing 'm/s' for a volume will not fail...)
//
// Testing:
// -------
// Code has not been tested extensively.
// A good start would be to compare outputs with UNIX or CYGWIN /bin/units
//

#ifndef __DECODE_UNIT__
#define __DECODE_UNIT__

#include <string>
#include <istream>
#include <sstream>
#include <map>
#include <vector>

namespace ssc
{
    namespace decode_unit
    {
        class UnitDecoder
        {
            public:
                // classification of chars
                enum e_char_type {
                    k_unknown=-1,
                    k_separator,
                    k_letter,
                    k_digit,
                    k_percent,
                    k_star,k_slash,k_caret,k_minus,
                    k_leftpar,k_rightpar,
                    k_underscore
                };

                // classification of tokens
                enum e_token_type {
                    k_unknown_token=-1,
                    k_word,
                    k_integer,
                    k_double,
                    k_operation,
                    k_leftparenthesis,
                    k_rightparenthesis
                };

            protected :
                // The tokens
                class Token
                {
                    public :
                        Token();
                        virtual ~Token();
                        virtual e_token_type type();
                        virtual std::string description();
                        virtual std::string string_value();
                        virtual int         int_value();
                        virtual double      double_value();
                };

                class WordToken : public Token
                {
                    std::string m_word;
                    public :
                        WordToken(std::string word);
                        virtual ~WordToken();
                        virtual e_token_type type();
                        virtual std::string description();
                        virtual std::string string_value();
                };

                class IntToken : public Token
                {
                    int m_num;
                    public :
                        IntToken(int num);
                        virtual ~IntToken();
                        virtual e_token_type type();
                        virtual std::string description();
                        virtual int    int_value();
                        virtual double double_value();
                };
                class DoubleToken : public Token {
                    double m_num;
                public :
                    DoubleToken(double num);
                    virtual ~DoubleToken();
                    virtual e_token_type type();
                    virtual std::string description();
                    virtual double double_value();
                };

                class OperationToken : public Token
                {
                    int m_op;
                    public :
                        OperationToken(char op);
                        virtual ~OperationToken();
                        virtual e_token_type type();
                        virtual std::string description();
                        virtual int int_value();
                };

                class LeftParToken : public Token
                {
                    public :
                        LeftParToken();
                        virtual ~LeftParToken();
                        virtual e_token_type type();
                        virtual std::string description();
                };

                class RightParToken : public Token
                {
                    public :
                        RightParToken();
                        virtual ~RightParToken();
                        virtual e_token_type type();
                        virtual std::string description();
                };

            private :
                std::istringstream m_stream;                 // source stream
                int m_next_char;                             // next char to be decoded
                decode_unit::UnitDecoder::Token *m_token;    // next token to be decoded
                std::vector<e_char_type> m_char_table;       // classification of characters
                                                             // (for lexical scanner)
                std::map<std::string,double> m_known_units;  // dictionary of knows units

                void advance(); // read one character
                decode_unit::UnitDecoder::Token *scan();  // scan one token
                decode_unit::UnitDecoder::LeftParToken   *scan_leftParenthesis();
                decode_unit::UnitDecoder::RightParToken  *scan_rightParenthesis();
                decode_unit::UnitDecoder::OperationToken *scan_operation();
                decode_unit::UnitDecoder::WordToken      *scan_percent();
                decode_unit::UnitDecoder::WordToken      *scan_word();
                decode_unit::UnitDecoder::Token          *scan_digit();
                bool isIntToken(Token *token);
                bool isDblToken(Token *token);
                bool isWordToken(Token *token);
                bool isWordToken(Token *token, const char *value);
                bool isOpToken(Token *token,char value);
                void clear_token();
                double decode_expression();
                double decode_expression(std::vector<Token *> &tokens,int tbeg,int tend,bool imp);
                double decode_implicit_op_expression(std::vector<Token *> &tokens,int tbeg,int tend);
                double decode_parenthesis();
                double decode_function();
                double decode_word( Token *token );

                //STATIC VARIABLES AND INITIALIZER FUNCTIONS

                // classification of characters
                // static std::vector<e_char_type> k_char_table;

                // known units and conversion factors
                // static std::map<std::string,double> k_known_units;

                static std::map<std::string,double> get_base_units_for_bootstrapping();
                static std::vector<e_char_type> get_char_table();
                static std::map<std::string,double> get_known_units();
                static void extend_with_short_metric_prefix(std::map<std::string,double> &units, std::string unit );
                static void extend_with_long_metric_prefix(std::map<std::string,double> &units, std::string unit );
                static void extend_with_short_metric_prefix(std::map<std::string,double> &units, std::string unit , double value );
                static void extend_with_long_metric_prefix(std::map<std::string,double> &units, std::string unit , double value );
                UnitDecoder();
                UnitDecoder(const UnitDecoder& rhs);
                UnitDecoder& operator=(const UnitDecoder& rhs);
            public :
                UnitDecoder(std::string unit);
                UnitDecoder(std::string unit , std::map<std::string,double> &known_units);
                ~UnitDecoder();
                double decode();
        };

        // main function to decode units
        double decodeUnit( std::string unit );
        double decodeUnit( std::string unit , std::map<std::string,double> &known_units );

    }
}

#endif // __DECODE_UNIT__
