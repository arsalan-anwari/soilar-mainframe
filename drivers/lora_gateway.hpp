#ifndef LORA_GATEWAY_HPP
#define LORA_GATEWAY_HPP

#include <vector>
#include <array>
#include <inttypes.h>
#include <boost/thread/mutex.hpp>
#include <functional>

enum class LoraGatewayType{
    LORA_868_EU,
    LORA_915_US,
    LORA_433_ASIA
};

enum class LoraGatewayModes{
    TX_MODE,
    RX_MODE    
};


typedef struct LoraGatewayDeviceGroup{ std::array<uint16_t, 8> idList; int timeoutMS; } LoraGatewayDeviceGroup;

typedef std::vector< LoraGatewayDeviceGroup > LoraGatewayDeviceGroups; 

typedef struct GatewayChannelData{ uint8_t channelID; int32_t offsetFreq; uint8_t radioNum; } GatewayChannelData;

typedef struct LoraPackage{ uint16_t devID; uint8_t pktNum; uint16_t pktSize; std::array<uint8_t, 256> data; } LoraPackage;

class LoraGateway{

public:

    virtual void init() = 0;
    virtual void stop() = 0;
    virtual void start() = 0;

    virtual bool read() = 0;
    virtual void send(const LoraPackage& msg) = 0;

    virtual void print() { printBuffer(-1, -1, -1, -1); }
    virtual void print(int rowMin, int rowMax){ printBuffer(rowMin, rowMax, -1, -1); }
    virtual void print(int rowMin, int rowMax, int colMin, int colMax) { printBuffer(rowMin, rowMax, colMin, colMax); }

    void setMode( const LoraGatewayModes& newMode ){
        modeMutex.lock();
        currentMode = newMode;
        modeMutex.unlock();
        if (newMode == LoraGatewayModes::RX_MODE ){ setRXmode(); }
        if (newMode == LoraGatewayModes::TX_MODE ){ setTXmode(); }
    }
    
    LoraGatewayModes getMode(){
        LoraGatewayModes tmp;
        modeMutex.lock();
        tmp = currentMode;
        modeMutex.unlock();
        return tmp;
    }

protected:
    boost::mutex gatewayMutex;

    std::array< LoraPackage, 8 > gatewayBuffer;
    boost::mutex gatewayBufferMutex;

    LoraGatewayDeviceGroups deviceGroups;
    uint16_t currentDeviceGroup;
    boost::mutex deviceGroupsMutex;

    LoraGatewayModes currentMode;
    boost::mutex modeMutex;

    virtual void setRXmode(){};
    virtual void setTXmode(){};

    bool claimBuffer( const std::function< bool( std::array< LoraPackage, 8 >& ) > & task){
        bool result = false;
        gatewayBufferMutex.lock();
        result = task(gatewayBuffer);
        gatewayBufferMutex.unlock();
        return result;
    }

    void printBuffer(int rowMin, int rowMax, int colMin, int colMax){
        gatewayBufferMutex.lock();
        if (rowMin == -1){ rowMin=0; }
        if (rowMax == -1){ rowMax=8; }
        if (colMin == -1){ colMin=0; }
        if (colMax == -1){ colMax=255; }

       if(rowMin >= 0 && rowMax <= 8 && colMin >=0 && colMax <= 255){
            for(int row=rowMin; row<rowMax; row++){
                std::cout << "Endnode [" << +gatewayBuffer[row].devID << "] -- Packet [" << +gatewayBuffer[row].pktNum << "] ";
                for(int col=colMin; col<colMax; col++){
                     std::cout << +gatewayBuffer[row].data[col] << " ";
                }
                std::cout << std::endl;
            }
        }
        gatewayBufferMutex.unlock();
    }

    void addDeviceGroups(const LoraGatewayDeviceGroups& newDeviceGroups){
        deviceGroupsMutex.lock();
        deviceGroups = newDeviceGroups;
        deviceGroupsMutex.unlock();
    }

    void setDeviceGroup( uint16_t newDeviceGroup ){
        deviceGroupsMutex.lock();
        currentDeviceGroup = newDeviceGroup;
        deviceGroupsMutex.unlock();
    }

    bool isValidDevice( uint16_t deviceID ){
        bool result = false;
        deviceGroupsMutex.lock();
        for(uint8_t i=0; i<8; i++){
            if ( deviceID == deviceGroups[currentDeviceGroup].idList[i] ) { result = true; break; }
        }
        deviceGroupsMutex.unlock();
        return result;
    }

    uint8_t getWaitlist(){
        uint8_t result = 0;
        deviceGroupsMutex.lock();
        for(uint8_t i=0; i<8; i++){
            if ( deviceGroups[currentDeviceGroup].idList[i] != 0 ) { result++; }
        }
        deviceGroupsMutex.unlock();
        return result;
    }

    uint16_t getTimeout(){
        int timeoutVal;
        deviceGroupsMutex.lock();
        timeoutVal = deviceGroups[currentDeviceGroup].timeoutMS;
        deviceGroupsMutex.unlock();
        return timeoutVal;
    }

};

#endif //LORA_GATEWAY_HPP