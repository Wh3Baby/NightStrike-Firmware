#pragma once

#include <vector>
#include <map>
#include <string>

namespace NightStrike {
namespace Modules {

/**
 * @brief Base class for RF protocols
 */
class RFProtocol {
public:
    virtual ~RFProtocol() = default;
    
    // Encode data to protocol-specific format
    virtual std::vector<int> encode(const std::vector<uint8_t>& data) = 0;
    
    // Decode protocol-specific format to data
    virtual std::vector<uint8_t> decode(const std::vector<int>& timings) = 0;
    
    // Get protocol name
    virtual std::string getName() const = 0;
    
    // Get pilot period (if any)
    virtual std::vector<int> getPilotPeriod() const { return {}; }
    
    // Get stop bit (if any)
    virtual std::vector<int> getStopBit() const { return {}; }
};

/**
 * @brief Came protocol (433MHz)
 */
class CameProtocol : public RFProtocol {
public:
    std::vector<int> encode(const std::vector<uint8_t>& data) override;
    std::vector<uint8_t> decode(const std::vector<int>& timings) override;
    std::string getName() const override { return "Came"; }
    std::vector<int> getPilotPeriod() const override { return {-11520, 320}; }
};

/**
 * @brief Linear protocol (433MHz)
 */
class LinearProtocol : public RFProtocol {
public:
    std::vector<int> encode(const std::vector<uint8_t>& data) override;
    std::vector<uint8_t> decode(const std::vector<int>& timings) override;
    std::string getName() const override { return "Linear"; }
};

/**
 * @brief Holtek protocol (433MHz)
 */
class HoltekProtocol : public RFProtocol {
public:
    std::vector<int> encode(const std::vector<uint8_t>& data) override;
    std::vector<uint8_t> decode(const std::vector<int>& timings) override;
    std::string getName() const override { return "Holtek"; }
};

/**
 * @brief NiceFlo protocol (433MHz)
 */
class NiceFloProtocol : public RFProtocol {
public:
    std::vector<int> encode(const std::vector<uint8_t>& data) override;
    std::vector<uint8_t> decode(const std::vector<int>& timings) override;
    std::string getName() const override { return "NiceFlo"; }
};

/**
 * @brief Chamberlain protocol (433MHz)
 */
class ChamberlainProtocol : public RFProtocol {
public:
    std::vector<int> encode(const std::vector<uint8_t>& data) override;
    std::vector<uint8_t> decode(const std::vector<int>& timings) override;
    std::string getName() const override { return "Chamberlain"; }
};

/**
 * @brief Liftmaster protocol (433MHz)
 */
class LiftmasterProtocol : public RFProtocol {
public:
    std::vector<int> encode(const std::vector<uint8_t>& data) override;
    std::vector<uint8_t> decode(const std::vector<int>& timings) override;
    std::string getName() const override { return "Liftmaster"; }
};

/**
 * @brief Ansonic protocol (433MHz)
 */
class AnsonicProtocol : public RFProtocol {
public:
    std::vector<int> encode(const std::vector<uint8_t>& data) override;
    std::vector<uint8_t> decode(const std::vector<int>& timings) override;
    std::string getName() const override { return "Ansonic"; }
};

} // namespace Modules
} // namespace NightStrike

