/* Copyright 2023 Dual Tachyon
 * https://github.com/DualTachyon
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *     Unless required by applicable law or agreed to in writing, software
 *     distributed under the License is distributed on an "AS IS" BASIS,
 *     WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *     See the License for the specific language governing permissions and
 *     limitations under the License.
 */

#include "external/printf/printf.h"
#include "format_utils.h"

void FORMAT_Frequency(uint32_t freq_hz, char *buffer, uint8_t whole_digits)
{
    uint32_t mhz = freq_hz / 100000;
    uint32_t khz = freq_hz % 100000;

    switch (whole_digits)
    {
        case 3:
            sprintf(buffer, "%3u.%05u", mhz, khz);
            break;
        case 4:
            sprintf(buffer, "%03u.%05u", mhz, khz);
            break;
        default:
            sprintf(buffer, "%u.%05u", mhz, khz);
            break;
    }
}

void FORMAT_CTCSS(uint16_t ctcss_option, char *buffer, uint8_t with_suffix)
{
    if (with_suffix)
        sprintf(buffer, "%u.%uHz", ctcss_option / 10, ctcss_option % 10);
    else
        sprintf(buffer, "%u.%u", ctcss_option / 10, ctcss_option % 10);
}

void FORMAT_DCS(uint16_t dcs_option, char type_char, char *buffer)
{
    sprintf(buffer, "D%03o%c", dcs_option, type_char);
}

void FORMAT_StepFrequency(uint16_t step, char *buffer)
{
    sprintf(buffer, "%u.%02uk", step / 100, step % 100);
}
