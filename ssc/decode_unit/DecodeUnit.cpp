// TODO - handle non ASCII characters � � centered dot, cross, divide... accents ?
// - find a syntax for absolute temperature (maybe +K, +�C, +�F)
//   in this case we need function convertUsi(double value ,string unit)
// - complete and test the units

#include "ssc/decode_unit/DecodeUnit.h"
#include <iostream>
#include <cmath>

// NOTE : if i use #include <cmath>, then functions are either
// in std namespace or global namespace, according to the machine and compiler...


using namespace ssc::decode_unit;

double ssc::decode_unit::decodeUnit( std::string unit )
{
    double val;
    try {
        ssc::decode_unit::UnitDecoder decoder( unit );
        val=decoder.decode();
#ifndef NDEBUG
        std::cout << "Successful decoding : " << unit << " -> " << val << std::endl;
#endif
    } catch (const std::string& message) {
        std::cout << message << std::endl;
        val = 0.0;
    }
    return val;
}

double ssc::decode_unit::decodeUnit( std::string unit , std::map<std::string,double> &known_units )
{
    double val;
    try {
        ssc::decode_unit::UnitDecoder decoder( unit , known_units );
        val=decoder.decode();
#ifndef NDEBUG
        std::cout << "Successful decoding : " << unit << " -> " << val << std::endl;
#endif
    } catch (const std::string& message) {
        val = 0.0;
#ifndef NDEBUG
        std::cout << "Known units:" << std::endl;
        for( std::map<std::string,double>::iterator it = known_units.begin(); it != known_units.end() ; it++)
            std::cout << it->first << " -> " << it->second << std::endl;
        std::cout << "Failed decoding :" << unit << " with message : " << message << std::endl;
#else
        (void)message;
#endif
    }
    return val;
}

ssc::decode_unit::UnitDecoder::Token::Token() {}
ssc::decode_unit::UnitDecoder::Token::~Token() {}
ssc::decode_unit::UnitDecoder::e_token_type ssc::decode_unit::UnitDecoder::Token::type() {return k_unknown_token;}
std::string ssc::decode_unit::UnitDecoder::Token::description() {return std::string("unknown token");}
std::string ssc::decode_unit::UnitDecoder::Token::string_value() {std::string tmp; return tmp;}
int         ssc::decode_unit::UnitDecoder::Token::int_value()    {return 0;}
double      ssc::decode_unit::UnitDecoder::Token::double_value() {return 0.0;}

ssc::decode_unit::UnitDecoder::WordToken::WordToken(std::string word) : m_word(word) {}
ssc::decode_unit::UnitDecoder::WordToken::~WordToken() {}
ssc::decode_unit::UnitDecoder::e_token_type ssc::decode_unit::UnitDecoder::WordToken::type() {return k_word;}
std::string ssc::decode_unit::UnitDecoder::WordToken::description() {return std::string("Word token ") + m_word;}
std::string ssc::decode_unit::UnitDecoder::WordToken::string_value() {return m_word;}

ssc::decode_unit::UnitDecoder::IntToken::IntToken(int num) : m_num(num) {}
ssc::decode_unit::UnitDecoder::IntToken::~IntToken() {}
ssc::decode_unit::UnitDecoder::e_token_type ssc::decode_unit::UnitDecoder::IntToken::type() {return k_integer;}
std::string ssc::decode_unit::UnitDecoder::IntToken::description() {
    std::ostringstream tmp;
    tmp << "Int token " << m_num;
    return tmp.str();
}
int    ssc::decode_unit::UnitDecoder::IntToken::int_value() {return m_num;}
double ssc::decode_unit::UnitDecoder::IntToken::double_value() {return static_cast<double> (m_num);}

ssc::decode_unit::UnitDecoder::DoubleToken::DoubleToken(double num) : m_num(num) {}
ssc::decode_unit::UnitDecoder::DoubleToken::~DoubleToken() {}
ssc::decode_unit::UnitDecoder::e_token_type ssc::decode_unit::UnitDecoder::DoubleToken::type() {return k_double;}
std::string ssc::decode_unit::UnitDecoder::DoubleToken::description() {
    std::ostringstream tmp;
    tmp << "Int token " << m_num;
    return tmp.str();
}
double ssc::decode_unit::UnitDecoder::DoubleToken::double_value() {return m_num;}

ssc::decode_unit::UnitDecoder::OperationToken::OperationToken(char op) : m_op(op) {}
ssc::decode_unit::UnitDecoder::OperationToken::~OperationToken() {}
ssc::decode_unit::UnitDecoder::e_token_type ssc::decode_unit::UnitDecoder::OperationToken::type() {return k_operation;}
std::string ssc::decode_unit::UnitDecoder::OperationToken::description() {return std::string("Word token ") + std::to_string(m_op);}
int ssc::decode_unit::UnitDecoder::OperationToken::int_value() {return m_op;}

ssc::decode_unit::UnitDecoder::LeftParToken::LeftParToken() {}
ssc::decode_unit::UnitDecoder::LeftParToken::~LeftParToken() {}
ssc::decode_unit::UnitDecoder::e_token_type ssc::decode_unit::UnitDecoder::LeftParToken::type() {return k_leftparenthesis;}
std::string ssc::decode_unit::UnitDecoder::LeftParToken::description() {return std::string("Left parenthese (");}

ssc::decode_unit::UnitDecoder::RightParToken::RightParToken() {}
ssc::decode_unit::UnitDecoder::RightParToken::~RightParToken() {}
ssc::decode_unit::UnitDecoder::e_token_type ssc::decode_unit::UnitDecoder::RightParToken::type() {return k_rightparenthesis;}
std::string ssc::decode_unit::UnitDecoder::RightParToken::description() {return std::string("Right parenthese )");}


ssc::decode_unit::UnitDecoder::UnitDecoder(std::string unit)
:    m_stream(unit) ,
    m_next_char(0),
    m_token(NULL),
    m_char_table(get_char_table()) ,
    m_known_units(get_known_units())
{
}

ssc::decode_unit::UnitDecoder::UnitDecoder(std::string unit , std::map<std::string,double> &known_units)
:    m_stream(unit) ,
    m_next_char(0),
    m_token(NULL),
    m_char_table(get_char_table()) ,
    m_known_units(known_units)
{
}

ssc::decode_unit::UnitDecoder::~UnitDecoder()
{
    clear_token();
}

void ssc::decode_unit::UnitDecoder::clear_token()
{
    if (m_token != NULL) delete m_token;
    m_token = NULL;
}

// advance one character
void ssc::decode_unit::UnitDecoder::advance()
{
    m_next_char = m_stream.get();
}

// scan one token
ssc::decode_unit::UnitDecoder::Token *ssc::decode_unit::UnitDecoder::scan()
{
    while (m_next_char != -1 ) {
        switch(m_char_table[(size_t)m_next_char])
        {
        case k_separator :
            advance();
            break;
        case k_letter:
            return scan_word();
            break;
        case k_digit:
            return scan_digit();
            break;
        case k_star  :
        case k_slash :
        case k_caret :
        case k_minus :
            return scan_operation();
            break;
        case k_leftpar:
            return scan_leftParenthesis();
            break;
        case k_percent:
            return scan_percent();
            break;
        case k_rightpar:
            return scan_rightParenthesis();
            break;
        case k_underscore:
        default :
            throw std::string("unexpected character : ") + std::to_string(m_next_char);
            break;
        }
    }
    return NULL;
}

ssc::decode_unit::UnitDecoder::LeftParToken *ssc::decode_unit::UnitDecoder::scan_leftParenthesis()
{
    ssc::decode_unit::UnitDecoder::LeftParToken *token=new ssc::decode_unit::UnitDecoder::LeftParToken();
    advance();
    return token;
}

ssc::decode_unit::UnitDecoder::RightParToken *ssc::decode_unit::UnitDecoder::scan_rightParenthesis()
{
    ssc::decode_unit::UnitDecoder::RightParToken *token=new ssc::decode_unit::UnitDecoder::RightParToken();
    advance();
    return token;
}

ssc::decode_unit::UnitDecoder::OperationToken *ssc::decode_unit::UnitDecoder::scan_operation()
{
    ssc::decode_unit::UnitDecoder::OperationToken *token=new ssc::decode_unit::UnitDecoder::OperationToken((char)m_next_char);
    advance();
    return token;
}

ssc::decode_unit::UnitDecoder::WordToken *ssc::decode_unit::UnitDecoder::scan_percent()
{
    std::ostringstream tmp("");
    tmp.put((char)m_next_char);
    advance();
    ssc::decode_unit::UnitDecoder::WordToken *token=new ssc::decode_unit::UnitDecoder::WordToken(tmp.str());
    return token;
}
ssc::decode_unit::UnitDecoder::WordToken *ssc::decode_unit::UnitDecoder::scan_word()
{
    std::ostringstream tmp("");
    do {
        tmp.put((char)m_next_char);
        advance();
    } while(m_next_char != -1 && (m_char_table[(size_t)m_next_char]==k_letter || m_char_table[(size_t)m_next_char] == k_underscore));
    ssc::decode_unit::UnitDecoder::WordToken *token=new ssc::decode_unit::UnitDecoder::WordToken(tmp.str());
    return token;
}

ssc::decode_unit::UnitDecoder::Token *ssc::decode_unit::UnitDecoder::scan_digit()
{
    std::ostringstream str("");
    do {
        str.put((char)m_next_char);
        advance();
    } while(m_next_char != -1 && (m_char_table[(size_t)m_next_char]==k_digit));
    if(m_next_char=='.') {
        str.put((char)m_next_char);
        advance();
        do {
            str.put((char)m_next_char);
            advance();
        } while(m_next_char != -1 && (m_char_table[(size_t)m_next_char]==k_digit));
        if(m_next_char=='e') {
            str.put((char)m_next_char);
            advance();
            if(m_next_char=='-' || m_next_char=='+' ) {
                str.put((char)m_next_char);
                advance();
            }
            if( m_next_char == -1 || (m_char_table[(size_t)m_next_char]!=k_digit) )
                throw std::string("expected exponent integer");
            do {
                str.put((char)m_next_char);
                advance();
            } while(m_next_char != -1 && (m_char_table[(size_t)m_next_char]==k_digit));
        }
        std::istringstream interp(str.str());
        double dval;
        interp >> dval;
        ssc::decode_unit::UnitDecoder::DoubleToken *token=new ssc::decode_unit::UnitDecoder::DoubleToken(dval);
        return token;
    } else {
        std::istringstream interp(str.str());
        int ival;
        interp >> ival;
        ssc::decode_unit::UnitDecoder::IntToken *token=new ssc::decode_unit::UnitDecoder::IntToken(ival);
        return token;
    }
}


// testing tokens
bool ssc::decode_unit::UnitDecoder::isIntToken(ssc::decode_unit::UnitDecoder::Token *token)
{
    return token->type() == k_integer;
}

bool ssc::decode_unit::UnitDecoder::isDblToken(ssc::decode_unit::UnitDecoder::Token *token)
{
    return token->type() == k_double;
}

bool ssc::decode_unit::UnitDecoder::isWordToken(ssc::decode_unit::UnitDecoder::Token *token)
{
    return token->type() == k_word;
}

bool ssc::decode_unit::UnitDecoder::isWordToken(ssc::decode_unit::UnitDecoder::Token *token,const char *value)
{
    return (token->type() == k_word) && (token->string_value() == std::string(value));
}

bool ssc::decode_unit::UnitDecoder::isOpToken(ssc::decode_unit::UnitDecoder::Token *token,char value)
{
    return (token->type() == k_operation) && (token->int_value() == value);
}

// decode unit
double ssc::decode_unit::UnitDecoder::decode()
{
    advance();
    m_token=scan();
    return decode_expression();
}

// the main function dealing with unit grammar...
double ssc::decode_unit::UnitDecoder::decode_expression()
{
    double val=0;

    // 1) accumulate tokens
    std::vector<Token *> tokens;
    try {
        while( m_token != NULL && m_token->type() != k_rightparenthesis) {
            switch( m_token->type() ) {
            case k_operation:
            case k_integer:
            case k_double:
                tokens.push_back(m_token);
                m_token = scan();
                break;
            case k_word:
                if(isWordToken(m_token,"sqrt")) {
                    DoubleToken *token = new DoubleToken(decode_function());
                    tokens.push_back(token);
                } else if(isWordToken(m_token,"cubic")) {
                    clear_token();
                    m_token = scan();
                    if(m_token != NULL && isWordToken(m_token)) {
                        double val = decode_word(m_token);
                        clear_token();
                        m_token = scan();
                        tokens.push_back(new DoubleToken(pow(val,3)));
                    } else {
                        throw std::string("cubic must be followed by a valid unit");
                    }
                } else if(isWordToken(m_token,"square")) {
                    clear_token();
                    m_token = scan();
                    if(m_token != NULL && isWordToken(m_token)) {
                        double val = decode_word(m_token);
                        clear_token();
                        m_token = scan();
                        tokens.push_back(new DoubleToken(pow(val,2)));
                    } else {
                        throw std::string("square must be followed by a valid unit");
                    }
                } else if(isWordToken(m_token,"cube")) {
                    clear_token();
                    m_token = scan();
                    if(tokens.size() > 0 && isWordToken(tokens[tokens.size()-1])) {
                        double val = decode_word(tokens[tokens.size()-1]);
                        delete tokens[tokens.size()-1];
                        tokens[tokens.size()-1] = new DoubleToken(pow(val,3));
                    } else {
                        throw std::string("cube must follow a valid unit");
                    }
                } else if(isWordToken(m_token,"carre")) {
                    clear_token();
                    m_token = scan();
                    if(tokens.size() > 0 && isWordToken(tokens[tokens.size()-1])) {
                        double val = decode_word(tokens[tokens.size()-1]);
                        delete tokens[tokens.size()-1];
                        tokens[tokens.size()-1] = new DoubleToken(pow(val,2));
                    } else {
                        throw std::string("carre must follow a valid unit");
                    }
                } else {
                    tokens.push_back(m_token);
                    m_token = scan();
                }
                break;
            case k_leftparenthesis:
                tokens.push_back(new DoubleToken(decode_parenthesis()));
                break;
            default:
                throw std::string("unexpected token ") + m_token->description();
                break;
            }
        }

        // 2) search highest precedence and evaluate
        if(tokens.size()==0) {
            // no unit ? interpreted as 1
            val=1.0;
        } else {
            val = decode_expression(tokens,0,(int)tokens.size(),true);
        }

        // 3) delete tokens
        for(size_t i=0;i<tokens.size();i++) {
            delete tokens[i];
            tokens[i]=NULL;
        }

    } catch( const std::string mess ) {
        // delete tokens
        for(size_t i=0;i<tokens.size();i++) {
            delete tokens[i];
            tokens[i]=NULL;
        }
        // rethrow
        throw;
    }

    return val;
}

double ssc::decode_unit::UnitDecoder::decode_expression(std::vector<Token *> &tokens,int tbeg,int tend,bool imp)
{
    if(tend-tbeg==1) {
        if(isIntToken(tokens[(size_t)tbeg])) {
            return static_cast<double> (tokens[(size_t)tbeg]->int_value());
        } else if(isDblToken(tokens[(size_t)tbeg])) {
            return tokens[(size_t)tbeg]->double_value();
        } else if (isWordToken(tokens[(size_t)tbeg])) {
            return decode_word(tokens[(size_t)tbeg]);
        } else {
            throw std::string("unexpected token") + tokens[(size_t)tbeg]->description();
        }
    }
    else {
        // search highest precedence and evaluate
        int hprec = 0;
        int iprec = (int)tokens.size();
        for(int i=tend;i>tbeg;) {
            i--;
            if(isOpToken(tokens[(size_t)i],'*') || isOpToken(tokens[(size_t)i],'/') ) {
                if(i==tbeg || i==tend-1) {
                    throw std::string("unexpected */ position");
                } else {
                    double left=decode_expression(tokens,tbeg,i,false);
                    double right=decode_expression(tokens,i+1,tend,false);
                    return (isOpToken(tokens[(size_t)i],'*'))? left*right : left/right;
                }
            } else if(isOpToken(tokens[(size_t)i],'^') ) {
                if(i==tbeg || i==tend-1) {
                    throw std::string("unexpected ^ position");
                } else if(hprec < 2) {
                    hprec = 2;
                    iprec = i;
                }
            } else if(isOpToken(tokens[(size_t)i],'-') ) {
                if(i==tend-1) {
                    throw std::string("unexpected - position");
                } else if(hprec < 1) {
                    hprec = 1;
                    iprec = i;
                }
            }
        }
        if( hprec==2 ) {
            double left=decode_expression(tokens,tbeg,iprec,false);
            double right=decode_expression(tokens,iprec+1,tend,false);
            return pow(left,right);
        } else if (imp) {
            // now decode implicit notation
            return decode_implicit_op_expression(tokens,tbeg,tend);
        } else if (hprec==1) {
            if(iprec==tbeg) {
                return -decode_expression(tokens,iprec+1,tend,false);
            } else {
                throw std::string("unexpected - operation");
            }
        } else {
            throw std::string("implicit operation and non implicit cannot be mixed");
        }
    }
}

// decode format kg m2 s-2
double ssc::decode_unit::UnitDecoder::decode_implicit_op_expression(std::vector<Token *> &tokens,int tbeg,int tend)
{
    if(tend-tbeg<1) {
        return 1.0;
    }
    if(isIntToken(tokens[(size_t)tbeg])) {
        return tokens[(size_t)tbeg]->int_value() * decode_implicit_op_expression(tokens,tbeg+1,tend);
    } else if(isDblToken(tokens[(size_t)tbeg])) {
        return tokens[(size_t)tbeg]->double_value() * decode_implicit_op_expression(tokens,tbeg+1,tend);
    } else if(isWordToken(tokens[(size_t)tbeg])) {
        double val=decode_word(tokens[(size_t)tbeg]);
        if(tend-tbeg == 1) {
            return val;
        } else {
            if(isIntToken(tokens[(size_t)tbeg+1])) {
                return pow(val,tokens[(size_t)tbeg+1]->int_value())
                    * decode_implicit_op_expression(tokens,tbeg+2,tend);
            } else if(isDblToken(tokens[(size_t)tbeg+1])) {
                // QUESTIONNABLE : RAISED TO A NON INTEGER POWER ?
                return pow(val,tokens[(size_t)tbeg+1]->double_value())
                    * decode_implicit_op_expression(tokens,tbeg+2,tend);
            } else if(isOpToken(tokens[(size_t)tbeg+1],'-') && (tend-tbeg > 2) && isIntToken(tokens[(size_t)tbeg+2])) {
                return (1.0 / pow(val,tokens[(size_t)tbeg+2]->int_value()))
                    * decode_implicit_op_expression(tokens,tbeg+3,tend);
            } else if(isWordToken(tokens[(size_t)tbeg+1])) {
                return val * decode_implicit_op_expression(tokens,tbeg+1,tend);
            } else {
                throw std::string("unexpected token") + tokens[(size_t)tbeg+1]->description();
            }
        }
    } else {
        throw std::string("unexpected token") + tokens[(size_t)tbeg]->description();
    }
    return 0;
}

// decode an expression enclosed by parentheses
double ssc::decode_unit::UnitDecoder::decode_parenthesis()
{
    clear_token();
    m_token=scan();
    double result=decode_expression();
    if (m_token->type() != k_rightparenthesis) {
        throw std::string("expected right parenthesis");
    } else {
        clear_token();
        m_token=scan();
    }
    return result;
}

// decode a function call : so far, only function supported is sqrt()
double ssc::decode_unit::UnitDecoder::decode_function()
{
    double result=0;
    Token *func=m_token;
    m_token=scan();
    if(m_token->type() != k_leftparenthesis) {
        throw std::string("expected left parenthesis");
    } else {
        result=decode_parenthesis();
        if(isWordToken(func,"sqrt")) {
            result=sqrt(result);
        } else {
            throw std::string("unknow function : ") + func->string_value();
        }
        delete func;
    }
    return result;
}

// decode a word : replace a unit by its value
double ssc::decode_unit::UnitDecoder::decode_word( Token *token )
{
    double result=1.0;
    std::map<std::string,double>::iterator pointer = m_known_units.find(token->string_value());
    if( pointer == m_known_units.end()) {
        throw std::string("unknow unit : ") + token->string_value();
    } else {
        result = pointer->second;
    }
    return result;
}


// initialize the character classification table
std::vector<ssc::decode_unit::UnitDecoder::e_char_type> ssc::decode_unit::UnitDecoder::get_char_table()
{
    static std::vector<UnitDecoder::e_char_type> table;
    if (table.empty())
    {
        table = std::vector<UnitDecoder::e_char_type>(256,k_unknown);
        int c;
        for(c='0';c<='9';c++) table[(size_t)c]=k_digit;
        for(c='a';c<='z';c++) table[(size_t)c]=k_letter;
        for(c='A';c<='Z';c++) table[(size_t)c]=k_letter;
        table['*']=k_star;
        table['/']=k_slash;
        table['^']=k_caret;
        table['-']=k_minus;
        table['_']=k_underscore;
        table['%']=k_percent;
        table['(']=k_leftpar;
        table[')']=k_rightpar;
        table[' ']=k_separator;
        table['\t']=k_separator;
        table['\r']=k_separator;
        table['\n']=k_separator;
    }
    return table;
}

std::map<std::string,double> ssc::decode_unit::UnitDecoder::get_base_units_for_bootstrapping()
{
    static std::map<std::string,double> units;
    if (units.empty())
    {
        extend_with_short_metric_prefix(units, "m" , 1.0 );
        extend_with_long_metric_prefix(units, "meter" , 1.0 );
        extend_with_long_metric_prefix(units, "meters" , 1.0 );
        extend_with_long_metric_prefix(units, "metre" , 1.0 );
        extend_with_long_metric_prefix(units, "metres" , 1.0 );

        extend_with_short_metric_prefix(units, "s" , 1.0 );
        extend_with_long_metric_prefix(units, "second" , 1.0 );
        extend_with_long_metric_prefix(units, "seconds" , 1.0 );
        extend_with_long_metric_prefix(units, "seconde" , 1.0 );
        extend_with_long_metric_prefix(units, "secondes" , 1.0 );

        extend_with_short_metric_prefix(units, "g" , 1.0e-3 );
        extend_with_long_metric_prefix(units, "gram" , 1.0e-3 );
        extend_with_long_metric_prefix(units, "grams" , 1.0e-3 );
        extend_with_long_metric_prefix(units, "gramme" , 1.0e-3 );
        extend_with_long_metric_prefix(units, "grammes" , 1.0e-3 );

        extend_with_short_metric_prefix(units, "A" , 1.0 );
        extend_with_long_metric_prefix(units, "Ampere" , 1.0 );

        units[ "K" ]=1.0;
        units[ "Kelvin" ]=1.0;

        units[ "usi" ]=1.0;
        units[ "USI" ]=1.0;

        units[ "none" ]=1.0;

        // --------------
        // MEASURING TIME
        // --------------
        units[ "min" ]=60 * units[ "s" ];
        units[ "minute" ]=60 * units[ "s" ];
        units[ "minutes" ]=60 * units[ "s" ];

        units[ "hr" ]=60 * units[ "min" ];
        units[ "hour" ]=60 * units[ "min" ];
        units[ "hours" ]=60 * units[ "min" ];
        units[ "heure" ]=60 * units[ "min" ];
        units[ "heures" ]=60 * units[ "min" ];

        units[ "day" ]=24 * units[ "hr" ];
        units[ "days" ]=24 * units[ "hr" ];
        units[ "jour" ]=24 * units[ "hr" ];
        units[ "jours" ]=24 * units[ "hr" ];

        units[ "week" ]=7 * units[ "day" ];
        units[ "weeks" ]=7 * units[ "day" ];
        units[ "semaine" ]=7 * units[ "day" ];
        units[ "semaines" ]=7 * units[ "day" ];

        units[ "Hz" ]=1.0 /  units[ "s" ];
        units[ "Hertz" ]=1.0 /  units[ "s" ];

        // ----------------
        // MEASURING ANGLES
        // ----------------
        units[ "pi" ]=acos(-1.0);
        units[ "rad" ]=1.0;
        units[ "radian" ]=1.0;
        units[ "radians" ]=1.0;

        units[ "deg" ]=acos(-1.0)/180.0 * units[ "rad" ];
        units[ "degree" ]=acos(-1.0)/180.0 * units[ "rad" ];
        units[ "degrees" ]=acos(-1.0)/180.0 * units[ "rad" ];
        units[ "degre" ]=acos(-1.0)/180.0 * units[ "rad" ];
        units[ "degres" ]=acos(-1.0)/180.0 * units[ "rad" ];

        units[ "min_arc" ]=units[ "degre" ] /60.0;
        units[ "arcmin" ]=units[ "degre" ] /60.0;

        units[ "sec_arc" ]=units[ "degre" ] /3600.0;
        units[ "arcsec" ]=units[ "degre" ] /3600.0;

        units[ "tr" ]=2 * units[ "pi" ] * units[ "rad" ];
        units[ "tour" ]=2 * units[ "pi" ] * units[ "rad" ];
        units[ "tours" ]=2 * units[ "pi" ] * units[ "rad" ];
        units[ "turn" ]=2 * units[ "pi" ] * units[ "rad" ];
        units[ "turns" ]=2 * units[ "pi" ] * units[ "rad" ];
        units[ "rev" ]=2 * units[ "pi" ] * units[ "rad" ];
        units[ "revolution" ]=2 * units[ "pi" ] * units[ "rad" ];
        units[ "revolutions" ]=2 * units[ "pi" ] * units[ "rad" ];
        units[ "circle" ]=2 * units[ "pi" ] * units[ "rad" ];
        units[ "circles" ]=2 * units[ "pi" ] * units[ "rad" ];

        units[ "%" ]=1.0e-2;
        units[ "percent" ]=1.0e-2;
        units[ "percents" ]=1.0e-2;
        units[ "pourcent" ]=1.0e-2;
        units[ "pourcents" ]=1.0e-2;
        units[ "ppm" ]=1.0e-6;

        // ----------------
        // MEASURING LENGTH
        // ----------------
        units[ "in" ]=2.54 * units[ "cm" ];
        units[ "inch" ]=2.54 * units[ "cm" ];
        units[ "inches" ]=2.54 * units[ "cm" ];
        units[ "pouce" ]=2.54 * units[ "cm" ];
        units[ "pouces" ]=2.54 * units[ "cm" ];

        units[ "ft" ]=12 * units[ "in" ];
        units[ "foot" ]=12 * units[ "in" ];
        units[ "feet" ]=12 * units[ "in" ];
        units[ "pied" ]=12 * units[ "in" ];
        units[ "pieds" ]=12 * units[ "in" ];

        units[ "yard" ]=3 * units[ "ft" ];
        units[ "yards" ]=3 * units[ "ft" ];

        units[ "mile" ]=5280 * units[ "ft" ];
        units[ "miles" ]=5280 * units[ "ft" ];

        units[ "u_a" ]=1.49597871e11* units[ "m" ];
        units[ "a_u" ]=1.49597871e11* units[ "m" ];
        units[ "unite_astronomique" ]=1.49597871e11* units[ "m" ];
        units[ "astronomical_unit" ]=1.49597871e11* units[ "m" ];

        units[ "parsec" ]= units[ "a_u" ] / tan( units[ "arcsec" ] );

        units[ "micron" ]= 1.0e-6 * units[ "m" ];
        units[ "microns" ]= 1.0e-6 * units[ "m" ];

        units[ "Angstrom" ]= 1.0e-10 * units[ "m" ];

        units[ "acre" ]= 4840 * pow(units[ "yard" ] , 2);
        units[ "acres" ]= 4840 * pow(units[ "yard" ] , 2);

        units[ "are" ]= 100 * pow(units[ "m" ] , 2);
        units[ "ares" ]= 100 * pow(units[ "m" ] , 2);

        units[ "ha" ]= 100 * units[ "are" ];
        units[ "hectare" ]= 100 * units[ "are" ];
        units[ "hectares" ]= 100 * units[ "are" ];

        units[ "lb" ]= 0.45359237 * units[ "kg" ];
        units[ "lbs" ]= 0.45359237 * units[ "kg" ];
        units[ "livre" ]= 0.45359237 * units[ "kg" ];
        units[ "livres" ]= 0.45359237 * units[ "kg" ];
        units[ "pound" ]= 0.45359237 * units[ "kg" ];
        units[ "pounds" ]= 0.45359237 * units[ "kg" ];

        // metric ton
        units[ "tonne" ]= 1000 * units[ "kg" ];
        units[ "tonnes" ]= 1000 * units[ "kg" ];

        // short ton
        units[ "ton" ]= 2000 * units[ "lbs" ];
        units[ "tons" ]= 2000 * units[ "lbs" ];

        units[ "oz" ]= units[ "lbs" ] / 16;
        units[ "ounce" ]= units[ "lbs" ] / 16;
        units[ "ounces" ]= units[ "lbs" ] / 16;
        units[ "once" ]= units[ "lbs" ] / 16;
        units[ "onces" ]= units[ "lbs" ] / 16;

        units[ "grain" ]= units[ "lbs" ] / 7000;
        units[ "grains" ]= units[ "lbs" ] / 7000;
        units[ "graine" ]= units[ "lbs" ] / 7000;
        units[ "graines" ]= units[ "lbs" ] / 7000;

        // metric carat
        units[ "carat" ]= 0.2 * units[ "g" ];
        units[ "carats" ]= 0.2 * units[ "g" ];
        units[ "english_carat" ]= 0.3163 * units[ "grain" ];
        units[ "english_carats" ]= 0.3163 * units[ "grain" ];

        units[ "quintal" ]= 100 * units[ "kg" ];
        units[ "quintaux" ]= 100 * units[ "kg" ];

        units[ "cental" ]= 100 * units[ "lbs" ];
        units[ "centals" ]= 100 * units[ "lbs" ];

        units[ "l" ]= pow(units[ "dm" ],3);
        units[ "liter" ]= pow(units[ "dm" ],3);
        units[ "liters" ]= pow(units[ "dm" ],3);
        units[ "litre" ]= pow(units[ "dm" ],3);
        units[ "litres" ]= pow(units[ "dm" ],3);

        units[ "cc" ]= pow(units[ "cm" ],3);

        units[ "stere" ]= pow(units[ "m" ],3);
        units[ "steres" ]= pow(units[ "m" ],3);

        // US gallon
        units[ "gal_liquid" ]= 231*pow(units[ "in" ],3);
        units[ "gallon_liquid" ]= 231*pow(units[ "in" ],3);

        units[ "floz" ]= 29.57353*units[ "cc" ];
        units[ "fluid_ounce" ]= 29.57353*units[ "cc" ];

        units[ "bushel" ]= 2150.42*pow(units[ "in" ],3);
        units[ "peck" ]= units[ "bushel" ] / 4;
        units[ "gal_dry" ]= units[ "peck" ] / 2;
        units[ "gallon_dry" ]= units[ "peck" ] / 2;
    }
    return units;
}

// initialize known units
std::map<std::string,double> ssc::decode_unit::UnitDecoder::get_known_units()
{
    static std::map<std::string,double> units;
    if (units.empty())
    {
        units = get_base_units_for_bootstrapping();
        extend_with_short_metric_prefix(units, "Hz" , decodeUnit("1.0/s" , units) );
        extend_with_long_metric_prefix(units, "Hertz" , decodeUnit("1.0/s" , units) );
        extend_with_short_metric_prefix(units, "N" , decodeUnit("kg*m/s^2" , units) );
        extend_with_long_metric_prefix(units, "Newton" , decodeUnit("kg*m/s^2" , units) );

        extend_with_short_metric_prefix(units, "Pa" , decodeUnit("N/m^2" , units) );
        extend_with_long_metric_prefix(units, "Pascal" , decodeUnit("N/m^2" , units) );

        extend_with_short_metric_prefix(units, "J" , decodeUnit("N*m" , units) );
        extend_with_long_metric_prefix(units, "Joule" , decodeUnit("N*m" , units) );

        extend_with_short_metric_prefix(units, "W" , decodeUnit("J/s" , units) );
        extend_with_long_metric_prefix(units, "Watt" , decodeUnit("J/s" , units) );

        units[ "atm" ]= decodeUnit("101325.0*Pa" , units);
        units[ "atmosphere" ]= decodeUnit("101325.0*Pa" , units);

        extend_with_long_metric_prefix(units, "bar" , decodeUnit("1.0e5*Pa" , units) );
        extend_with_long_metric_prefix(units, "bars" , decodeUnit("1.0e5*Pa" , units) );
        extend_with_short_metric_prefix(units, "eV" , decodeUnit("1.6021917e-19*J" , units) );
        extend_with_short_metric_prefix(units, "cal" , decodeUnit("4.1868*J" , units) );
        extend_with_long_metric_prefix(units, "calorie" , decodeUnit("4.1868*J" , units) );
        extend_with_long_metric_prefix(units, "calories" , decodeUnit("4.1868*J" , units) );

        // ELECTRICITY
        extend_with_short_metric_prefix(units, "C" , decodeUnit("A*s" , units) );
        extend_with_long_metric_prefix(units, "Coulomb" , decodeUnit("A*s" , units) );

        extend_with_short_metric_prefix(units, "V" , decodeUnit("W/A" , units) );
        extend_with_long_metric_prefix(units, "Volt" , decodeUnit("W/A" , units) );

        extend_with_long_metric_prefix(units, "Ohm" , decodeUnit("V/A" , units) );

        extend_with_short_metric_prefix(units, "S" , decodeUnit("1/Ohm" , units) );
        extend_with_long_metric_prefix(units, "Siemens" , decodeUnit("1/Ohm" , units) );

        extend_with_short_metric_prefix(units, "F" , decodeUnit("C/V" , units) );
        extend_with_long_metric_prefix(units, "Farad" , decodeUnit("C/V" , units) );

        extend_with_short_metric_prefix(units, "H" , decodeUnit("V/(A/s)" , units) );
        extend_with_long_metric_prefix(units, "Henry" , decodeUnit("V/(A/s)" , units) );

        extend_with_short_metric_prefix(units, "Wb" , decodeUnit("V*s" , units) );
        extend_with_long_metric_prefix(units, "Weber" , decodeUnit("V*s" , units) );

        extend_with_short_metric_prefix(units, "T" , decodeUnit("Wb/m^2" , units) );
        extend_with_long_metric_prefix(units, "Tesla" , decodeUnit("Wb/m^2" , units) );

        extend_with_long_metric_prefix(units, "Gauss" , decodeUnit("1.0e-4*Tesla" , units) );
        extend_with_long_metric_prefix(units, "Maxwell" , decodeUnit("1.0e-8*Wb" , units) );

        // ------
        // MARINE
        // ------
        units[ "n_m" ]= decodeUnit("1852*m" , units);
        units[ "m_n" ]= decodeUnit("1852*m" , units);
        units[ "nautical_mile" ]= decodeUnit("1852*m" , units);
        units[ "nautical_miles" ]= decodeUnit("1852*m" , units);
        units[ "mille_nautique" ]= decodeUnit("1852*m" , units);
        units[ "milles_nautiques" ]= decodeUnit("1852*m" , units);

        units[ "kt" ]= decodeUnit("n_m / hr" , units);
        units[ "knot" ]= decodeUnit("n_m / hr" , units);
        units[ "knots" ]= decodeUnit("n_m / hr" , units);
        units[ "noeud" ]= decodeUnit("n_m / hr" , units);
        units[ "noeuds" ]= decodeUnit("n_m / hr" , units);

        // international cable (not US nor british)
        units[ "cable" ]= decodeUnit("n_m / 10" , units);
        units[ "cables" ]= decodeUnit("n_m / 10" , units);
        units[ "encablure" ]= decodeUnit("n_m / 10" , units);
        units[ "encablures" ]= decodeUnit("n_m / 10" , units);

        // OLD CGS SYSTEM
        extend_with_long_metric_prefix(units, "dyne" , decodeUnit("cm*g/s^2" , units) );
        extend_with_long_metric_prefix(units, "dynes" , decodeUnit("cm*g/s^2" , units) );

        extend_with_long_metric_prefix(units, "erg" , decodeUnit("cm^2*g/s^2" , units) );
        extend_with_long_metric_prefix(units, "ergs" , decodeUnit("cm^2*g/s^2" , units) );

        extend_with_long_metric_prefix(units, "poise" , decodeUnit("g/(cm*s)" , units) );
        extend_with_long_metric_prefix(units, "poises" , decodeUnit("g/(cm*s)" , units) );

        extend_with_long_metric_prefix(units, "Stokes" , decodeUnit("cm^2/s" , units) );

        extend_with_short_metric_prefix(units, "Gal" , decodeUnit("cm/s^2" , units) );
        extend_with_long_metric_prefix(units, "Galileo" , decodeUnit("cm/s^2" , units) );

        // OTHER
        units[ "rpm" ]= units[ "rev" ] / units[ "min" ];

        units[ "mph" ]= decodeUnit("mile/hour" , units);
        units[ "mile_per_hour" ]= decodeUnit("mile/hour" , units);
        units[ "miles_per_hour" ]= decodeUnit("mile/hour" , units);

        units[ "kWh" ]= decodeUnit("kW*hr" , units);

        units[ "kgf" ]= decodeUnit("kg*9.80665*m/s^2" , units);
        units[ "kilogram_force" ]= decodeUnit("kg*9.80665*m/s^2" , units);

        units[ "lbf" ]= decodeUnit("lb*9.80665*m/s^2" , units);
        units[ "pound_force" ]= decodeUnit("lb*9.80665*m/s^2" , units);

        units[ "cheval_force" ]= decodeUnit("550*ft*lbf/s" , units);
        units[ "horse_power" ]= decodeUnit("550*ft*lbf/s" , units);

        units[ "btu" ]= decodeUnit("1054.68*J" , units);
        units[ "british_thermal_unit" ]= decodeUnit("1054.68*J" , units);

        units[ "tep" ]= decodeUnit("42*GJ" , units);
        units[ "tonne_equivalent_petrole" ]= decodeUnit("42*GJ" , units);
    }
    return units;
}


void ssc::decode_unit::UnitDecoder::extend_with_short_metric_prefix(std::map<std::string,double> &units, std::string unit )
{
    units[ std::string("y") + unit ]=1.0e-24 * units[ unit ];
    units[ std::string("z") + unit ]=1.0e-21 * units[ unit ];
    units[ std::string("a") + unit ]=1.0e-18 * units[ unit ];
    units[ std::string("f") + unit ]=1.0e-15 * units[ unit ];
    units[ std::string("p") + unit ]=1.0e-12 * units[ unit ];
    units[ std::string("n") + unit ]=1.0e-9  * units[ unit ];
    units[ std::string("u") + unit ]=1.0e-6  * units[ unit ];
    units[ std::string("m") + unit ]=1.0e-3  * units[ unit ];
    units[ std::string("c") + unit ]=1.0e-2  * units[ unit ];
    units[ std::string("d") + unit ]=1.0e-1  * units[ unit ];
    units[ std::string("da")+ unit ]=1.0e+1  * units[ unit ];
    units[ std::string("h") + unit ]=1.0e+2  * units[ unit ];
    units[ std::string("k") + unit ]=1.0e+3  * units[ unit ];
    units[ std::string("M") + unit ]=1.0e+6  * units[ unit ];
    units[ std::string("G") + unit ]=1.0e+9  * units[ unit ];
    units[ std::string("T") + unit ]=1.0e+12 * units[ unit ];
    units[ std::string("P") + unit ]=1.0e+15 * units[ unit ];
    units[ std::string("E") + unit ]=1.0e+18 * units[ unit ];
    units[ std::string("Z") + unit ]=1.0e+21 * units[ unit ];
    units[ std::string("Y") + unit ]=1.0e+24 * units[ unit ];
}

void ssc::decode_unit::UnitDecoder::extend_with_long_metric_prefix(std::map<std::string,double> &units, std::string unit )
{
    units[ std::string("yocto") + unit ]=1.0e-24 * units[ unit ];
    units[ std::string("zepto") + unit ]=1.0e-21 * units[ unit ];
    units[ std::string("atto" ) + unit ]=1.0e-18 * units[ unit ];
    units[ std::string("femto") + unit ]=1.0e-15 * units[ unit ];
    units[ std::string("pico" ) + unit ]=1.0e-12 * units[ unit ];
    units[ std::string("nano" ) + unit ]=1.0e-9  * units[ unit ];
    units[ std::string("micro") + unit ]=1.0e-6  * units[ unit ];
    units[ std::string("milli") + unit ]=1.0e-3  * units[ unit ];
    units[ std::string("centi") + unit ]=1.0e-2  * units[ unit ];
    units[ std::string("deci" ) + unit ]=1.0e-1  * units[ unit ];
    units[ std::string("deca" ) + unit ]=1.0e+1  * units[ unit ];
    units[ std::string("hecto") + unit ]=1.0e+2  * units[ unit ];
    units[ std::string("kilo" ) + unit ]=1.0e+3  * units[ unit ];
    units[ std::string("mega" ) + unit ]=1.0e+6  * units[ unit ];
    units[ std::string("giga" ) + unit ]=1.0e+9  * units[ unit ];
    units[ std::string("tera" ) + unit ]=1.0e+12 * units[ unit ];
    units[ std::string("peta" ) + unit ]=1.0e+15 * units[ unit ];
    units[ std::string("exa"  ) + unit ]=1.0e+18 * units[ unit ];
    units[ std::string("zeta" ) + unit ]=1.0e+21 * units[ unit ];
    units[ std::string("yotta") + unit ]=1.0e+24 * units[ unit ];
}

void ssc::decode_unit::UnitDecoder::extend_with_short_metric_prefix(std::map<std::string,double> &units, std::string unit , double value )
{
    units[ unit ]=value;
    extend_with_short_metric_prefix( units , unit );
}

void ssc::decode_unit::UnitDecoder::extend_with_long_metric_prefix(std::map<std::string,double> &units, std::string unit , double value )
{
    units[ unit ]=value;
    extend_with_long_metric_prefix( units , unit );
}

