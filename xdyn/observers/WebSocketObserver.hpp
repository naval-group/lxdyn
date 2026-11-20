#ifndef WEBSOCKETOBSERVER_HPP_
#define WEBSOCKETOBSERVER_HPP_

#include <memory>
#include <ssc/macros.hpp>
#include <string>
#include <vector>

#include "DictObserver.hpp"

namespace ssc {
namespace websocket {
class Client;
}
}  // namespace ssc

using WebSocketPtr = std::shared_ptr<ssc::websocket::Client>;

class WebSocketObserver : public DictObserver {
public:
    WebSocketObserver(const std::string& address, const short unsigned int port);
    WebSocketObserver(
        const std::string& address,
        const short unsigned int port,
        const std::vector<std::string>& data);

private:
    WebSocketPtr socket;
    void flush_after_write() override;
};

#endif /* WEBSOCKETOBSERVER_HPP_ */
