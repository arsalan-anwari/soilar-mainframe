#include "ic880a.hpp"

iC880a::iC880a(const LoraGatewayDeviceGroups& deviceGroups){ 
    addDeviceGroups(deviceGroups); 
    setDeviceGroup(0);
}

void iC880a::init(){
/// ==== GATEWAY RESET ==== 
    delay(200);
    pinMode( 9, OUTPUT );
    digitalWrite( 9, HIGH);
    delay(5000);
    digitalWrite(9, LOW);
/// ==== ! GATEWAY RESET ==== 

/// ==== BOARD CONFIG ====    
    boardconf.lorawan_public = false;
    boardconf.clksrc = 1; 
/// ==== ! BOARD CONFIG ====   

/// ==== RADIO CONFIG ====  
    radioA.enable = true;
    radioA.rssi_offset = -169;
    radioA.type = LGW_RADIO_TYPE_SX1257;

    radioB.enable = true;
    radioB.rssi_offset = -169;
    radioB.type = LGW_RADIO_TYPE_SX1257;
/// ==== ! RADIO CONFIG ====  


/// ==== RX CHANNEL CONFIG (1/2) ====  
    chRX1.rf_chain = iC880aChannel[1].radioNum;
    chRX1.freq_hz = iC880aChannel[1].offsetFreq;
    chRX1.bandwidth = BW_125KHZ;
    chRX1.sync_word_size = 1;
    chRX1.sync_word = 0x12;

    chRX2.rf_chain = iC880aChannel[2].radioNum;
    chRX2.freq_hz = iC880aChannel[2].offsetFreq;
    chRX2.bandwidth = BW_125KHZ;
    chRX2.sync_word_size = 1;
    chRX2.sync_word = 0x12;

    chRX3.rf_chain = iC880aChannel[3].radioNum;
    chRX3.freq_hz = iC880aChannel[3].offsetFreq;
    chRX3.bandwidth = BW_125KHZ;
    chRX3.sync_word_size = 1;
    chRX3.sync_word = 0x12;

    chRX4.rf_chain = iC880aChannel[4].radioNum;
    chRX4.freq_hz = iC880aChannel[4].offsetFreq;
    chRX4.bandwidth = BW_125KHZ;
    chRX4.sync_word_size = 1;
    chRX4.sync_word = 0x12;
/// ==== ! CHANNEL CONFIG (1/2) ====  


/// ==== RX CHANNEL CONFIG (2/2) ====
    chRX5.rf_chain = iC880aChannel[5].radioNum;
    chRX5.freq_hz = iC880aChannel[5].offsetFreq;
    chRX5.bandwidth = BW_125KHZ;
    chRX5.sync_word_size = 1;
    chRX5.sync_word = 0x12;

    chRX6.rf_chain = iC880aChannel[6].radioNum;
    chRX6.freq_hz = iC880aChannel[6].offsetFreq;
    chRX6.bandwidth = BW_125KHZ;
    chRX6.sync_word_size = 1;
    chRX6.sync_word = 0x12;

    chRX7.rf_chain = iC880aChannel[7].radioNum;
    chRX7.freq_hz = iC880aChannel[7].offsetFreq;
    chRX7.bandwidth = BW_125KHZ;
    chRX7.sync_word_size = 1;
    chRX7.sync_word = 0x12;

    chRX8.rf_chain = iC880aChannel[8].radioNum;
    chRX8.freq_hz = iC880aChannel[8].offsetFreq;
    chRX8.bandwidth = BW_125KHZ;
    chRX8.sync_word_size = 1;
    chRX8.sync_word = 0x12;
/// ==== ! RX CHANNEL CONFIG (2/2) ====

/// ==== TX CHANNEL SETUP ==== 
    chTX.enable = true;
    chTX.rf_chain = 0;
    chTX.freq_hz = -125000;
    chTX.sync_word = 0x12;
    chTX.sync_word_size = 1;
    chTX.bandwidth = BW_250KHZ;
    chTX.datarate = DR_LORA_SF10;

    txBuff.tx_mode = IMMEDIATE;
    txBuff.rf_power = 10;
    txBuff.modulation = MOD_LORA;
    txBuff.bandwidth = BW_250KHZ;
    txBuff.datarate = DR_LORA_SF10;
    txBuff.coderate = CR_LORA_4_6;
    txBuff.preamble = 8;
    txBuff.rf_chain = 0; 
    txBuff.freq_hz = ( (uint32_t)((867.0*1e6) + 0.5) - 125000);
/// ==== ! TX PKT SETUP ====

}

void iC880a::stop(){ 
    _lgw_stop();
}

void iC880a::start(){ 
    _lgw_start();
}


bool iC880a::read(){
    bool result = false;
    rxBuffMutex.lock();
    result = claimBuffer(

        [this](std::array< LoraPackage, 8 >& buff)->bool{
            int pktsReceived;
            uint8_t buffPointer = 0;
            uint8_t pktCount = 0;
            uint8_t pktSize = 0;
            uint8_t pktTrueSize = 0;

            uint8_t sessionDevNum = getWaitlist();
            int sessionTimeoutVal = getTimeout();
            uint16_t foundDevices[sessionDevNum] = {0};

            uint8_t devId_msb;
            uint8_t devId_lsb;
            uint16_t devId;
    
            bool allDevFound = false;
            bool timeout = false;

            boost::chrono::steady_clock::time_point endTime;
            boost::chrono::steady_clock::time_point startTime;
            
            std::function<bool(uint16_t)> isAlreadyfound = [&sessionDevNum, &foundDevices](uint16_t _devID)->bool{ 
                for (uint8_t i=0; i<sessionDevNum; i++){ if (_devID == foundDevices[i]){ return true; } }
                return false;
            };

            std::function<bool( int, boost::chrono::steady_clock::time_point&, boost::chrono::steady_clock::time_point&) > withinTimeout = []( 
                int timeoutMS, 
                boost::chrono::steady_clock::time_point & start, 
                boost::chrono::steady_clock::time_point & end 
            )->bool {
                auto time_elapsed = boost::chrono::duration_cast<boost::chrono::milliseconds>(end - start).count();
                return ( time_elapsed < timeoutMS );
            };

            startTime = boost::chrono::steady_clock::now();

            while ( !allDevFound && !timeout ){
                pktsReceived = _lgw_receive(sessionDevNum, rxBuff);
                
                if (pktsReceived > 0){

                    for( int i=0; i<pktsReceived; i++){
                        pktSize = rxBuff[i].size;
                        if( rxBuff[i].payload[0] == 0xFF && rxBuff[i].payload[pktSize - 1] == 0xFF && rxBuff[i].payload[pktSize - 2] == 0xFF ){
                            devId_msb = rxBuff[i].payload[1];
                            devId_lsb = rxBuff[i].payload[2];
                            devId = ((uint16_t)devId_lsb << 8) | devId_msb;

                            pktCount = rxBuff[i].payload[3];
                            pktTrueSize = pktSize - 6;
                            if ( pktCount != 0 && isValidDevice(devId) && !isAlreadyfound(devId) ){
                                buff[buffPointer].devID = devId;
                                buff[buffPointer].pktNum = pktCount;
                                buff[buffPointer].pktSize = pktTrueSize;
                                for(uint16_t pcol = 4; pcol < pktSize; pcol++ ){ buff[buffPointer].data[pcol-4] = rxBuff[i].payload[pcol]; }
                                foundDevices[buffPointer] = devId;
                                buffPointer++;
                            }

                        }

                    }
                }

                endTime = boost::chrono::steady_clock::now();

                if ( buffPointer >= sessionDevNum ){ allDevFound = true; }
                if ( !withinTimeout(sessionTimeoutVal, startTime, endTime) ){ timeout = true; }

            }
            if (timeout){ return false; }
            return allDevFound;
        }
    );

    rxBuffMutex.unlock();
    return result;
}

void iC880a::send(const LoraPackage& msg){
    gatewayMutex.lock();
    
    int j=0;
    uint8_t status;
    uint16_t pktSize = 0;
    uint16_t pktTrueSize = 0;

    pktSize = msg.pktSize;
    pktTrueSize = pktSize + 6;
    if (pktSize > 250){ pktSize=250; }
    txBuff.size = pktTrueSize;
    txBuff.payload[0] = 0xFF;
    txBuff.payload[1] = msg.devID & 0xFF;
    txBuff.payload[2] = (msg.devID >> 8);
    txBuff.payload[3] = msg.pktNum;
    for(uint8_t p=0; p<pktSize; p++ ){ txBuff.payload[p+4] = msg.data[p]; }
    txBuff.payload[pktTrueSize -2] =  0xFF;
    txBuff.payload[pktTrueSize -1] =  0xFF;

    _lgw_send(txBuff);
    do{
        ++j;
        delay(100);
        _lgw_status(TX_STATUS, &status);
    } while( (status != TX_FREE) && (j < 100) );
 
    gatewayMutex.unlock();
}

void iC880a::setRXmode(){

/// ==== RADIO CONFIG ====  
    radioA.tx_enable = false;
    radioB.tx_enable = false;

    radioA.freq_hz = (uint32_t)((867.0*1e6) + 0.5);
    radioB.freq_hz = (uint32_t)((876.5*1e6) + 0.5);

    chRX1.enable = true; chRX2.enable = true; chRX3.enable = true; chRX4.enable = true;
    chRX5.enable = true; chRX6.enable = true; chRX7.enable = true; chRX8.enable = true;
    // chTX.enable = false;
/// ==== ! RADIO CONFIG ====  

/// ==== APPLYING MODEM SETTINGS ====
    std::cout << "Configuring RX modem settings.... \r\n";
    
    _lgw_board_setconf( boardconf );

    _lgw_rxrf_setconf( 0, radioA ); 
    _lgw_rxrf_setconf( 1, radioB );

    _lgw_rxif_setconf( 0, chRX1 );
    _lgw_rxif_setconf( 1, chRX2 );
    _lgw_rxif_setconf( 2, chRX3 );
    _lgw_rxif_setconf( 3, chRX4 );

    _lgw_rxif_setconf( 4, chRX5 );
    _lgw_rxif_setconf( 5, chRX6 );
    _lgw_rxif_setconf( 6, chRX7 );
    _lgw_rxif_setconf( 7, chRX8 );

    // _lgw_rxif_setconf( 8, chTX );

    std::cout << "Succesfully configured RX modem settings! \r\n";
/// ==== ! APPLYING MODEM SETTINGS ====

}

void iC880a::setTXmode(){

/// ==== RADIO CONFIG ====  
    radioA.tx_enable = true;
    radioB.tx_enable = false;
    
    radioA.tx_notch_freq = 129000U;
    radioA.tx_notch_freq = 0;

    radioA.freq_hz = (uint32_t)((867.0*1e6) + 0.5);
    radioB.freq_hz = (uint32_t)((868.4*1e6) + 0.5);

    chRX1.enable = false; chRX2.enable = false; chRX3.enable = false; chRX4.enable = false;
    chRX5.enable = false; chRX6.enable = false; chRX7.enable = false; chRX8.enable = false;
    
/// ==== ! RADIO CONFIG ====  

/// ==== APPLYING MODEM SETTINGS ====
    std::cout << "Configuring TX modem settings.... \r\n";
    
    _lgw_board_setconf( boardconf );

    _lgw_rxrf_setconf( 0, radioA );
    _lgw_rxrf_setconf( 1, radioB );

    _lgw_rxif_setconf( 0, chRX1 );
    _lgw_rxif_setconf( 1, chRX2 );
    _lgw_rxif_setconf( 2, chRX3 );
    _lgw_rxif_setconf( 3, chRX4 );

    _lgw_rxif_setconf( 4, chRX5 );
    _lgw_rxif_setconf( 5, chRX6 );
    _lgw_rxif_setconf( 6, chRX7 );
    _lgw_rxif_setconf( 7, chRX8 );

    _lgw_rxif_setconf( 8, chTX );

    std::cout << "Succesfully configured TX modem settings! \r\n";
/// ==== ! APPLYING MODEM SETTINGS ====

}