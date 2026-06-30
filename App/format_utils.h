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

#ifndef FORMAT_UTILS_H
#define FORMAT_UTILS_H

#include <stdint.h>

void FORMAT_Frequency(uint32_t freq_hz, char *buffer, uint8_t whole_digits);
void FORMAT_CTCSS(uint16_t ctcss_option, char *buffer, uint8_t with_suffix);
void FORMAT_DCS(uint16_t dcs_option, char type_char, char *buffer);
void FORMAT_StepFrequency(uint16_t step, char *buffer);

#endif
