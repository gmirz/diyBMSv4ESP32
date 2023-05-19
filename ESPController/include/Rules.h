#ifndef Rules_H_
#define Rules_H_

#pragma once

#include "defines.h"

// Needs to match the ordering on the HTML screen
// You also need to update "RuleTextDescription" (Rules.cpp)
// Define a max constant for the highest value (change if you add more rules)
#define MAXIMUM_RuleNumber 15
enum Rule : uint8_t
{
    EmergencyStop = 0,
    BMSError = 1,
    CurrentMonitorOverCurrentAmps = 2,
    ModuleOverVoltage = 3,
    ModuleUnderVoltage = 4,
    ModuleOverTemperatureInternal = 5,
    ModuleUnderTemperatureInternal = 6,
    ModuleOverTemperatureExternal = 7,
    ModuleUnderTemperatureExternal = 8,
    CurrentMonitorOverVoltage = 9,
    CurrentMonitorUnderVoltage = 10,
    BankOverVoltage = 11,
    BankUnderVoltage = 12,
    BankRange = 13,
    Timer2 = 14,
    Timer1 = 15
};

// Define a max constant for the highest value (change if you add more warnings)
#define MAXIMUM_InternalWarningCode 9
enum InternalWarningCode : uint8_t
{
    NoWarning = 0,
    ModuleInconsistantBypassVoltage = 1,
    ModuleInconsistantBypassTemperature = 2,
    ModuleInconsistantCodeVersion = 3,
    ModuleInconsistantBoardRevision = 4,
    LoggingEnabledNoSDCard = 5,
    AVRProgrammingMode = 6,
    ChargePrevented = 7,
    DischargePrevented = 8,
    NoExternalTempSensor = 9
};

// Define a max constant for the highest value (change if you add more errors)
#define MAXIMUM_InternalErrorCode 7
enum InternalErrorCode : uint8_t
{
    NoError = 0,
    CommunicationsError = 1,
    ModuleCountMismatch = 2,
    TooManyModules = 3,
    WaitingForModulesToReply = 4,
    ZeroVoltModule = 5,
    ControllerMemoryError = 6,
    ErrorEmergencyStop = 7
};

enum class ChargingMode : uint8_t
{
    standard = 0,
    absorb = 1,
    floating = 2,
    dynamic = 3,
    stopped = 4
};

class Rules
{
private:
    uint16_t dynamicChargeVoltage;
    uint16_t dynamicChargeCurrent;

    bool SharedChargingDischargingRules(diybms_eeprom_settings *mysettings);

    /// @brief Calculate future time (specified in minutes)
    /// @param minutes number of minutes to delay for
    /// @return microseconds "ticks" for esp_timer
    int64_t FutureTime(uint16_t minutes) const
    {
        return esp_timer_get_time() + ((int64_t)minutes * (int64_t)60000000);
    }
    ChargingMode chargemode{ChargingMode::standard};

    int64_t ChargingTimer{0};

public:
    bool rule_outcome[RELAY_RULES];
    // Number of TRUE values in array rule_outcome
    uint8_t active_rule_count;

    // Actual bank voltage reported by the modules (sum of voltage reported by modules) (millivolts)
    uint32_t bankvoltage[maximum_number_of_banks];
    // As above, but each voltage reading limited to "cellmaxmv" setting (used for charge voltage calc)
    uint32_t limitedbankvoltage[maximum_number_of_banks];

    uint16_t LowestCellVoltageInBank[maximum_number_of_banks];
    uint16_t HighestCellVoltageInBank[maximum_number_of_banks];

    // Number of modules who have reported zero volts (bad!)
    uint8_t zeroVoltageModuleCount;

    // Highest pack voltage (millivolts)
    uint32_t highestBankVoltage;
    // Lowest pack voltage (millivolts)
    uint32_t lowestBankVoltage;

    // Highest cell voltage in the whole system (millivolts)
    uint16_t highestCellVoltage;
    // Lowest cell voltage in the whole system (millivolts)
    uint16_t lowestCellVoltage;

    // Highest cell voltage range (mV) across all banks
    uint16_t highestBankRange;

    // Identify address (id) of which module reports the highest/lowest values
    uint8_t address_HighestCellVoltage;
    // Bank with the highest cell voltage in it
    uint8_t index_bank_HighestCellVoltage;
    uint8_t address_LowestCellVoltage;
    uint8_t address_highestExternalTemp;
    uint8_t address_lowestExternalTemp;

    int8_t highestExternalTemp;
    int8_t lowestExternalTemp;
    int8_t highestInternalTemp;
    int8_t lowestInternalTemp;

    InternalErrorCode ErrorCodes[1 + MAXIMUM_InternalErrorCode];
    InternalWarningCode WarningCodes[1 + MAXIMUM_InternalWarningCode];

    // True if at least 1 module has an external temp sensor fitted
    bool moduleHasExternalTempSensor;

    int32_t getChargingTimerSecondsRemaining() const
    {
        if (ChargingTimer == 0)
        {
            return -1;
        }
        // Calculate seconds from microseconds
        return (int32_t)((ChargingTimer - esp_timer_get_time()) / (int64_t)1E6);
    }
    ChargingMode getChargingMode() const
    {
        return chargemode;
    }
    void setChargingMode(ChargingMode newMode)
    {
        if (chargemode == newMode)
            return;
        ESP_LOGI(TAG, "Charging mode changed %u", newMode);
        chargemode = newMode;
    }

    // Number of modules which have not yet reported back to the controller
    uint8_t invalidModuleCount;

    int8_t numberOfActiveErrors;
    int8_t numberOfActiveWarnings;
    int8_t numberOfBalancingModules;

    void ClearValues();
    void ProcessCell(uint8_t bank, uint8_t cellNumber, CellModuleInfo *c, uint16_t cellmaxmv);
    void ProcessBank(uint8_t bank);
    void SetWarning(InternalWarningCode warncode);
    void CalculateChargingMode(diybms_eeprom_settings *mysettings, currentmonitoring_struct *currentMonitor);

    void ClearWarnings()
    {
        memset(&WarningCodes, 0, sizeof(WarningCodes));
        numberOfActiveWarnings = 0;
    }

    void ClearErrors()
    {
        memset(&ErrorCodes, 0, sizeof(ErrorCodes));
        numberOfActiveErrors = 0;
    }

    /// @brief Is the SoC a valid value?
    /// @return Returns TRUE if the state of charge value can be relied upon (its real)
    bool IsStateOfChargeValid(diybms_eeprom_settings *mysettings, currentmonitoring_struct *currentMonitor) const
    {
        return (mysettings->currentMonitoringEnabled &&
                currentMonitor->validReadings &&
                (mysettings->currentMonitoringDevice == CurrentMonitorDevice::DIYBMS_CURRENT_MON_MODBUS || mysettings->currentMonitoringDevice == CurrentMonitorDevice::DIYBMS_CURRENT_MON_INTERNAL));
    }

    void SetError(InternalErrorCode err);
    uint16_t VoltageRangeInBank(uint8_t bank);
    void RunRules(
        int32_t *value,
        int32_t *hysteresisvalue,
        bool emergencyStop,
        uint16_t mins, currentmonitoring_struct *currentMonitor);

    bool IsChargeAllowed(diybms_eeprom_settings *mysettings);
    bool IsDischargeAllowed(diybms_eeprom_settings *mysettings);
    void CalculateDynamicChargeVoltage(diybms_eeprom_settings *mysettings, CellModuleInfo *cellarray);
    void CalculateDynamicChargeCurrent(diybms_eeprom_settings *mysettings, CellModuleInfo *cellarray);
    uint16_t DynamicChargeVoltage() const;
    int16_t DynamicChargeCurrent() const;
    uint16_t StateOfChargeWithRulesApplied(diybms_eeprom_settings *mysettings, float realSOC) const;
};

#endif