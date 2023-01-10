#ifndef CPP_HAL_ADAPTER_H
#define CPP_HAL_ADAPTER_H

extern "C" {

#include "loragw_hal.h"

/* -------------------------------------------------------------------------- */
/* --- PUBLIC FUNCTIONS PROTOTYPES ------------------------------------------ */


static inline int _lgw_board_setconf(struct lgw_conf_board_s conf){ return lgw_board_setconf(conf); }


static inline int _lgw_lbt_setconf(struct lgw_conf_lbt_s conf){ return lgw_lbt_setconf(conf); }


static inline int _lgw_rxrf_setconf(uint8_t rf_chain, struct lgw_conf_rxrf_s conf){ return lgw_rxrf_setconf(rf_chain, conf); }


static inline int _lgw_rxif_setconf(uint8_t if_chain, struct lgw_conf_rxif_s conf){ return lgw_rxif_setconf(if_chain, conf); }


static inline int _lgw_txgain_setconf(struct lgw_tx_gain_lut_s *conf){ return lgw_txgain_setconf( conf ); }


static inline int _lgw_start(void){ return lgw_start(); }


static inline int _lgw_stop(void){ return lgw_stop(); }


static inline int _lgw_receive(uint8_t max_pkt, struct lgw_pkt_rx_s *pkt_data){ return lgw_receive(max_pkt, pkt_data); }


static inline int _lgw_send(struct lgw_pkt_tx_s pkt_data){ return lgw_send(pkt_data); }


static inline int _lgw_status(uint8_t select, uint8_t *code){ return lgw_status(select, code); }


static inline int _lgw_abort_tx(void){ return lgw_abort_tx(); }


static inline int _lgw_get_trigcnt(uint32_t* trig_cnt_us){ return lgw_get_trigcnt(trig_cnt_us); }

static inline uint32_t _lgw_time_on_air(struct lgw_pkt_tx_s *packet){ return lgw_time_on_air(packet); }

}

#endif //CPP_HAL_ADAPTER_H