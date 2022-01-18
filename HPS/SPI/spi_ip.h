// SPI IP 
// SPI IP Library
// CSE4356-SoC | Fall 2021 | Term Project
// Deborah Jahaj and Nathan Fusselman

//=============================================================================
// Hardware Target
//=============================================================================

// Target Platform: DE1-SoC Board

// Hardware configuration:
// SPI Port:
//   GPIO_0[7,9,11,13,15,17,19] are used as a SPI interface
// HPS interface:
//   Mapped to offset of 8000 in light-weight MM interface aperature

//=============================================================================

#ifndef SPI_H_
#define SPI_H_

#include <stdint.h>
#include <stdbool.h>

//=============================================================================
// Subroutines
//=============================================================================

bool spiOpen();

bool getStatus(bool *state);
bool setStatus(bool state);

bool sendData(uint32_t data);
bool readData(uint32_t *data);

bool getRxStatus(bool *empty, bool *full, bool *ovr);
bool getTxStatus(bool *empty, bool *full, bool *ovr);
bool getRxCount(uint8_t *count);
bool getTxCount(uint8_t *count);
bool clearRxOV();
bool clearTxOV();
bool resetRx();
bool resetTx();

bool getWordsize(uint8_t *size);
bool setWordsize(uint8_t size);

bool getDevice(uint8_t *dev);
bool setDevice(uint8_t dev);

bool getCSModeForDevice(uint8_t dev, bool *mode);
bool setCSModeForDevice(uint8_t dev, bool mode);
bool getCSEnableForDevice(uint8_t dev, bool *enable);
bool setCSEnableForDevice(uint8_t dev, bool enable);
bool getSPIModeForDevice(uint8_t dev, bool *spo, bool *sph);
bool setSPIModeForDevice(uint8_t dev, bool spo, bool sph);

bool getBRD(double *brd);
bool setBRD(double brd);

#endif
